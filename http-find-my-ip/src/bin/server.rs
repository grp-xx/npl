use clap::Parser;
use tiny_http::{Header, Method, Response, Server};

#[derive(Parser, Debug)]
struct Cli {
    /// Server URI 
    #[arg(short, long, default_value_t = String::from("/find-my-ip"))]
    uri: String,
    /// Server port
    #[arg(short, long, default_value_t = 10000)] 
    port: u16,
    /// Verbose mode (-v, -vv, -vvv, etc.)
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,
}

mod utils;

fn handle_request(mut request: tiny_http::Request, uri: &str, verbose: u8) {
    let client_ip = request.remote_addr().unwrap().ip();
    let reply = utils::Message::Response(client_ip.to_string());
    let response_body = serde_json::to_string(&reply).unwrap();
    let response = Response::from_string(&response_body)
        .with_status_code(200)
        .with_header(
            Header::from_bytes(String::from("Content-Type"), String::from("application/json")).unwrap(),
        );

    match request.method() {
        Method::Post if request.url() == uri => {
            utils::verbose_log(
                verbose,
                2,
                format!("Received POST request for client IP: {}", client_ip),
            );

            let mut body = String::new();
            request
                .as_reader()
                .read_to_string(&mut body)
                .expect("Failed to read request body");
            utils::verbose_log(verbose, 2, format!("Received body: {}", body));
            let msg: utils::Message = serde_json::from_str(&body).unwrap();

            if let utils::Message::Query = msg {
                utils::verbose_log(
                    verbose,
                    2,
                    format!("Received query for client IP: {}", client_ip),
                );
                request.respond(response).expect("Failed to send response");
            } else {
                let response = Response::from_string("Invalid message format")
                    .with_status_code(400)
                    .with_header(
                        Header::from_bytes(String::from("Content-Type"), String::from("text/plain")).unwrap(),
                    );
                request.respond(response).expect("Failed to send response");
            }
        }

        Method::Get if request.url() == uri => {
            utils::verbose_log(
                verbose,
                2,
                format!("Received query for client IP: {}", client_ip),
            );
            request.respond(response).expect("Failed to send response");
        }

        _ => {
            let response = Response::from_string("Not Found")
                .with_status_code(404)
                .with_header(
                    Header::from_bytes(String::from("Content-Type"), String::from("text/plain")).unwrap(),
                );
            request.respond(response).expect("Failed to send response");
        }
    }
}


fn main() {
    let cli = Cli::parse();

    // Start the HTTP server    
    let server_addr = format!("0.0.0.0:{}", cli.port);
    let server = Server::http(&server_addr).expect("Unable to start HTTP server");

    utils::verbose_log(cli.verbose, 1, format!("Starting HTTP server at {}", server.server_addr().to_ip().unwrap()));
    utils::verbose_log(cli.verbose, 2, "Waiting for client queries...".to_string());

    for request in server.incoming_requests() {
        let uri = cli.uri.clone();
        std::thread::spawn(move || {                                // multithreaded version
            handle_request(request, &uri, cli.verbose);
        });
//        utils::handle_request(request, &cli.uri, cli.verbose);   // Single threaded version

    }

}
