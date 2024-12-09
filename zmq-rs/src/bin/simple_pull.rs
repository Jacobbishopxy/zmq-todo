//! file: simple_pull.rs
//! author: Jacob Xie
//! date: 2024/12/06 11:08:57 Friday
//! brief:

use zeromq::{PullSocket, Socket, SocketRecv};
use zmq_todo::common::EP;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    println!("{:?}", args);

    let mut socket = PullSocket::new();

    if args.len() >= 2 && args[1] == "connect" {
        socket.connect(EP).await?;
        println!("Pull connected to {}...", EP);
    } else {
        socket.bind(EP).await?;
        println!("Pull bound to {}...", EP);
    }

    loop {
        let msg: String = socket.recv().await?.try_into()?;
        println!("Received Push: {}", msg);
    }
}
