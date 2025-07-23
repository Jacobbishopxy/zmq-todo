# @file:	socket_monitor.py
# @author:	Jacob Xie
# @date:	2025/03/05 14:42:27 Wednesday
# @brief:   python zmq-py/tests/socket_monitor.py

import asyncio
from typing import Any, Dict

import zmq.asyncio
from zmq.utils.monitor import recv_monitor_message

EVENT_MAP = {}
print("Event names:")
for name in dir(zmq):
    if name.startswith("EVENT_"):
        value = getattr(zmq, name)
        print(f"{name:21} : {value:4}")
        EVENT_MAP[value] = name


class Server:
    def __init__(self):
        self.context = zmq.asyncio.Context()
        self.socket = self.context.socket(zmq.ROUTER)
        self.socket.bind("tcp://*:5555")

        # Enable monitoring
        self.monitor_socket = self.socket.get_monitor_socket()
        self.client_map = {}  # Map routing IDs to endpoints
        self.endpoint_map = {}  # Map endpoints to routing IDs

    async def handle_monitor(self):
        while True:
            try:
                # Receive monitoring event
                evt: Dict[str, Any] = {}
                mon_evt = await recv_monitor_message(self.monitor_socket)
                evt.update(mon_evt)
                evt["description"] = EVENT_MAP[evt["event"]]
                print(f"Event: {evt}")

                event_code = evt["event"]
                event_endpoint = evt["endpoint"]

                if event_code == zmq.EVENT_ACCEPTED:
                    print(f"New connection from: {event_endpoint}")
                    # Store the endpoint; routing ID will be associated later
                    self.endpoint_map[event_endpoint] = None
                elif event_code == zmq.EVENT_DISCONNECTED:
                    print(f"Disconnected: {event_endpoint}")
                    # Remove the endpoint and its associated routing ID
                    if event_endpoint in self.endpoint_map:
                        routing_id = self.endpoint_map[event_endpoint]
                        if routing_id in self.client_map:
                            del self.client_map[routing_id]
                        del self.endpoint_map[event_endpoint]
            except zmq.ZMQError as e:
                print(f"Monitor error: {e}")
                break

    async def handle_messages(self):
        while True:
            try:
                # Receive routing ID and message
                routing_id, message = await self.socket.recv_multipart()
                print(f"Received message from client {routing_id}: {message.decode('utf-8')}")

                # Associate routing ID with endpoint
                if routing_id not in self.client_map:
                    # Find the endpoint for this routing ID
                    endpoint = None
                    for ep, rid in self.endpoint_map.items():
                        if rid is None:
                            self.endpoint_map[ep] = routing_id
                            self.client_map[routing_id] = ep
                            endpoint = ep
                            break
                    if endpoint:
                        print(f"Associated routing ID {routing_id} with endpoint {endpoint}")

                # Send a response back to the client
                response = f"Hello, client {routing_id}"
                await self.socket.send_multipart([routing_id, response.encode("utf-8")])
            except zmq.ZMQError as e:
                print(f"Message handling error: {e}")
                break

    async def run(self):
        # Start monitor and message handlers concurrently
        await asyncio.gather(self.handle_monitor(), self.handle_messages())


async def main():
    server = Server()
    print("Server is running...")
    await server.run()


if __name__ == "__main__":
    asyncio.run(main())
