# @file:	simple_pub.py
# @author:	Jacob Xie
# @date:	2025/02/06 17:48:17 Thursday
# @brief:

import asyncio
import zmq
from zmq.asyncio import Context
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).absolute().parent.parent))
from common.const import EP


async def publisher():
    context = Context()

    # Create a PUB socket
    pub = context.socket(zmq.PUB)
    pub.bind(EP)  # Bind to port
    print(f"Publisher is ready: {EP}")

    topic = "news"  # Default topic
    topic2 = "sports"
    count = 0

    while True:
        try:
            # Publish a message with a topic
            message = f"Message {count}".encode("utf-8")
            print(f"Publishing: [{topic}] {message.decode('utf-8')}")
            await pub.send_multipart([topic.encode("utf-8"), message])

            if count % 3 == 0:
                await pub.send_multipart([topic2.encode("utf-8"), message])

            count += 1

            await asyncio.sleep(1)  # Publish every second
        except KeyboardInterrupt:
            print("Publisher shutting down...")
            break

    # Clean up
    pub.close()
    context.term()


async def main():
    await publisher()


if __name__ == "__main__":
    asyncio.run(main())
