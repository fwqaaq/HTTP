use std::io::{Read, Write};
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::atomic::Ordering::Relaxed;
use std::sync::{atomic::AtomicI16, Arc, Mutex};
use std::{collections::HashMap, thread};

// const MAX_CLIENTS: u16 = 1000;
const SERVER_PORT: u16 = 7711;

struct Client {
    stream: TcpStream,
    nick: String,
}

struct Chat {
    clients: HashMap<i16, Client>,
    next_id: AtomicI16,
}

impl Chat {
    fn new() -> Self {
        Self {
            clients: HashMap::new(),
            next_id: AtomicI16::new(0),
        }
    }

    fn add_client(&mut self, stream: TcpStream) -> i16 {
        let prev_id = self.next_id.fetch_add(1, Relaxed);
        let client = Client {
            stream,
            nick: format!("user:{}", prev_id),
        };
        self.clients.insert(prev_id, client);
        prev_id
    }

    fn remove_client(&mut self, id: i16) {
        self.clients.remove(&id);
    }

    fn broadcast(&mut self, message: &str, excluded: i16) {
        for (id, client) in self.clients.iter_mut() {
            if *id == excluded {
                continue;
            }
            client
                .stream
                .write_fmt(format_args!("{}\n", message))
                .unwrap();
        }
    }
}

fn main() {
    let socket_addr = SocketAddr::from(([0, 0, 0, 0], SERVER_PORT));
    let listener = TcpListener::bind(socket_addr).unwrap();

    println!("Server listening on port {}", SERVER_PORT);

    let chat = Arc::new(Mutex::new(Chat::new()));
    for stream in listener.incoming() {
        let mut stream = stream.expect("Failed to unwrap stream");
        let clone_stream = stream.try_clone().expect("Failed to clone stream");

        let chat_clone = chat.clone();
        let mut chat = chat.lock().unwrap();
        let id = chat.add_client(clone_stream);

        println!("Client {} connected", id);

        chat.broadcast(&format!("{} joined", id), id);

        let welcome = "Welcome to Rust Chat! Use /nick <newnick> to change your nickname.\n";
        // Connect to the client
        stream
            .write_all(welcome.as_bytes())
            .expect("Failed to write to client");

        let mut buffer = [0; 512];

        thread::spawn(move || loop {
            let n = stream.read(&mut buffer).unwrap();
            let mut chat_clone = chat_clone.lock().unwrap();
            if n == 0 {
                println!("Client {} disconnected", id);
                chat_clone.remove_client(id);
                chat_clone.broadcast(&format!("{} left", id), id);
                break;
            }

            let message = String::from_utf8_lossy(&buffer[..n]);

            if message.starts_with("/nick") {
                let parts = message.splitn(2, ' ').collect::<Vec<&str>>();
                if parts.len() > 1 {
                    let client = chat_clone.clients.get_mut(&id).unwrap();
                    client.nick = parts[1].to_string();
                }
            } else {
                chat_clone.broadcast(&format!("user {}: {}", id, message), id);
            }
        });
    }
}
