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

- [req](./tests/simple_req.cc)(connect, sync) <-> [rep](./tests/simple_rep.cc)(bind, sync)

- [dealer](./tests/simple_dealer.cc)(connect, async) <-> [router](./tests/simple_router_server.cc)(bind, async)

- [req](./tests/simple_req.cc)(connect, sync) <-> [router](./tests/simple_router_server.cc)(bind, async)

- [dealer](./tests/simple_dealer.cc)(connect, async) <-> [rep](./tests/simple_rep.cc)(bind, sync)

- [router_client](./tests/simple_router_client.cc)(connect, async) <-> [router_sever](./tests/simple_router_server.cc)(bind, async)

- [pub_server](./tests/simple_pub_server.cc)(bind) -> [sub_client](./tests/simple_sub_client.cc)(connect)

- [pub_client](./tests/simple_pub_client.cc)(connect) -> [sub_server](./tests/simple_sub_server.cc)(bind)

- [req_client](./tests/proxy_req_client.cc)(connect, sync) <-> [proxy_dealer_router](./tests/proxy_dealer_router.cc)(binds) <-> [rep_worker](./tests/proxy_rep_worker.cc)(connect, sync)

- [dealer_client](./tests/proxy_dealer_client.cc)(connect, async) <-> [proxy_dealer_router](./tests/proxy_dealer_router.cc)(binds) <-> [rep_worker](./tests/proxy_rep_worker.cc)(connect, sync)

- [req_client](./tests/proxy_req_client.cc)(connect, sync) <-> [proxy_dealer_router](./tests/proxy_dealer_router.cc)(binds) <-> [router_worker](./tests/proxy_router_worker.cc)(connect, async)

- [dealer_client](./tests/proxy_dealer_client.cc)(connect, async) <-> [proxy_dealer_router](./tests/proxy_dealer_router.cc)(binds) <-> [router_worker](./tests/proxy_router_worker.cc)(connect, async)

- [pub](./tests/proxy_pub.cc)(connect) -> [proxy_xpub_xsub](./tests/proxy_pub_sub.cc)(binds) -> [sub](./tests/proxy_sub.cc)(connect)
