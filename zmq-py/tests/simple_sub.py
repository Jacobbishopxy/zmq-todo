# @file:	simple_sub.py
# @author:	Jacob Xie
# @date:	2025/02/06 17:49:27 Thursday
# @brief:

import asyncio
import zmq
from zmq.asyncio import Context
import sys
from pathlib import Path

sys.path.append(str(Path(__file__).absolute().parent.parent))
from common.const import EP


async def subscriber(subscriber_id, topic_filter):
    context = Context()

    # Create a SUB socket
    sub = context.socket(zmq.SUB)
    sub.connect(EP)  # Connect to the PUB server

    # Subscribe to a specific topic
    sub.setsockopt(zmq.SUBSCRIBE, topic_filter.encode("utf-8"))
    print(f"Subscriber {subscriber_id} is ready and subscribed to topic '{topic_filter}'...")

    while True:
        try:
            # Receive a message
            topic, message = await sub.recv_multipart()
            print(f"Subscriber {subscriber_id} received: [{topic.decode('utf-8')}] {message.decode('utf-8')}")
        except KeyboardInterrupt:
            print(f"Subscriber {subscriber_id} shutting down...")
            break

    # Clean up
    sub.close()
    context.term()


async def main():
    # Run multiple subscribers concurrently
    await asyncio.gather(
        subscriber("sub1", "news"),  # Subscriber 1 subscribes to "news"
        subscriber("sub2", "sports"),  # Subscriber 2 subscribes to "sports"
        subscriber("sub3", ""),  # Subscriber 3 subscribes to all topics
    )


if __name__ == "__main__":
    asyncio.run(main())
