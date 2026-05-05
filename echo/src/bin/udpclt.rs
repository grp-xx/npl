use std::net::{SocketAddr, UdpSocket};
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

    let clt_addr: SocketAddr = "0.0.0.0:0".parse().unwrap();

    let srv_sock: SocketAddr = format!("{}:{}", cli.server, cli.port).parse().unwrap();

//    In case we want to support hostnames instead of IP addresses, we can use the following code to resolve the server address
//    let srv_sock: SocketAddr = format!("{}:{}", cli.server, cli.port)
//        .to_socket_addrs().unwrap()
//        .find(|addr| addr.is_ipv4())
//        .ok_or_else(|| std::io::Error::new(
//            std::io::ErrorKind::AddrNotAvailable,
//            "no IPv4 address found",
//        )).unwrap();

    let sock = UdpSocket::bind(clt_addr).unwrap();
    let local_socket = sock.local_addr().unwrap();
    verbose_log(cli.verbose, 1, format!("UDP client started on {}:{}", local_socket.ip(), local_socket.port()));

    
    loop {
        let mut input = String::new();
        let nn = std::io::stdin().read_line(&mut input).unwrap();

        if nn == 0 {
            verbose_log(cli.verbose, 1, "EOF received, exiting".to_string()); // EOF: CTRL+D
            break;
        }

        let _ = sock.send_to(input.as_bytes(), srv_sock).unwrap();
        
        let mut buffer = [0_u8;1500];
        let (nbytes, _) = sock.recv_from(&mut buffer).unwrap();

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