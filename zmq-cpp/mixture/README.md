# Mixture

```sh
# broker
./build/zmq-cpp/mixture/x_broker

# worker
./build/zmq-cpp/mixture/x_worker --worker_id=my_worker

# client
./build/zmq-cpp/mixture/x_client --client_id=my_client --worker_id=my_worker --action=GET_ALL

./build/zmq-cpp/mixture/x_client --client_id=my_client --worker_id=my_worker --action=CREATE \
  --payload='{"id": 0, "description": "test zmq", "completed": false}'

./build/zmq-cpp/mixture/x_client --client_id=my_client --worker_id=my_worker --action=GET \
  --payload=0

./build/zmq-cpp/mixture/x_client --client_id=my_client --worker_id=my_worker --action=MODIFY \
  --payload='{"id": 0, "description": "test zmq!", "completed": true}'

```
