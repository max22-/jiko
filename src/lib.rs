extern crate pest;
#[macro_use]
extern crate pest_derive;

use std::collections::HashMap;
use std::collections::VecDeque;
use std::fmt;

mod words;

type BuiltinWord = fn(&mut JkFiber) -> Result<(), JkError>;

#[derive(Clone)]
enum JkProgram {
    JkBuiltin(BuiltinWord),
    JkInt(i64),
    JkFloat(f64),
    JkBool(bool),
    JkChar(char),
    JkWord(String),
    JkString(String),
    JkQuotation(JkList),
}

impl JkProgram {
    fn as_builtin(self) -> Result<BuiltinWord, JkError> {
        match self {
            JkBuiltin(b) => Ok(b),
            _ => Err(JkError::Expected("builtin".to_string())),
        }
    }
    fn as_int(self) -> Result<i64, JkError> {
        match self {
            JkInt(i) => Ok(i),
            _ => Err(JkError::Expected("integer".to_string())),
        }
    }
    fn as_float(self) -> Result<f64, JkError> {
        match self {
            JkFloat(f) => Ok(f),
            _ => Err(JkError::Expected("float".to_string())),
        }
    }
    fn assert_number(self) -> Result<JkProgram, JkError> {
        match self {
            JkInt(_) => Ok(self),
            JkFloat(_) => Ok(self),
            _ => Err(JkError::Expected("number".to_string())),
        }
    }
    fn as_boolean(self) -> Result<bool, JkError> {
        match self {
            JkBool(b) => Ok(b),
            _ => Err(JkError::Expected("boolean".to_string())),
        }
    }
    fn word_as_string(self) -> Result<String, JkError> {
        match self {
            JkWord(w) => Ok(w),
            _ => Err(JkError::Expected("word".to_string())),
        }
    }
    fn as_string(self) -> Result<String, JkError> {
        match self {
            JkString(s) => Ok(s),
            _ => Err(JkError::Expected("string".to_string())),
        }
    }
    fn as_list(self) -> Result<JkList, JkError> {
        match self {
            JkQuotation(q) => Ok(q),
            _ => Err(JkError::Expected("quotation".to_string())),
        }
    }
}

use JkProgram::*;

impl fmt::Display for JkProgram {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            JkBuiltin(b) => write!(f, "<builtin {:p}>", b),
            JkInt(i) => write!(f, "{}", i),
            JkFloat(fl) => write!(f, "{}", fl),
            JkBool(b) => write!(f, "{}", b),
            JkChar(c) => write!(f, "'{}'", c),
            JkWord(w) => write!(f, "{}", w),
            JkString(s) => write!(f, "\"{}\"", s),
            JkQuotation(q) => write!(f, "{}", q),
        }
    }
}

#[derive(Clone)]
pub struct JkList(VecDeque<JkProgram>);
pub type JkStack = JkList;
pub type JkQueue = JkList;
pub type JkDict = HashMap<String, JkList>;
pub struct JkFiber {
    stack: JkStack,
    queue: JkQueue,
    dict: JkDict,
    children: Vec<JkFiber>,
}

impl JkFiber {
    pub fn new() -> JkFiber {
        JkFiber {
            stack: JkStack::new(),
            queue: JkQueue::new(),
            dict: words::builtin_dict(),
            children: vec![],
        }
    }
}

