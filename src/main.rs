use std::net::SocketAddr;

use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let addr = SocketAddr::from(([0, 0, 0, 0], 8080));
    let listener = TcpListener::bind(addr).await?;
    println!("List ening on http://azure.fwqaq.us:{}", addr.port());

    loop {
        let (mut socket, _) = listener.accept().await?;
        tokio::spawn(async move {
            let mut buf = [0; 1024];
            loop {
                let n = match socket.read(&mut buf).await {
                    Ok(0) => return,
                    Ok(n) => n,
                    Err(e) => {
                        eprintln!("failed to read from socket; err = {:?}", e);
                        return;
                    }
                };
                println!("Content: {}", String::from_utf8_lossy(&buf[..n]));
                let response = b"HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello world!";
                if let Err(e) = socket.write_all(response).await {
                    eprintln!("failed to write to socket; err = {:?}", e);
                }
            }
        });
        println!("Accepted connection");
    }
}
