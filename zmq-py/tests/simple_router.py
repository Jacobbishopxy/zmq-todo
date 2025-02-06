# @file:	simple_router.py
# @author:	Jacob Xie
# @date:	2025/02/06 14:31:53 Thursday
# @brief:   python zmq-py/tests/simple_router.py

import asyncio
import zmq
from zmq.asyncio import Context
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).absolute().parent.parent))
from common.const import EP


async def router():
    context = Context()

    # Create a ROUTER socket
    router = context.socket(zmq.ROUTER)
    router.bind(EP)  # Bind to port
    print(f"Router is ready: {EP}")

    while True:
        try:
            # Receive a message from a DEALER client
            # The ROUTER socket prepends the client's identity
            identity, message = await router.recv_multipart()
            print(f"Received message from client {identity.decode('utf-8')}: {message.decode('utf-8')}")

            # Send a reply back to the client
            reply = f"Hello, client {identity}!".encode("utf-8")
            await router.send_multipart([identity, reply])
        except KeyboardInterrupt:
            print("Router shutting down...")
            break

    # Clean up
    router.close()
    context.term()


async def main():
    await router()


if __name__ == "__main__":
    asyncio.run(main())
