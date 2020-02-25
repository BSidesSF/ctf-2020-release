use std::collections::HashMap;
use std::error::Error;
//use std::fmt;
use std::fs::File;
use std::io::{self, BufRead, BufReader, LineWriter};
use std::io::prelude::*;
use std::process;

use simple_error::{bail};

use salsa20::Salsa20;
use salsa20::stream_cipher::generic_array::GenericArray;
use salsa20::stream_cipher::{NewStreamCipher, SyncStreamCipher};

static DEFAULT_CONFIG: &str = "./config-me.conf";

#[derive(Debug)]
enum MaybeEncrypted {
  NotEncrypted(String),
  Encrypted(Vec<u8>),
}

fn key() -> Vec<u8> {
  // Make this mildly convoluted, since we want to guide folks to an easier solution
  let mut xorer: u8 = 0x41;
  let mut key: Vec<u8> = [0; 32].iter().map(|_| {
    let b: u8 = xorer.wrapping_mul(3).wrapping_add(3);
    xorer = b;
    b
  }).collect();
  key.reverse();
  key
}

impl MaybeEncrypted {
  fn encrypt(data: &[u8]) -> Vec<u8> {
    let key = key();
    let key = GenericArray::from_slice(&key[0..32]);
    let nonce: Vec<u8> = (0..8).map(|_| rand::random::<u8>()).collect();
    let nonce = GenericArray::from_slice(&nonce);

    // create cipher instance
    let mut cipher = Salsa20::new(&key, &nonce);

    // apply keystream (encrypt)
    let mut data: Vec<u8> = Vec::from(data);
    cipher.apply_keystream(&mut data);

    let mut result: Vec<u8> = Vec::new();
    result.extend(nonce);
    result.extend(data);

    result
  }

  fn decrypt(nonce: &[u8], data: &[u8]) -> Vec<u8> {
    let key = key();
    let key = GenericArray::from_slice(&key[0..32]);
    let nonce = GenericArray::from_slice(nonce);


    // create cipher instance
    let mut cipher = Salsa20::new(&key, &nonce);

    // apply keystream (encrypt)
    let mut data: Vec<u8> = Vec::from(data);
    cipher.apply_keystream(&mut data);

    data
  }

  fn new(value: &str, encrypt: bool) -> Self {
    match encrypt {
      false => Self::NotEncrypted(value.into()),
      true  =>  Self::Encrypted(Self::encrypt(value.as_bytes())),
    }
  }

  fn deserialize(value: &[u8]) -> Result<Self, Box<dyn Error>> {
    if value.len() > 1 && value[0..2] == ['E' as u8, '$' as u8] {
      Ok(MaybeEncrypted::Encrypted(hex::decode(&value[2..])?))
    } else {
      Ok(MaybeEncrypted::NotEncrypted(String::from_utf8(value.into())?))
    }
  }

  fn get(&self) -> Result<String, Box<dyn std::error::Error>> {
    match self {
      Self::NotEncrypted(s) => Ok(s.clone()),
      Self::Encrypted(e) => {
        if e.len() < 8 {
          bail!("Invalid nonce value!");
        }

        Ok(String::from_utf8(Self::decrypt(&e[0..8], &e[8..]))?)
      }
    }
  }

  fn serialize(&self) -> String {
    match self {
      Self::NotEncrypted(s) => format!("{}", s),
      Self::Encrypted(e)    => format!("E${}", hex::encode(e)),
    }
  }
}

// impl fmt::Display for MaybeEncrypted {
//   fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
//     match self {
//       Self::NotEncrypted(s) => write!(f, "{}", s),
//       Self::Encrypted(e)    => write!(f, "E${}", hex::encode(e)),
//     }
//   }
// }

#[derive(Debug)]
struct Config(HashMap<String, MaybeEncrypted>);

impl Config {
  fn load(filename: &str) -> Result<Config, Box<dyn Error>> {
    let file = match File::open(filename) {
      Ok(f) => f,
      Err(_) => {
        println!("\nWARNING: Could not load {}!", filename);
        return Ok(Config(HashMap::new()));
      }
    };

    let reader = BufReader::new(file);
    let mut data = HashMap::new();

    // Read the file line by line using the lines() iterator from std::io::BufRead.
    for line in reader.lines() {
      let line = line?;
      let split_line: Vec<&str> = line.splitn(2, ":").collect();
      if split_line.len() != 2 {
        bail!(format!("Badly formatted line (expected: \"key: value\"): {}", line));
      }

      let value = match MaybeEncrypted::deserialize(String::from(split_line[1]).trim().as_bytes()) {
        Ok(d) => d,
        Err(e) => {
          eprintln!("Failed to parse key `{}`: {:?}", split_line[0].trim(), e);
          continue;
        }
      };

      data.insert(
        // Key = a string comprised of the first string
        String::from(split_line[0].trim()),

        // Value = the second string, trimmed and as a MaybeEncrypted object
        value,
      );
    }

    Ok(Config(data))
  }

