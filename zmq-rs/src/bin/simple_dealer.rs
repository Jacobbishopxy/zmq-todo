//! file: simple_dealer.rs
//! author: Jacob Xie
//! date: 2024/12/05 17:16:02 Thursday
//! brief:

use tokio;
use zeromq::{DealerSocket, Socket, SocketRecv, SocketSend, ZmqMessage};
use zmq_todo::common::EP;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut socket = DealerSocket::new();

    socket.connect(EP).await?;
    println!("Dealer connected to {}...", EP);

    let msg = ZmqMessage::try_from("Hello, Router!")?;
    socket.send(msg).await?;

    let rep: String = socket.recv().await?.try_into()?;
    println!("Received Router {}", rep);

    Ok(())
}
