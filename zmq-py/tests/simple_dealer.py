# @file:	simple_dealer.py
# @author:	Jacob Xie
# @date:	2025/02/06 14:30:20 Thursday
# @brief:   python zmq-py/tests/simple_dealer.py

import asyncio
import zmq
from zmq.asyncio import Context
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).absolute().parent.parent))
from common.const import EP


async def dealer(client_id):
    context = Context()

    # Create a DEALER socket
    dealer = context.socket(zmq.DEALER)

    # Set an identity for the client (optional)
    # dealer.setsockopt_string(zmq.IDENTITY, client_id)
    dealer.setsockopt(zmq.IDENTITY, client_id.encode("utf-8"))

    dealer.connect(EP)  # Connect to the ROUTER server

    print(f"Dealer client {client_id} is ready...")

    # for i in range(5):  # Send 5 messages
    for i in range(1):  # Send 5 messages
        message = f"Message {i + 1}".encode("utf-8")
        print(f"Sending message: {message.decode('utf-8')}")
        await dealer.send(message)  # Send the message

        # Wait for a reply
        reply = await dealer.recv_multipart()

        print(f"Received reply from server: {reply[0].decode('utf-8')}")

        await asyncio.sleep(1)  # Simulate some delay

    # Clean up
    dealer.close()
    context.term()


async def main():
    # Run multiple clients concurrently
    await asyncio.gather(
        dealer("client1"),
        dealer("client2"),
    )


if __name__ == "__main__":
    asyncio.run(main())
