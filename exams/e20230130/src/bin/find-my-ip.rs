use std::net::{IpAddr, SocketAddr};

use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
struct Message {
    mtype: String,    //Query/Response
    serverip: IpAddr, //Source host
    sport: u16,       //Source port
    clientip: IpAddr, //Destination host
    cport: u16,       //Destination port
}
fn main() {
    let srv_ip = std::env::args().nth(1).unwrap();
    let srv_port = std::env::args().nth(2).unwrap();

    let srv_ip = srv_ip.parse::<IpAddr>().unwrap();
    let srv_port = srv_port.parse::<u16>().unwrap();

    let sock = std::net::UdpSocket::bind("0.0.0.0:0").unwrap();
    let srv = SocketAddr::new(srv_ip, srv_port);

    let msg = Message {
        mtype: "Query".to_string(),
        serverip: srv_ip,
        sport: srv_port,
        clientip: sock.local_addr().unwrap().ip(),
        cport: sock.local_addr().unwrap().port(),
    };

    // Convert the Message to a JSON string.
    let serialized = serde_json::to_string(&msg).unwrap();
    sock.send_to(serialized.as_bytes(), srv).unwrap();

    let mut buf = [0; 1024];
    let (size, _) = sock.recv_from(&mut buf).unwrap();

    let response: Message = serde_json::from_slice(&buf[..size]).unwrap();
    println!("Received response: {:?}", response);
    // Print my IP address
    println!("My IP address is: {}", response.clientip);
}
