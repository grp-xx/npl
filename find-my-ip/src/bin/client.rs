use std::net::UdpSocket;
use clap::Parser;

#[derive(Parser, Debug)]
struct Cli {
    /// Server address 
    #[arg(short, long, default_value_t = String::from("127.0.0.1"))]
    server: String,
    /// Server port
    #[arg(short, long, default_value_t = 10000)]
    port: u16,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

use serde::{Deserialize, Serialize};
#[derive(Serialize, Deserialize, Debug)]
enum Message {
    Query,
    Response(String),
}


fn main() {
    let cli = Cli::parse();

    let sock = UdpSocket::bind("0.0.0.0:0").expect("Unable to bind socket");
    let srv_addr = format!("{}:{}",cli.server,cli.port);

    let query = Message::Query;
    let query_jstr = serde_json::to_string(&query).unwrap();

    let _ = sock.send_to(query_jstr.as_bytes(), srv_addr).expect("Unabel to quesry remote server");
    let mut buffer = [0_u8; 1500];
    let (nb,srv) = sock.recv_from(&mut buffer).unwrap();

    let response = std::str::from_utf8(&buffer[..nb]).unwrap();
    let r: Message = serde_json::from_str(&response).unwrap();


    match r {
        Message::Response(ip) => {
            println!("Response from {}:{}",srv.ip(),srv.port());
            println!("Your IP is: {}",ip);
        },
        _ => println!("Unkown response from remote server"),
    }

}


// fn verbose_log(verbose: u8, level: u8, message: String) {
//     if verbose >= level {
//         eprintln!("{message}");
//     }
// }