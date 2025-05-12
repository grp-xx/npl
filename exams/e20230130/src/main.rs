use std::net::IpAddr;

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
    let sock = std::net::UdpSocket::bind("0.0.0.0:10000").unwrap();
//    let sock = std::net::UdpSocket::bind("192.168.1.34:10000").unwrap();
    println!("Listening on {}:{}", sock.local_addr().unwrap().ip(),sock.local_addr().unwrap().port());

    loop {
        let mut buf = [0; 1024];
        let (size, clt) = sock.recv_from(&mut buf).unwrap();
        let msg: Message = serde_json::from_slice(&buf[..size]).unwrap();
        println!("Received message: {:?}", msg);

        // Send a response back to the client
        let response = Message {
            mtype: "Response".to_string(),
            serverip: sock.local_addr().unwrap().ip(),
            sport: sock.local_addr().unwrap().port(),
            clientip: clt.ip(),
            cport: clt.port(),
        };

        let serialized = serde_json::to_string(&response).unwrap();
        sock.send_to(serialized.as_bytes(), clt).unwrap();
    }
}
