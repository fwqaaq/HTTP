use std::net::SocketAddr;

use hyper::client::conn::Builder;
use hyper::server::conn::Http;
use hyper::service::service_fn;
use hyper::upgrade::Upgraded;
use hyper::{Body, Method, Request, Response};
use tokio::net::{TcpListener, TcpStream};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let addr = SocketAddr::from(([0, 0, 0, 0], 8080));
    let listener = TcpListener::bind(&addr).await?;

    println!("Listening on http://localhost:{}", addr.port());

    loop {
        let (stream, _) = listener.accept().await?;

        tokio::spawn(async move {
            let conn = Http::new()
                .serve_connection(stream, service_fn(proxy))
                .with_upgrades();
            if let Err(e) = conn.await {
                eprintln!("server error: {}", e);
            }
        });
    }
}

async fn proxy(mut req: Request<Body>) -> Result<Response<Body>, hyper::Error> {
    // println!("req: {:?}", req);

    // Remove proxy-connection header
    req.headers_mut().remove("proxy-connection");
    let req_method = req.method();
    let req_addr = req.uri().authority().map(|a| a.to_string());

    // Handle HTTPS Proxy Request
    if Method::CONNECT == req_method {
        if let Some(addr) = req_addr {
            tokio::task::spawn(async move {
                match hyper::upgrade::on(req).await {
                    Ok(upgraded) => {
                        if let Err(e) = tunnel(upgraded, addr).await {
                            eprintln!("server io error: {}", e);
                        };
                    }
                    Err(e) => eprintln!("upgrade error: {}", e),
                }
            });

            Ok(Response::new(Body::empty()))
        } else {
            eprintln!("CONNECT host is not socket addr: {:?}", req.uri());
            let mut resp = Response::new("CONNECT must be to a socket address".into());
            *resp.status_mut() = http::StatusCode::BAD_REQUEST;

            Ok(resp)
        }
    } else {
        // Handle HTTP Proxy Request
        let (host, port) = (
            req.uri().host().expect("uri has no host"),
            req.uri().port_u16().unwrap_or(80),
        );
        let addr = format!("{}:{}", host, port);

        let stream = TcpStream::connect(addr).await.unwrap();

        let (mut sender, conn) = Builder::new()
            .http1_preserve_header_case(true)
            .http1_title_case_headers(true)
            .handshake(stream)
            .await?;
        tokio::task::spawn(async move {
            if let Err(err) = conn.await {
                println!("Connection failed: {:?}", err);
            }
        });

        let resp = sender.send_request(req).await?;
        Ok(resp)
    }
}

async fn tunnel(mut upgraded: Upgraded, addr: String) -> std::io::Result<()> {
    // Connect to remote server
    let mut server = TcpStream::connect(addr).await?;

    // Proxying data
    let (from_client, from_server) =
        tokio::io::copy_bidirectional(&mut upgraded, &mut server).await?;

    // Print message when done
    println!(
        "client wrote {} bytes and received {} bytes",
        from_client, from_server
    );

    Ok(())
}