  fn save(&self, filename: &str) -> Result<(), Box<dyn Error>> {
    let file = File::create(filename)?;
    let mut file = LineWriter::new(file);

    self.0.iter().for_each(|(k, v)| {
      file.write_all(format!("{}: {}\n", k, v.serialize()).as_bytes()).expect("Error writing to file");
    });

    Ok(())
  }

  fn get(&self, s: &str) -> Option<String> {
    match self.0.get(s) {
      Some(s) => s.get().ok(),
      None    => None,
    }
  }

  fn add(&mut self, key: &str, value: &str, encrypt: bool) {
    self.0.insert(
      key.into(),
      MaybeEncrypted::new(value, encrypt),
    );
  }

  fn remove(&mut self, key: &str) {
    self.0.remove(key);
  }

  fn len(&self) -> usize {
    self.0.len()
  }
}

// impl fmt::Display for Config {
//   fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
//     let buf: Vec<String> = self.0.keys().map(|k| {
//       format!("{} -> {}", k, match self.get(k) {
//         Some(s) => s,
//         None => "(None)".into(),
//       })
//     }).into_iter().collect();

//     write!(f, "{}", buf.join("\n"))
//   }
// }

fn prompt(prompt: &str) -> String {
  let stdin = io::stdin();
  let mut stdin = stdin.lock().lines();

  print!("{}", prompt);
  io::stdout().flush().unwrap();

  let choice = stdin.next().unwrap_or_else(|| {
    eprintln!("All done!");
    process::exit(1);
  }).unwrap_or_else(|_| {
    eprintln!("All done!");
    process::exit(1);
  });

  choice.trim().into()
}

fn main() {
  println!("Welcome to the Configuration Configurer, the tool for configuring your config files! Now featuring secure encryption!");
  println!();
  println!("(This tool is brought to you by the San Francisco Department of Redundancy Department)");
  println!();

  let config_file = std::env::args().skip(1).next().unwrap_or(DEFAULT_CONFIG.into());

  println!("Let's start by loading your configuration from {}", config_file);
  let mut config = Config::load(&config_file).unwrap_or_else(|e| {
    eprintln!("Couldn't parse config file: {}", e);
    process::exit(1);
  });

  loop {
    let name = config.get("name").unwrap_or_else(|| {
      println!("\nUh oh! Your config file is missing its name entry! This is required; can you please provide one?");
      let name = prompt("Name > ");
      config.add("name", &name, false);

      name
    });

    println!("\n------------------------------------\n\nWelcome back, {}! Your config file currently has {} entries. What would you like to do?", name, config.len());
    println!();
    println!(" [A]dd a key");
    println!(" [D]elete a key");
    println!(" [S]ave configuration file");
    println!(" [L]oad a different configuration file");
    println!(" [Q]uit");
    println!();

    let choice = prompt("Your choice > ");
    match &choice[..] {
      // Add / update (same code :) )
      "a" | "A" | "u" | "U" => {
        println!("Sure! This will add a new field");
        let key = prompt("Key --> ");
        if key.len() == 0 {
          continue;
        }

        let value = prompt("Value --> ");
        if value.len() == 0 {
          continue;
        }

        let encrypted = prompt("Encrypt? (yes/no) > ");
        let encrypted = match &encrypted[..] {
          "y" | "yes" | "t" | "true" => true,
          _                          => false,
        };

        config.add(&key, &value, encrypted);
      },
      // Delete
      "d" | "D" => {
        println!("Sure! This will delete a key (if it exists)!");
        let key = prompt("Key --> ");
        if key.len() == 0 {
          continue;
        }

        config.remove(&key);
      },
      // Save
      "s" | "S" => {
        println!("Sure! This will write out a config file (potentially overwriting the old one)");
        let filename = prompt(&format!("Filename [default: {}] --> ", &config_file));
        let filename = match filename.len() {
          0 => &config_file,
          _ => &filename,
        };

        config.save(filename).unwrap_or_else(|e| {
          eprintln!("Couldn't save: {:?}", e);
          process::exit(0);
        });

        println!("Saved to {}!", filename);
      },

      // Load / reload
      "r" | "R" | "l" | "L" => {
        println!("Sure! This will load a new config file!");
        let filename = prompt(&format!("Filename [default: {}] --> ", &config_file));
        let filename = match filename.len() {
          0 => &config_file,
          _ => &filename,
        };

        config = Config::load(filename).unwrap_or_else(|e| {
          eprintln!("Couldn't parse config file: {}", e);
          process::exit(1);
        });
      },
      // Quit
      "q" | "Q" => {
        println!("Bye!");
        process::exit(0);
      },
      _ => println!("\nERROR: Invalid choice!\n"),
    }
  }
}
