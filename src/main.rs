extern crate pest;
#[macro_use]
extern crate pest_derive;

use std::collections::HashMap;
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
type JkDict = HashMap<String, JkProgram>;
struct JkFiber {
    stack: JkStack,
    queue: JkQueue,
    dict: JkDict,
    children: Vec<JkFiber>,
}

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

fn from_parse_result(p: pest::iterators::Pair<Rule>) -> JkProgram {
    let (p_rule, p_str, mut p_inner) = (p.as_rule(), p.as_str(), p.into_inner());
    match p_rule {
        Rule::integer => JkInt(p_str.parse::<i64>().unwrap()),
        Rule::boolean => JkBool(p_str.parse::<bool>().unwrap()),
        Rule::word => JkWord(p_str.to_string()),
        Rule::quotation => {
            let mut res: Vec<JkProgram> = vec![];
            println!("quotation: {}", p_inner);
            for p2 in p_inner {
                res.push(from_parse_result(p2));
            }
            JkQuotation(res)
        }
        Rule::program => from_parse_result(p_inner.next().unwrap()),
        Rule::EOI => JkWord("EOI".to_string()),
        _ => {
            println!("unreachable: {:?}", (p_rule, p_str, p_inner));
            JkWord("Unreachable".to_string())
        } //_ => unreachable!(),
    }
}

fn parse(input: &str) -> Result<JkQueue, &str> {
    let pest_output = JkParser::parse(Rule::input, input);
    match pest_output {
        Ok(mut checked_output) => {
            let mut res: JkQueue = JkList(vec![]);
            for program in checked_output.next().unwrap().into_inner() {
                for p in program.into_inner() {
                    res.0.push(from_parse_result(p));
                }
            }
            Ok(res)
        }
        Err(err) => {
            println!("{:?}", err);
            Err("Parse error")
        }
    }
}

fn main() {
    match parse("1 2 add 3 sub false true [1 2 false]") {
        Ok(res) => println!("{}", res),
        Err(msg) => println!("{}", msg),
    }
}
