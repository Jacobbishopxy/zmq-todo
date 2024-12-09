//! file: simple_router.rs
//! author: Jacob Xie
//! date: 2024/12/05 17:16:12 Thursday
//! brief:

use bytes::Bytes;
use zeromq::{RouterSocket, Socket, SocketRecv, SocketSend, ZmqMessage};
use zmq_todo::common::EP;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut socket = RouterSocket::new();

    socket.bind(EP).await?;

    println!("Router bound on {}...", EP);

    loop {
        // possible msg pattern:
        // [dealer_id, payload]
        // [dealer_id, delimiter, payload]
        let mut msg = socket.recv().await?.into_vec();
        println!("Received Router: {:?}", msg);

        msg.last_mut().map(|_| Bytes::from("Hello, Dealer!"));
        let rpl = ZmqMessage::try_from(msg).unwrap();

        socket.send(rpl).await?;
    }
}
