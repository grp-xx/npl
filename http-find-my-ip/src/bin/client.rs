
use clap::Parser;
#[derive(Parser, Debug)]
struct Cli {
    /// Server url 
    #[arg(short, long, default_value_t = String::from("http://127.0.0.1:10000/find-my-ip"))]
    url: String,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

mod utils;
use utils::Message;
fn main(){
    let cli = Cli::parse();

    let query = Message::Query;

    let mut response = ureq::post(&cli.url)
        .send_json(serde_json::to_value(query).unwrap()).unwrap();

    utils::verbose_log(cli.verbose, 3, format!("Server response status: {}", &response.status())); 
    utils::verbose_log(cli.verbose, 3, format!("Server response headers: {:#?}", &response.headers()));

    let response_body = response.body_mut().read_to_string().unwrap();
    utils::verbose_log(cli.verbose, 3, format!("Server response body: {}", &response_body));
    let msg: Message = serde_json::from_str(&response_body).unwrap();
    match msg {
        Message::Response(ip) => println!("Your IP address is: {}", ip),
        _ => eprintln!("Unexpected response format")
    }


}

