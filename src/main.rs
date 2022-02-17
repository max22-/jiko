extern crate  pest;
#[macro_use]
extern crate pest_derive;

use std::fmt;

enum JkProgram {
    JkInt(i64),
    JkFloat(f64),
    JkBool(bool),
    JkChar(char),
    JkWord(String),
    JkString(String),
    JkQuotation(Vec<JkProgram>),
}

use JkProgram::*;

impl fmt::Display for JkProgram {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            JkInt(i) => write!(f, "{}", i),
            JkFloat(fl) => write!(f, "{}", fl),
            JkBool(b) => write!(f, "{}", b),
            JkChar(c) => write!(f, "'{}'", c),
            JkWord(w) => write!(f, "{}", w),
            JkString(s) => write!(f, "\"{}\"", s),
            JkQuotation(q) => {
                write!(f, "[")?;
                write!(
                    f,
                    "{}",
                    q.iter()
                        .map(|p| p.to_string())
                        .collect::<Vec<String>>()
                        .join(", ")
                )?;
                write!(f, "]")
            }
        }
    }
}

use pest::Parser;

#[derive(Parser)]
#[grammar = "jiko.pest"]
pub struct JkParser;

fn main() {
    let program = JkQuotation(vec![JkInt(1), JkInt(2), JkWord("+".to_string())]);

    println!("Program:");
    println!("{}", program);

    let parse = JkParser::parse(Rule::input, "1234");
    println!("parse: {:?}", parse);
}
