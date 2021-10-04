use bytes::{Buf, Bytes};
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
    eprintln!("{} | {}: {}", time, level_str, message);
}

// removes spaces and newlines from string
// also deletes non-hex-characters
fn prepare_values(vals: Vec<u8>) -> Bytes {
    // filter everything except hex characters
    let filtered: Vec<u8> = vals
        .into_iter()
        .filter_map(|val: u8| {
            let v = val;
            // println!("{}", val);
            if v > 64 && v < 71 {
                // Letters A-F
                Some((v - 65 + 10) as u8)
            } else if v > 47 && v < 58 {
                // Numbers 0-9
                Some((v - 48) as u8)
            } else {
                None
            }
        })
        .collect();

    log(format!("Found {} 4-Bit-Chunks",filtered.len()), LogLevel::DEBUG);
    let mut out = Vec::<u8>::with_capacity(filtered.len() / 2);
    out.resize(filtered.len() / 2, 0);

    // Aggregate neighboring 2 hex characters into 1 byte
    for (idx, val) in filtered.into_iter().enumerate() {
        let map_idx = (idx as f32 / 2.0).floor() as usize;
        out[map_idx] += if idx % 2 == 0 { val.checked_mul(16).unwrap() } else { val };
    }

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
    while signal.remaining() >= 8 {
        let status = signal.get_u16_le();
        let hpos = signal.get_u16_le();
        let vpos = signal.get_u16_le();
        let pressure = signal.get_u16_le();
        println!("{} {} {} {}",
                 status,
                 vpos,
                 hpos,
                 pressure);
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
