//! file: simple_rep.rs
//! author: Jacob Xie
//! date: 2024/12/05 16:35:02 Thursday
//! brief:

use tokio;
use zeromq::prelude::*;
use zeromq::RepSocket;
use zeromq::ZmqMessage;
use zmq_todo::common::EP;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // create a REP (reply) socket
    let mut socket = RepSocket::new();

    // bind the socket to an address
    socket.bind(EP).await?;

    println!("Rep bound on {}...", EP);

    loop {
        // receive a message from the client
        let req: String = socket.recv().await?.try_into()?;
        println!("Received Req: {}", req);

        let rpl = ZmqMessage::try_from("Hello, Req!")?;
        // send a reply back to the client
        socket.send(rpl).await?;
    }
}
