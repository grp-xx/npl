use serde::{Deserialize, Serialize};
#[derive(Serialize, Deserialize, Debug)]

pub enum Message {
    Query,
    Response(String),
}

pub fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}
