extern crate pest;
#[macro_use]
extern crate pest_derive;

use std::collections::HashMap;
use std::collections::VecDeque;
use std::env;
use std::fmt;
use std::fs::File;
use std::io::{BufRead, BufReader};

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
struct JkList(VecDeque<JkProgram>);
type JkStack = JkList;
type JkQueue = JkList;
type JkDict = HashMap<String, JkList>;
struct JkFiber {
    stack: JkStack,
    queue: JkQueue,
    dict: JkDict,
    children: Vec<JkFiber>,
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

fn parse(input: &str) -> Result<JkQueue, JkError> {
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
enum JkError {
    ParseError,
    FileNotFound,
    StackUnderflow,
    TypeError,
    UndefinedWord,
    Expected(String),
    RuntimeError(String),
    DivisionByZero,
}

fn add(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.assert_number()?;
    let a = fiber.pop()?.assert_number()?;
    match (a, b) {
        (JkInt(a), JkInt(b)) => {
            fiber.push(JkInt(a + b));
            Ok(())
        }
        (JkFloat(a), JkFloat(b)) => {
            fiber.push(JkFloat(a + b));
            Ok(())
        }
        _ => Err(JkError::TypeError),
    }
}

fn sub(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.assert_number()?;
    let a = fiber.pop()?.assert_number()?;
    match (a, b) {
        (JkInt(a), JkInt(b)) => {
            fiber.push(JkInt(a - b));
            Ok(())
        }
        (JkFloat(a), JkFloat(b)) => {
            fiber.push(JkFloat(a - b));
            Ok(())
        }
        _ => Err(JkError::TypeError),
    }
}

fn mul(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.assert_number()?;
    let a = fiber.pop()?.assert_number()?;
    match (a, b) {
        (JkInt(a), JkInt(b)) => {
            fiber.push(JkInt(a * b));
            Ok(())
        }
        (JkFloat(a), JkFloat(b)) => {
            fiber.push(JkFloat(a * b));
            Ok(())
        }
        _ => Err(JkError::TypeError),
    }
}

fn div(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.assert_number()?;
    let a = fiber.pop()?.assert_number()?;
    match (a, b) {
        (JkInt(a), JkInt(b)) => {
            if b == 0 { return Err(JkError::DivisionByZero) }
            fiber.push(JkInt(a / b));
            Ok(())
        }
        (JkFloat(a), JkFloat(b)) => {
            if b == 0f64 { return Err(JkError::DivisionByZero) }
            fiber.push(JkFloat(a / b));
            Ok(())
        }
        _ => Err(JkError::TypeError),
    }
}

fn modulo(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.assert_number()?;
    let a = fiber.pop()?.assert_number()?;
    match (a, b) {
        (JkInt(a), JkInt(b)) => {
            if b == 0 { return Err(JkError::DivisionByZero) }
            fiber.push(JkInt(a % b));
            Ok(())
        }
        (JkFloat(a), JkFloat(b)) => {
            if b == 0f64 { return Err(JkError::DivisionByZero) }
            fiber.push(JkFloat(a % b));
            Ok(())
        }
        _ => Err(JkError::TypeError),
    }
}

fn lt(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.assert_number()?;
    let a = fiber.pop()?.assert_number()?;
    match (a, b) {
        (JkInt(a), JkInt(b)) => {
            fiber.push(JkBool(a < b));
            Ok(())
        }
        (JkFloat(a), JkFloat(b)) => {
            fiber.push(JkBool(a < b));
            Ok(())
        }
        _ => Err(JkError::TypeError),
    }  
}

fn gt(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.assert_number()?;
    let a = fiber.pop()?.assert_number()?;
    match (a, b) {
        (JkInt(a), JkInt(b)) => {
            fiber.push(JkBool(a > b));
            Ok(())
        }
        (JkFloat(a), JkFloat(b)) => {
            fiber.push(JkBool(a > b));
            Ok(())
        }
        _ => Err(JkError::TypeError),
    }   
}

fn and(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.as_boolean()?;
    let a = fiber.pop()?.as_boolean()?;
    fiber.push(JkBool(a && b));
    Ok(())
}

fn or(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.as_boolean()?;
    let a = fiber.pop()?.as_boolean()?;
    fiber.push(JkBool(a || b));
    Ok(())
}

fn not(fiber: &mut JkFiber) -> Result<(), JkError> {
    let a = fiber.pop()?.as_boolean()?;
    fiber.push(JkBool(!a));
    Ok(())
}

fn dup(fiber: &mut JkFiber) -> Result<(), JkError> {
    let p = fiber.pop()?;
    fiber.push(p.clone());
    fiber.push(p);
    Ok(())
}

fn swap(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?;
    let a = fiber.pop()?;
    fiber.push(b);
    fiber.push(a);
    Ok(())
}

fn drop(fiber: &mut JkFiber) -> Result<(), JkError> {
    fiber.pop()?;
    Ok(())
}

fn quote(fiber: &mut JkFiber) -> Result<(), JkError> {
    let p = fiber.pop()?;
    fiber.push(JkQuotation(JkList::from_program(p)));
    Ok(())
}

fn cat(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.as_list()?;
    let mut a = fiber.pop()?.as_list()?;
    a.append(b);
    fiber.push(JkQuotation(a));
    Ok(())
}

fn apply(fiber: &mut JkFiber) -> Result<(), JkError> {
    let q = fiber.pop()?.as_list()?;
    fiber.prepend_queue(q);
    Ok(())
}

fn over(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?;
    let a = fiber.pop()?;
    fiber.push(a.clone());
    fiber.push(b);
    fiber.push(a);
    Ok(())
}

fn ifte(fiber: &mut JkFiber) -> Result<(), JkError> {
    let else_part = fiber.pop()?.as_list()?;
    let then_part = fiber.pop()?.as_list()?;
    let cond = fiber.pop()?.as_boolean()?;
    if cond {
        fiber.prepend_queue(then_part);
        Ok(())
    } else {
        fiber.prepend_queue(else_part);
        Ok(())
    }
}

fn def(fiber: &mut JkFiber) -> Result<(), JkError> {
    let mut names = fiber.pop()?.as_list()?;

    while names.size() > 0 {
        let name = names.pop_back().unwrap().word_as_string()?;
        let definition = match fiber.pop()? {
            JkQuotation(l) => l,
            atom => JkList::from_program(atom),
        };
        fiber.dict.insert(name, definition);
    }
    Ok(())
}

fn load(fiber: &mut JkFiber) -> Result<(), JkError> {
    use std::io::prelude::*;
    let filepath = fiber.pop()?.as_string()?;
    let mut file = match File::open(&filepath) {
        Ok(file) => file,
        Err(_) => {
            return Err(JkError::FileNotFound);
        }
    };
    let mut s = String::new();
    match file.read_to_string(&mut s) {
        Ok(_) => {
            fiber.append_queue(parse(&s)?);
            Ok(())
        }
        Err(_) => Err(JkError::RuntimeError(format!(
            "Couldn't load file \"{}\"",
            filepath
        ))),
    }
}

fn eq(fiber: &mut JkFiber) -> Result<(), JkError> {
    let b = fiber.pop()?.as_int()?;
    let a = fiber.pop()?.as_int()?;
    fiber.push(JkBool(a == b));
    Ok(())
}

fn head(fiber: &mut JkFiber) -> Result<(), JkError> {
    let mut q = fiber.pop()?.as_list()?;
    let res = q
        .pop_front()
        .ok_or(JkError::RuntimeError("[] head".to_string()))?;
    fiber.push(res);
    Ok(())
}

fn tail(fiber: &mut JkFiber) -> Result<(), JkError> {
    let mut q = fiber.pop()?.as_list()?;
    q.pop_front()
        .ok_or(JkError::RuntimeError("[] tail".to_string()))?;
    fiber.push(JkQuotation(q));
    Ok(())
}

fn empty(fiber: &mut JkFiber) -> Result<(), JkError> {
    let q = fiber.pop()?.as_list()?;
    fiber.push(JkBool(q.size() == 0));
    Ok(())
}

fn reset(fiber: &mut JkFiber) -> Result<(), JkError> {
    fiber.stack = JkStack::new();
    fiber.queue = JkQueue::new();
    Ok(())
}

fn iota(fiber: &mut JkFiber) -> Result<(), JkError> {
    let n = fiber.pop()?.as_int()?;
    let mut res = JkList::new();
    for i in 0..n {
        res.push_back(JkInt(i));
    }
    fiber.push(JkQuotation(res));
    Ok(())
}

fn jk_print(fiber: &mut JkFiber) -> Result<(), JkError> {
    let s = fiber.pop()?.as_string()?;
    print!("{}", s);
    Ok(())
}

fn jk_println(fiber: &mut JkFiber) -> Result<(), JkError> {
    let s = fiber.pop()?.as_string()?;
    println!("{}", s);
    Ok(())
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

fn main() -> Result<(), JkError> {
    let arg1 = env::args().nth(1);
    let reader: Box<dyn BufRead> = match arg1 {
        Some(filename) => {
            let file = match File::open(filename) {
                Ok(f) => f,
                Err(_) => {
                    return Err(JkError::FileNotFound);
                }
            };
            Box::new(BufReader::new(file))
        }
        None => Box::new(BufReader::new(std::io::stdin())),
    };

    let mut fiber = JkFiber {
        stack: JkStack::new(),
        queue: JkQueue::new(),
        dict: JkDict::from([
            ("+".to_string(), JkList::from_program(JkBuiltin(add))),
            ("-".to_string(), JkList::from_program(JkBuiltin(sub))),
            ("*".to_string(), JkList::from_program(JkBuiltin(mul))),
            ("/".to_string(), JkList::from_program(JkBuiltin(div))),
            ("%".to_string(), JkList::from_program(JkBuiltin(modulo))),

            ("<".to_string(), JkList::from_program(JkBuiltin(lt))),
            (">".to_string(), JkList::from_program(JkBuiltin(gt))),
            ("and".to_string(), JkList::from_program(JkBuiltin(and))),
            ("or".to_string(), JkList::from_program(JkBuiltin(or))),
            ("not".to_string(), JkList::from_program(JkBuiltin(not))),

            ("dup".to_string(), JkList::from_program(JkBuiltin(dup))),
            ("swap".to_string(), JkList::from_program(JkBuiltin(swap))),
            ("drop".to_string(), JkList::from_program(JkBuiltin(drop))),
            ("quote".to_string(), JkList::from_program(JkBuiltin(quote))),
            ("cat".to_string(), JkList::from_program(JkBuiltin(cat))),
            ("i".to_string(), JkList::from_program(JkBuiltin(apply))),

            ("over".to_string(), JkList::from_program(JkBuiltin(over))),

            ("ifte".to_string(), JkList::from_program(JkBuiltin(ifte))),
            ("def".to_string(), JkList::from_program(JkBuiltin(def))),


            ("load".to_string(), JkList::from_program(JkBuiltin(load))),
            ("=".to_string(), JkList::from_program(JkBuiltin(eq))),
            ("head".to_string(), JkList::from_program(JkBuiltin(head))),
            ("tail".to_string(), JkList::from_program(JkBuiltin(tail))),
            ("empty?".to_string(), JkList::from_program(JkBuiltin(empty))),
            ("reset".to_string(), JkList::from_program(JkBuiltin(reset))),
            ("iota".to_string(), JkList::from_program(JkBuiltin(iota))),
            ("print".to_string(), JkList::from_program(JkBuiltin(jk_print))),
            ("println".to_string(), JkList::from_program(JkBuiltin(jk_println))),
        ]),
        children: vec![],
    };

    for line in reader.lines() {
        let unwrapped = line.unwrap();
        let res = parse(&unwrapped);
        match res {
            Ok(parsed_line) => {
                fiber.append_queue(parsed_line);
            }
            Err(jkerror) => {
                println!("{:?}", jkerror);
            }
        }
        while fiber.queue.size() > 0 {
            println!("* {} : {}", fiber.stack, fiber.queue);
            match eval_step(&mut fiber) {
                Ok(_) => (),
                Err(jkerror) => println!("Error: {:?}", jkerror),
            }
        }
        println!("* {} : {}", fiber.stack, fiber.queue);
    }
    Ok(())
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
