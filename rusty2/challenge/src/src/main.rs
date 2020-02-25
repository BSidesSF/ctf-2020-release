use tokio::{io, task, process};
use tokio::prelude::*;
use std::process::{Stdio, exit};
use tokio::io::AsyncBufReadExt;

use std::cmp::min;

const ALPHABET: &'static [u8] = b"0123456789ABCDEFGHJKMNPQRSTVWXYZ";

pub fn encode(data: &[u8]) -> String {
    let (alphabet, padding) = (ALPHABET, false);
    let mut ret = Vec::with_capacity((data.len()+3)/4*5);

    for chunk in data.chunks(5) {
        let buf = {
            let mut buf = [0u8; 5];
            for (i, &b) in chunk.iter().enumerate() {
                buf[i] = b;
            }
            buf
        };
        ret.push(alphabet[((buf[0] & 0xF8) >> 3) as usize]);
        ret.push(alphabet[(((buf[0] & 0x07) << 2) | ((buf[1] & 0xC0) >> 6)) as usize]);
        ret.push(alphabet[((buf[1] & 0x3E) >> 1) as usize]);
        ret.push(alphabet[(((buf[1] & 0x01) << 4) | ((buf[2] & 0xF0) >> 4)) as usize]);
        ret.push(alphabet[(((buf[2] & 0x0F) << 1) | (buf[3] >> 7)) as usize]);
        ret.push(alphabet[((buf[3] & 0x7C) >> 2) as usize]);
        ret.push(alphabet[(((buf[3] & 0x03) << 3) | ((buf[4] & 0xE0) >> 5)) as usize]);
        ret.push(alphabet[(buf[4] & 0x1F) as usize]);
    }

    if data.len() % 5 != 0 {
        let len = ret.len();
        let num_extra = 8-(data.len()%5*8+4)/5;
        if padding {
            for i in 1..num_extra+1 {
                ret[len-i] = b'=';
            }
        } else {
            ret.truncate(len-num_extra);
        }
    }

    String::from_utf8(ret).unwrap()
}

const INV_ALPHABET: [i8; 43] = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, 1, 18, 19, 1, 20, 21, 0, 22, 23, 24, 25, 26, -1, 27, 28, 29, 30, 31];

pub fn decode(data: &str) -> Option<Vec<u8>> {
    if !data.is_ascii() {
        return None;
    }
    let data = data.as_bytes();
    let alphabet = INV_ALPHABET;

    let mut unpadded_data_length = data.len();
    for i in 1..min(6, data.len())+1 {
        if data[data.len() - i] != b'=' {
            break;
        }
        unpadded_data_length -= 1;
    }
    let output_length = unpadded_data_length*5/8;
    let mut ret = Vec::with_capacity((output_length+4)/5*5);
    for chunk in data.chunks(8) {
        let buf = {
            let mut buf = [0u8; 8];
            for (i, &c) in chunk.iter().enumerate() {
                match alphabet.get(c.to_ascii_uppercase().wrapping_sub(b'0') as usize) {
                    Some(&-1) | None => return None,
                    Some(&value) => buf[i] = value as u8,
                };
            }
            buf
        };
        ret.push((buf[0] << 3) | (buf[1] >> 2));
        ret.push((buf[1] << 6) | (buf[2] << 1) | (buf[3] >> 4));
        ret.push((buf[3] << 4) | (buf[4] >> 1));
        ret.push((buf[4] << 7) | (buf[5] << 2) | (buf[6] >> 3));
        ret.push((buf[6] << 5) | buf[7]);
    }
    ret.truncate(output_length);
    Some(ret)
}

async fn input_task(mut child_stdin: process::ChildStdin) {
  let mut lines = io::BufReader::new(io::stdin()).lines();

  loop {
    let line = lines.next_line().await.unwrap_or_else(|e| {
      eprintln!("Error reading from stdin: {}", e);
      exit(1);
    }).unwrap_or_else(|| {
      eprintln!("Error reading from stdin: closed");
      exit(1);
    });

    if let Some(line) = decode(&line) {
      if let Ok(line) = String::from_utf8(line) {
        let mut bytes: Vec<u8> = line.into_bytes();
        bytes.push('\n' as u8);

        child_stdin.write_all(&bytes).await.unwrap_or_else(|e| {
          eprintln!("Unable to write to child process: {}", e);
          exit(1);
        });
      }
    }
  }
}

async fn output_task(child_stdout: process::ChildStdout) {
  let mut lines = io::BufReader::new(child_stdout).lines();

  loop {
    let line = lines.next_line().await.unwrap_or_else(|e| {
      eprintln!("Error reading from stdin: {}", e);
      exit(1);
    }).unwrap_or_else(|| {
      eprintln!("Error reading from stdin: closed");
      exit(1);
    });

    let mut bytes: Vec<u8> = encode(&line.into_bytes()).into_bytes();
    bytes.push('\n' as u8);

    io::stdout().write_all(&bytes).await.unwrap_or_else(|e| {
      eprintln!("Unable to write to child process: {}", e);
      exit(1);
    });
  }
}

#[tokio::main]
async fn main() {
  // Spawn a child process
  let mut child = process::Command::new("/bin/bash").stdin(Stdio::piped()).stdout(Stdio::piped()).stderr(Stdio::null()).spawn().unwrap_or_else(|e| {
    eprintln!("Error creating process: {}", e);
    exit(1);
  });

  // Create a task that feeds the child stdin from our stdin
  task::spawn(input_task(child.stdin.take().unwrap()));
  task::spawn(output_task(child.stdout.take().unwrap()));

  child.await.unwrap();
}
