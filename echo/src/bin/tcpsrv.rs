use core::str;
use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};

fn handle_client(mut stream: TcpStream) {
    loop {
        let mut buffer = [0u8; 1500];
        let size = stream.read(&mut buffer).unwrap();
        let response = str::from_utf8(&buffer[..size]).unwrap();
        stream.write(response.to_uppercase().as_bytes()).unwrap();
    }
}

fn main() {
    let listener = TcpListener::bind("0.0.0.0:30000").unwrap();

    for connection in listener.incoming() {
        match connection {
            Ok(stream) => { 
                let clt = stream.peer_addr().unwrap();
                println!{"Connected to {}:{}",clt.ip(),clt.port()};
                std::thread::spawn(move || handle_client(stream));
            },
            Err(e) => {
                eprintln!("Error {}", e);
                continue;
            }
        }
    }
}
