use std::{io::{BufRead, BufReader, Write}, net::{SocketAddr, TcpListener}};
// use std::net::ToSocketAddrs;
use clap::Parser;


#[derive(Parser, Debug)]
struct Cli {
    /// Server host
    #[arg(long, default_value = "127.0.0.1")]
    host: String,
    /// Server port
    #[arg(short, long, default_value_t = 10000)]
    port: u16,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}



fn main () {
    let cli = Cli::parse();

    let srv_sock: SocketAddr = format!("{}:{}", cli.host, cli.port).parse().unwrap();

    let sock = TcpListener::bind(srv_sock).expect("Unable to open socket");

    verbose_log(cli.verbose, 1, format!("TCP server started on {}:{}", sock.local_addr().unwrap().ip(), cli.port));


    for connection in sock.incoming() {
        match connection {
            Ok(stream ) => handle_client(stream, cli.verbose),
            Err(e) => { 
                eprint!("Cannot Handle Client: {}", e);
                continue;
            }
        }
    }


}


fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}

fn handle_client(mut stream: std::net::TcpStream, verbose: u8) {
    let clt = stream.peer_addr().unwrap();
    let message = format!("Connected to Client {}:{}", clt.ip(), clt.port());
    verbose_log(verbose, 1, message);
    let cloned_stream = stream.try_clone().expect("Failed to clone stream");
    let reader = BufReader::new(&cloned_stream);

    for line in reader.lines() {
        let s = &line.unwrap()[..];
        let response = s.to_uppercase();
        let _ = stream.write_all(response.as_bytes()).expect("Can't send response");
    }
}