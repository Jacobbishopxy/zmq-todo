//! file: simple_sub.rs
//! author: Jacob Xie
//! date: 2024/12/06 10:21:22 Friday
//! brief:

use zeromq::{Socket, SocketRecv, SubSocket};
use zmq_todo::common::{EP, PUB_SUB_TOPIC};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    println!("{:?}", args);

    let mut socket = SubSocket::new();

    if args.len() >= 2 && args[1] == "bind" {
        socket.bind(EP).await?;
        println!("Sub bound to {}...", EP);
    } else {
        socket.connect(EP).await?;
        println!("Sub connected to {}...", EP);
    }

    // topic
    socket.subscribe(PUB_SUB_TOPIC).await?;

    loop {
        let msg = socket.recv().await?.into_vec();
        println!("Received Pub: {:?}", msg);
    }
}
