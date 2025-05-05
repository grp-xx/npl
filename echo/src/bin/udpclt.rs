use std::{net::{SocketAddr, UdpSocket}, str};

fn main() {
    // let srv_addr = SocketAddr::from((Ipv4Addr::UNSPECIFIED,20000));
    // let srv: SocketAddr = "127.0.0.1:20000".parse().unwrap(); // Importeded wrong module...
    let srv: SocketAddr = "127.0.0.1:20000".parse().unwrap(); 
    let socket = UdpSocket::bind("0.0.0.0:0").expect("Unable to bind socket");

    loop {
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).unwrap();
        let _ = socket.send_to(input.as_bytes(), srv);
        let mut buffer = [0_u8; 1500];

        let (size, _) = socket.recv_from(&mut buffer).unwrap();

        let rx = str::from_utf8(&buffer[..size]).unwrap();
        // println!("Received {} from clt: {}:{}",rx,clt.ip(),clt.port());
        println!("{}",rx);
    }
}