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

struct JkList(Vec<JkProgram>);
type JkStack = JkList;
type JkQueue = JkList;

impl fmt::Display for JkList {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            write!(f, "[")?;
            write!(
                f,
                "{}",
                self.0
                    .iter()
                    .map(|p| p.to_string())
                    .collect::<Vec<String>>()
                    .join(", ")
            )?;
            write!(f, "]")
    }
}

use pest::Parser;

#[derive(Parser)]
#[grammar = "jiko.pest"]
pub struct JkParser;


fn parse(input: &str) -> Result<JkQueue, &str>
{
    let pest_output = JkParser::parse(Rule::input, input);
    match pest_output {
        Ok(mut checked_output) => {
            let mut res: JkQueue = JkList(vec!());
            for program in checked_output.next().unwrap().into_inner() {
                for p in program.into_inner() {
                    match p.as_rule() {
                        Rule::integer => res.0.push(JkInt(p.as_str().parse::<i64>().unwrap())),
                        Rule::word => res.0.push(JkWord(p.as_str().to_string())),
                        Rule::EOI => (),
                        _ => unreachable!(),
                    }
                }
            }
            Ok(res)
        },
        Err(_) => Err("Parse error"),
    }
}

fn main() {
    match parse("1 2 add 3 sub") {
        Ok(res) => println!("{}", res),
        Err(msg) => println!("{}", msg),
    }
}
