use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};
use std::sync::{Arc, Mutex};
use std::{collections::HashMap, thread};

const SERVER_PORT: u16 = 7711;

struct Client {
    stream: TcpStream,
    nick: String,
}

struct Chat {
    clients: Arc<Mutex<HashMap<usize, Client>>>,
    next_id: usize,
}

impl Clone for Chat {
    fn clone(&self) -> Self {
        Self {
            clients: self.clients.clone(),
            next_id: self.next_id,
        }
    }
}

impl Chat {
    fn new() -> Self {
        Self {
            clients: Arc::new(Mutex::new(HashMap::new())),
            next_id: 0,
        }
    }

    fn add_client(&self, stream: TcpStream) -> usize {
        let id = stream
            .peer_addr()
            .expect("Failed to get peer address")
            .port() as usize;
        let mut clients = self.clients.lock().unwrap();
        clients.insert(
            id,
            Client {
                stream,
                nick: id.to_string(),
            },
        );
        drop(clients); // Release lock
        id
    }

    fn get_nick(&self, id: usize) -> String {
        let clients = self.clients.lock().unwrap();
        clients
            .get(&id)
            .unwrap_or_else(|| panic!("Don't find this {}", id))
            .nick
            .clone()
    }

    fn remove_client(&self, id: usize) {
        let mut clients = self.clients.lock().unwrap();
        clients.remove(&id);
        drop(clients); // Release lock
    }

    fn broadcast(&self, message: &str, excluded: usize) {
        let mut clients = self.clients.lock().unwrap();
        for (id, client) in clients.iter_mut() {
            if *id == excluded {
                continue;
            }
            let _ = client.stream.write(message.as_bytes());
        }
    }
}

fn handle_client(chat: Chat, mut stream: TcpStream, id: usize) {
    let welcome = "Welcome to chat! Please pick a nickname.\n";
    let _ = stream.write(welcome.as_bytes());

    let mut buffer = [0; 512];
    loop {
        let n = stream.read(&mut buffer).unwrap();

        if n == 0 {
            println!("Client {} disconnected", id);
            chat.remove_client(id);
            chat.broadcast(&format!("Client {} left", id), id);
            break;
        }
        let message = String::from_utf8_lossy(&buffer[..n]).to_string();
        let parts = message.splitn(2, ' ').collect::<Vec<&str>>();

        match parts[0] {
            "/nick" if parts.len() > 1 => {
                let new_nick = parts[1].trim().to_string();

                let mut clients = chat.clients.lock().unwrap();
                let client = clients.get_mut(&id).unwrap();
                client.nick = new_nick.clone();
                drop(clients); // Release lock
                chat.broadcast(&format!("Client {} is now known as {}", id, new_nick), id);
            }
            _ => {
                let msg = format!("user {}: {}", chat.get_nick(id), message);
                chat.broadcast(&msg, id);
            }
        }
    }
}

fn main() {
    let listener = TcpListener::bind("127.0.0.1:7711").expect("Failed to bind port");
    let chat = Chat::new();

    println!("Chat server listening on port {}", SERVER_PORT);

    for stream in listener.incoming() {
        let stream = stream.unwrap();
        let id = chat.add_client(stream.try_clone().unwrap());
        let chat_clone = chat.clone();
        println!("Client {} connected", id);

        thread::spawn(move || handle_client(chat_clone, stream, id));
    }
}
