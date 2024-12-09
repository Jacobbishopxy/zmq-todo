# Zmq CPP

## Deps

```sh
# Ubuntu
apt-get install libzmq-dev
# MacOS
brew install zeromq

# clone repo & build
git clone git@github.com:zeromq/cppzmq.git
cd cppzmq
mkdir build
cmake ..
sudo make -j install
```

## Examples

- [req](./simple_req.cc)(connect, sync) <-> [rep](./simple_rep.cc)(bind, sync)

- [dealer](./simple_dealer.cc)(connect, async) <-> [router](./simple_router_server.cc)(bind, async)

- [req](./simple_req.cc)(connect, sync) <-> [router](./simple_router_server.cc)(bind, async)

- [dealer](./simple_dealer.cc)(connect, async) <-> [rep](./simple_rep.cc)(bind, sync)

- [router_client](./simple_router_client.cc)(connect, async) <-> [router_sever](./simple_router_server.cc)(bind, async)

- [pub_server](./simple_pub_server.cc)(bind) -> [sub_client](./simple_sub_client.cc)(connect)

- [pub_client](./simple_pub_client.cc)(connect) -> [sub_server](./simple_sub_server.cc)(bind)

- [req_client](./proxy_req_client.cc)(connect) <-> [proxy_dealer_router](./proxy_dealer_router.cc)(binds) <-> [rep_worker](./proxy_rep_worker.cc)(connect)

- [pub](./proxy_pub.cc)(connect) -> [proxy_xpub_xsub](./proxy_pub_sub.cc)(binds) -> [sub](./proxy_sub.cc)(connect)
