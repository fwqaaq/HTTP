use hyper::server::conn::AddrIncoming;
use hyper::service::{make_service_fn, service_fn};
use hyper::{Body, Request, Response, Server};
use hyper_rustls::TlsAcceptor;
use rustls::{Certificate, PrivateKey};
use std::{fs, io, net::SocketAddr};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let addr = SocketAddr::from(([127, 0, 0, 1], 9999));

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
fn load_certs(pathname: &str) -> io::Result<Vec<Certificate>> {
    rustls_pemfile::certs(&mut io::BufReader::new(fs::File::open(pathname)?))
        .map(|v| v.into_iter().map(Certificate).collect())
}

fn load_private_key(pathname: &str) -> io::Result<PrivateKey> {
    rustls_pemfile::pkcs8_private_keys(&mut io::BufReader::new(fs::File::open(pathname)?))
        .map(|v| rustls::PrivateKey(v[0].clone()))
}
