use std::{fs, io, net::SocketAddr};

use hyper::{
    server::conn::AddrIncoming,
    service::{make_service_fn, service_fn},
    Body, Request, Response, Server,
};
use hyper_rustls::TlsAcceptor;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let addr = SocketAddr::from(([0, 0, 0, 0], 8080));

    let (cert, key) = (
        load_certs("ssl/cert.pem")?,
        load_private_key("ssl/private.pem")?,
    );
    let incoming = AddrIncoming::bind(&addr)?;
    let acceptor = TlsAcceptor::builder()
        .with_single_cert(cert, key)
        .map_err(|e| io::Error::new(io::ErrorKind::Other, e))?
        .with_all_versions_alpn()
        .with_incoming(incoming);
    let service = make_service_fn(|_| async { Ok::<_, io::Error>(service_fn(handle_request)) });
    let server = Server::builder(acceptor).serve(service);

    println!("Listening on https://localhost:{}", addr.port());
    server.await?;

    Ok(())
}

async fn handle_request(req: Request<Body>) -> Result<Response<Body>, hyper::Error> {
    println!("Got request at {:#?}", req);
    Ok(Response::new(Body::from("Hello World!")))
}

fn load_certs(pathname: &str) -> io::Result<Vec<rustls::Certificate>> {
    let certfile = fs::File::open(pathname).expect("cannot open certfile");
    let mut reader = io::BufReader::new(certfile);

    let certs = rustls_pemfile::certs(&mut reader)
        .expect("cannot read certfile")
        .into_iter()
        .map(rustls::Certificate)
        .collect();
    Ok(certs)
}

fn load_private_key(pathname: &str) -> io::Result<rustls::PrivateKey> {
    let keyfile = fs::File::open(pathname).expect("cannot open keyfile");
    let mut reader = io::BufReader::new(keyfile);

    let keys = rustls_pemfile::pkcs8_private_keys(&mut reader).expect("cannot read keyfile");

    if keys.len() != 1 {
        panic!("expected a single private key,{}", keys.len());
    }

    Ok(rustls::PrivateKey(keys[0].clone()))
}
