use std::net::UdpSocket;
use clap::Parser;

#[derive(Parser, Debug)]
struct Cli {
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

    let srv_addr = format!("0.0.0.0:{}",cli.port);
    let sock = UdpSocket::bind(srv_addr).expect("Unable to bind socket");
    verbose_log(cli.verbose, 1, format!("Started server on {}:{}",sock.local_addr().unwrap().ip(),sock.local_addr().unwrap().port()));
    verbose_log(cli.verbose, 2, "Waiting for client queries...".to_string());

    loop {
        let mut buffer = [0_u8;1500];
        let (nb,clt_addr) = sock.recv_from(&mut buffer).unwrap();
        let m: Message = serde_json::from_str(str::from_utf8(&buffer[..nb]).unwrap()).unwrap();

        match m {
            Message::Query => {
                let response = Message::Response(format!("{}",clt_addr.ip()));
                let resp_jstr = serde_json::to_string(&response).unwrap();
                let _ = sock.send_to(resp_jstr.as_bytes(), clt_addr).unwrap();
            },
            Message::Response(_) => {
                verbose_log(cli.verbose, 3, format!("Received unexpected response from client: {}", clt_addr));
            }
        }

    }



}


fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}