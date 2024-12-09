//! file: simple_push.rs
//! author: Jacob Xie
//! date: 2024/12/06 11:05:14 Friday
//! brief:

use zeromq::{PushSocket, Socket, SocketSend, ZmqMessage};
use zmq_todo::common::EP;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    println!("{:?}", args);

    let mut socket = PushSocket::new();

    if args.len() >= 2 && args[1] == "bind" {
        socket.bind(EP).await?;
        println!("Push bound to {}...", EP);
    } else {
        socket.connect(EP).await?;
        println!("Push connected to {}...", EP);
    }

    let mut idx = 0;
    loop {
        tokio::time::sleep(std::time::Duration::new(1, 0)).await;

        let m: String = format!("Hello, Pull! {}", idx);
        println!("{}", &m);
        let msg = ZmqMessage::try_from(m)?;
        if let Err(_) = socket.send(msg).await {
            continue;
        }
        idx += 1;
    }
}
