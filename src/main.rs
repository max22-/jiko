use std::env;

use std::fs::File;
use std::io::{BufRead, BufReader};

use jiko::*;

fn main() -> Result<(), JkError> {
    let arg1 = env::args().nth(1);
    let reader: Box<dyn BufRead> = match arg1 {
        Some(filename) => {
            let file = match File::open(filename) {
                Ok(f) => f,
                Err(_) => {
                    return Err(jiko::JkError::FileNotFound);
                }
            };
            Box::new(BufReader::new(file))
        }
        None => Box::new(BufReader::new(std::io::stdin())),
    };

    let mut fiber = JkFiber::new();

    for line in reader.lines() {
        println!("{}", eval(&mut fiber, line.unwrap()));
    }
    Ok(())
}
