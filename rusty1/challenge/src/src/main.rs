use tokio::{io, task, process};
use tokio::prelude::*;
use std::process::{Stdio, exit};
use tokio::io::AsyncBufReadExt;

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

    let mut bytes: Vec<u8> = line.into_bytes().into_iter().map(|b| b - 1).collect();
    bytes.push('\n' as u8);

    child_stdin.write_all(&bytes).await.unwrap_or_else(|e| {
      eprintln!("Unable to write to child process: {}", e);
      exit(1);
    });
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

    let mut bytes: Vec<u8> = line.into_bytes().into_iter().map(|b| b - 1).collect();
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
