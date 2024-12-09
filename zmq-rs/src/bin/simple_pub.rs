//! file: simple_pub.rs
//! author: Jacob Xie
//! date: 2024/12/06 10:15:39 Friday
//! brief:

use bytes::Bytes;
use zeromq::{PubSocket, Socket, SocketSend, ZmqMessage};
use zmq_todo::common::{EP, PUB_SUB_TOPIC};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args: Vec<String> = std::env::args().collect();
    println!("{:?}", args);

    let mut socket = PubSocket::new();

    if args.len() >= 2 && args[1] == "connect" {
        // TODO
        // if sub rebind, this connection is invalid and should have a reconnect;
        // this is different from C++ behavior
        socket.connect(EP).await?;
        println!("Pub connected to {}...", EP);
    } else {
        socket.bind(EP).await?;
        println!("Pub bound to {}...", EP);
    }

    let mut message_count = 0;
    loop {
        tokio::time::sleep(std::time::Duration::new(2, 0)).await;

        let payload = format!("Message #{}", message_count);
        let log = format!("Published: [{}] {}", PUB_SUB_TOPIC, payload);

        // [topic, payload]
        let msg = vec![Bytes::from(PUB_SUB_TOPIC), Bytes::from(payload)];
        let msg = ZmqMessage::try_from(msg).unwrap();
        socket.send(msg).await?;

        println!("{}", log);

        message_count += 1;
    }
}
