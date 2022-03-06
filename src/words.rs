use super::{JkFiber, JkError, JkProgram::*, JkList, JkStack, JkQueue, JkDict, parse};

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

fn dig(fiber: &mut JkFiber) -> Result<(), JkError> {
    let c = fiber.pop()?;
    let b = fiber.pop()?;
    let a = fiber.pop()?;
    fiber.push(b);
    fiber.push(c);
    fiber.push(a);
    Ok(())
}

fn bury(fiber: &mut JkFiber) -> Result<(), JkError> {
    let c = fiber.pop()?;
    let b = fiber.pop()?;
    let a = fiber.pop()?;
    fiber.push(c);
    fiber.push(a);
    fiber.push(b);
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

#[cfg(not(target_arch = "wasm32"))]
fn load(fiber: &mut JkFiber) -> Result<(), JkError> {
    use std::io::prelude::*;
    use std::fs::File;
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


pub fn builtin_dict() -> JkDict {
    JkDict::from([
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
        ("dig".to_string(), JkList::from_program(JkBuiltin(dig))),
        ("bury".to_string(), JkList::from_program(JkBuiltin(bury))),

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
    ])
}