use bytes::{Bytes, Buf};
use chrono::Utc;
use std::env;
use std::fs::File;
use std::io::prelude::*;

enum LogLevel {
    DEBUG,
    WARN,
    ERROR,
}

fn log(message: String, level: LogLevel) {
    let level_str = match level {
        LogLevel::DEBUG => "DBG",
        LogLevel::WARN => "WRN",
        LogLevel::ERROR => "ERR",
    };

    let time = Utc::now().format("%H:%M:%S %d.%m.%Y");
    println!("{} | {}: {}", time, level_str, message);
}

// removes spaces and newlines from string
// also deletes non-hex-characters
fn prepare_values(vals: Vec<u8>) -> Bytes {
    let out: Vec<u8> = vals
        .into_iter()
        .filter(|val: &u8| {
            let v = *val;
            ((v > 64) && (v < 71)) || // Letters A-F
            ((v > 47) && (v < 58)) // Numbers 0-9
        })
        .collect();
    bytes::Bytes::from(out)
}

fn read_signal_from_file(filename: &String) -> Result<Bytes, ()> {
    if let Ok(mut file) = File::open(&filename) {
        let mut chars = Vec::new();
        match file.read_to_end(&mut chars) {
            Err(err) => log(format!("{}", err), LogLevel::ERROR),
            _ => (),
        };

        Ok(prepare_values(chars))
    } else {
        Err(())
    }
}

fn process_signal(mut signal: Bytes) {
    while signal.remaining() > 8 {
        // let status = signal.get_u16_le();
        let vpos = signal.get_u16();
        let hpos = signal.get_u16();
        let pressure = signal.get_u16();
        println!("{} {} {}", vpos, hpos, pressure);
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        log(format!("Aborting..."), LogLevel::ERROR);
        log(format!("Specify a signal file to parse"), LogLevel::ERROR);
        return;
    }
    let fname: String = args[1].clone();

    if let Ok(signal) = read_signal_from_file(&fname) {
        process_signal(signal);
    } else {
        log(String::from("Error trying to read data"), LogLevel::ERROR);
    }
}
