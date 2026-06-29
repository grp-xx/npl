use std::net::SocketAddrV4;
use std::net::{IpAddr, ToSocketAddrs};
use std::io::Result;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct FlowIdV4(pub SocketAddrV4, pub SocketAddrV4, pub u8);

impl FlowIdV4 {
    pub fn new(src: &SocketAddrV4, dst: &SocketAddrV4, proto: u8) -> Self {
        if src <= dst {
            Self(*src, *dst, proto)
        } else {   
        Self(*dst, *src, proto)
        }
    }
}

//impl std::fmt::Display for FlowIdV4 {
//    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
//        write!(
//            f,
//            "{} -> {} (proto {})",
//            self.0,
//            self.1,
//            self.2
//        )
//    }
//}


impl std::fmt::Display for FlowIdV4 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let s = format!("{:<21} -> {:<21} ({:>3})", self.0, self.1, self.2);

        f.pad(&s)
    }
}


pub fn hostname_to_ipv4(hostname: &str) -> std::io::Result<Vec<std::net::Ipv4Addr>> {
    let addrs = (hostname, 0).to_socket_addrs()?;

    let ipv4_addrs = addrs
        .filter_map(|addr| match addr.ip() {
            IpAddr::V4(ipv4) => Some(ipv4),
            IpAddr::V6(_) => None,
        })
        .collect();

    Ok(ipv4_addrs)
}

pub fn get_ipv4_addr_from_udp_socket(socket: &std::net::UdpSocket) -> Result<String> {
    let ip = socket.local_addr()?.ip();
    let port = match socket.local_addr()?.port() {
        50000 => 51000,
        _ => 50000,
    };
    let addr = std::net::SocketAddr::new(ip, port);
    let sock = std::net::UdpSocket::bind(addr)?;
    let remote = "8.8.8.8:53";
    sock.connect(remote)?;
    let local =     sock.local_addr()?;
    Ok(local.ip().to_string())
}