use std::{net::UdpSocket, str};

fn main() {
    // let srv_addr = SocketAddr::from((Ipv4Addr::UNSPECIFIED,20000));

    let socket = UdpSocket::bind("0.0.0.0:20000").expect("Unable to bind socket");

    loop {
        let mut buffer = [0_u8; 1500];
        let (size, clt) = socket.recv_from(&mut buffer).unwrap();

        let rx = str::from_utf8(&buffer[..size]).unwrap();
        println!("Received {} from clt: {}:{}", rx, clt.ip(), clt.port());
        let response = rx.to_uppercase();
        let _ = socket.send_to(response.as_bytes(), clt);
    }
}
