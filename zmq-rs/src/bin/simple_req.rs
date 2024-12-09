//! file: simple_req.rs
//! author: Jacob Xie
//! date: 2024/12/05 16:33:31 Thursday
//! brief:

use tokio;
use zeromq::prelude::*;
use zeromq::ReqSocket;
use zeromq::ZmqMessage;
use zmq_todo::common::EP;

#[tokio::main]
// #[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // create a REQ (request) socket
    let mut socket = ReqSocket::new();

    // connect the socket to the server's address
    socket.connect(EP).await?;

    println!("Req connected to {}...", EP);

    // send a request to the server
    let msg = ZmqMessage::try_from("Hello, Rep!")?;
    socket.send(msg).await?;

    let rep: String = socket.recv().await?.try_into()?;
    println!("Received Rep: {}", rep);

    Ok(())
}
