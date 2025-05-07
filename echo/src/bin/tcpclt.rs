use std::{
    io::{self, Read, Write},
    net::{SocketAddr, TcpStream},
    str,
};

fn main() -> io::Result<()> {
    // let srv_addr = SocketAddr::from((Ipv4Addr::UNSPECIFIED,20000));
    // let srv: SocketAddr = "127.0.0.1:20000".parse().unwrap(); // Importeded wrong module...
    let srv: SocketAddr = "127.0.0.1:30000".parse().unwrap();
    let mut stream = TcpStream::connect(srv)?;

    loop {
        let mut input = String::new();
        std::io::stdin().read_line(&mut input).unwrap();
        let _ = stream.write(input.as_bytes());
        let mut buffer = [0_u8; 1500];

        let size = stream.read(&mut buffer).unwrap();

        let rx = str::from_utf8(&buffer[..size]).unwrap();
        // println!("Received {} from clt: {}:{}",rx,clt.ip(),clt.port());
        println!("{}", rx);
    }
}
