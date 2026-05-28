use std::net::SocketAddrV4;
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

impl std::fmt::Display for FlowIdV4 {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let s = format!("{:<21} -> {:<21} ({:>3})", self.0, self.1, self.2);

        f.pad(&s)
    }
}
