use std::{net::{SocketAddr, UdpSocket}};
// use std::net::ToSocketAddrs;
use clap::{Parser};


#[derive(Parser, Debug)]
struct Cli {
    /// Server port
    #[arg(short, long, default_value_t = 10000)]
    port: u16,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

fn main() {
    let cli = Cli::parse();
    let sock_addr: SocketAddr = SocketAddr::from(([0,0,0,0], cli.port));
    let lst_sock = UdpSocket::bind(sock_addr).expect("Failed to bind socket");
    verbose_log(cli.verbose, 1, format!("Listening for news on {}", lst_sock.local_addr().unwrap()));

    loop {
        let mut buf = [0_u8;1500];
        match lst_sock.recv_from(&mut buf) {
            Ok((n,srv)) => {
                let msg = std::str::from_utf8(&buf[..n]).unwrap();
                println!("News: {} from {}", msg.trim_end(), srv);
            }, 
            Err(e) => eprintln!("Error in receiving news {}",e), 
        }
    }
}

fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}