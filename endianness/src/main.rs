fn main() {
    let n = 1042_u16;
    let nn = n.to_ne_bytes();
    println!("{:?}",nn);

    println!("Native Endian: {}", n);
    println!("Big Endian: {}", n.to_be());
    println!("Little Endian: {}", n.to_le());

    if n == n.to_be() {
        println!("Big Endian");
    } else {
        println!("Your host is Little Endian");
    }


}
