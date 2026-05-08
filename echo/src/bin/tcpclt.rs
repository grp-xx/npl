use std::{io::Write, io::Read, net::{SocketAddr, TcpStream}};
// use std::net::ToSocketAddrs;
use clap::Parser;


#[derive(Parser, Debug)]
struct Cli {
    /// Server host 
    #[arg(short, long, default_value = "127.0.0.1")]
    server: String,
    /// Server port
    #[arg(short, long, default_value_t = 10000)]
    port: u16,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}


fn main() {
    let cli = Cli::parse();
    // let clt_addr = SocketAddr::new(IpAddr::V4(Ipv4Addr::UNSPECIFIED),0);

    let srv_sock: SocketAddr = format!("{}:{}", cli.server, cli.port).parse().unwrap();

    let mut stream = TcpStream::connect(srv_sock).unwrap();
    let local_socket = stream.local_addr().unwrap();
    verbose_log(cli.verbose, 1, format!("TCP client started on {}:{}", local_socket.ip(), local_socket.port()));

    
    loop {
        let mut input = String::new();
        let nn = std::io::stdin().read_line(&mut input).unwrap();

        if nn == 0 {
            verbose_log(cli.verbose, 1, "EOF received, exiting".to_string()); // EOF: CTRL+D
            break;
        }

        let _ = stream.write_all(input.as_bytes()).unwrap();
        
        let mut buffer = [0_u8;1500];

        let nbytes= stream.read(&mut buffer).unwrap();

        if nbytes == 0 {
            verbose_log(cli.verbose, 1, "Server closed connection".to_string());
            break;
        }
        
        let response = std::str::from_utf8(&buffer[..nbytes]).unwrap();
        print!("{}",response);
//      Note: Could have used print!(), but in this way the output may not appear immediately unless stdout is flushed, especially when printing prompts or progress text.
    }
}

fn verbose_log(verbose: u8, level: u8, message: String) {
    if verbose >= level {
        eprintln!("{message}");
    }
}