impl JkList {
    fn new() -> JkList {
        JkList(VecDeque::new())
    }
    fn from_program(p: JkProgram) -> JkList {
        let mut res = JkList::new();
        res.push_back(p);
        res
    }
    fn push_back(&mut self, p: JkProgram) {
        self.0.push_back(p);
    }
    fn push_front(&mut self, p: JkProgram) {
        self.0.push_front(p);
    }
    fn prepend(&mut self, mut p: JkList) {
        p.0.append(&mut self.0);
        self.0 = p.0;
    }
    fn append(&mut self, mut p: JkList) {
        self.0.append(&mut p.0);
    }
    fn pop_back(&mut self) -> Option<JkProgram> {
        self.0.pop_back()
    }
    fn pop_front(&mut self) -> Option<JkProgram> {
        self.0.pop_front()
    }
    fn size(&self) -> usize {
        self.0.len()
    }
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

impl JkFiber {
    fn push(&mut self, p: JkProgram) {
        self.stack.push_back(p);
    }
    fn pop(&mut self) -> Result<JkProgram, JkError> {
        self.stack.pop_back().ok_or(JkError::StackUnderflow)
    }
    fn pop_queue(&mut self) -> Option<JkProgram> {
        self.queue.pop_front()
    }
    fn prepend_queue(&mut self, l: JkList) {
        self.queue.prepend(l);
    }
    fn append_queue(&mut self, l: JkList) {
        self.queue.append(l);
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
        Rule::float => JkFloat(p_str.parse::<f64>().unwrap()),
        Rule::boolean => JkBool(p_str.parse::<bool>().unwrap()),
        Rule::word => JkWord(p_str.to_string()),
        Rule::string => from_parse_result(p_inner.next().unwrap()),
        Rule::string_inner => JkString(p_str.to_string()),
        Rule::quotation => {
            let mut res = JkList::new();
            for p2 in p_inner {
                res.push_back(from_parse_result(p2));
            }
            JkQuotation(res)
        }
        Rule::program => from_parse_result(p_inner.next().unwrap()),
        Rule::EOI => JkWord("EOI".to_string()), /* TODO : handle this case better ^^ */
        _ => unreachable!(),
    }
}

pub fn parse(input: &str) -> Result<JkQueue, JkError> {
    let pest_output = JkParser::parse(Rule::input, input);
    match pest_output {
        Ok(mut checked_output) => {
            let mut res: JkQueue = JkQueue::new();
            for program in checked_output.next().unwrap().into_inner() {
                for p in program.into_inner() {
                    res.push_back(from_parse_result(p));
                }
            }
            Ok(res)
        }
        Err(err) => {
            println!("{:?}", err);
            Err(JkError::ParseError)
        }
    }
}

#[derive(Debug)]
pub enum JkError {
    ParseError,
    FileNotFound,
    StackUnderflow,
    TypeError,
    UndefinedWord,
    Expected(String),
    RuntimeError(String),
    DivisionByZero,
}

fn eval_atom(fiber: &mut JkFiber, p: JkProgram) -> Result<(), JkError> {
    match p {
        JkWord(w) => match fiber.dict.get(&w) {
            Some(definition) => {
                fiber.prepend_queue(definition.clone());
                Ok(())
            }
            None => Err(JkError::UndefinedWord),
        },
        JkBuiltin(b) => b(fiber),
        _ => {
            fiber.push(p);
            Ok(())
        }
    }
}

fn eval_step(fiber: &mut JkFiber) -> Result<(), JkError> {
    let p = fiber.pop_queue();
    match p {
        Some(p) => eval_atom(fiber, p),
        None => Ok(()),
    }
}

pub fn eval(fiber: &mut JkFiber, s: String) -> String {
    let res = parse(&s);
    match res {
        Ok(parsed_line) => {
            fiber.append_queue(parsed_line);
        }
        Err(jkerror) => {
            return format!("{:?}", jkerror);
        }
    }
    while fiber.queue.size() > 0 {
        //println!("* {} : {}", fiber.stack, fiber.queue);
        match eval_step(fiber) {
            Ok(_) => (),
            Err(jkerror) => println!("Error: {:?}", jkerror),
        }
    }
    format!("* {} : {}", fiber.stack, fiber.queue)
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_basic_parse() {
        match parse("1 2 add 3 sub false true [1 2 false]") {
            Ok(_) => (),
            Err(jkerror) => panic!("parse error: {:?}", jkerror),
        }
    }

    #[test]
    fn test_word_parse() {
        let mut res = parse("word2").unwrap();
        assert_eq!(res.size(), 1);
        match res.pop_front() {
            Some(JkWord(w)) => assert_eq!(w, "word2"),
            None => panic!("parse error"),
            _ => panic!("parse error"),
        }
    }

    /* TODO
    #[test]
    fn test_jklist_prepend() {
        let mut l1 = JkList(VecDeque::from([JkInt(1), JkInt(2), JkWord("+".to_string())]));
        let l2 = JkList(VecDeque::from([JkInt(3), JkInt(4), JkWord("-".to_string())]));
        l1.prepend(l2);
        assert_eq!(l1, VecDeque::from([JkInt(3), JkInt(4), JkWord("-".to_string()),
            JkInt(1), JkInt(2), JkWord("+".to_string())]));
        //println!("{}", l1);
    }
    */
}
