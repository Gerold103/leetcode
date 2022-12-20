## Fibonacci numbers server-client library

The framework provides 2 libraries for client and server talking in Fibonacci
numbers.

The server listens on a given port. Connected clients send numbers N. The
responses contain N-th Fibonacci number + UTC timestamp of the request serving +
how many times the number was already requested (first counts as 1).

The project provides a framework (client and server libraries), and a couple of
example-executables with usage of those libs.

Mostly this is based on 'keyvaluestore' example from gRPC repository.

## Build and run

Requires only gRPC and all its dependencies.
Build steps:

* `$> mkdir installed` - do it right here, next to this README file;
* `$> export MY_INSTALL_DIR=$PWD/installed/`. It should point at the just
  created `installed` dir.
* Visit gRPC installation guide: https://grpc.io/docs/languages/cpp/quickstart/
  and perform the requires steps.
* `$> cd fibwork`.
* `$> mkdir build && cd build`.
* `$> cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ..`.
* `$> make -j`.

Now should be able to run the server and client exes and test various numbers:

**Server**:
```
$> ./server 3313 
```

**Client**:
```
$> ./client localhost:3313
1
1 = {fib = 1, ts = 1671567008, count = 1}
5
5 = {fib = 5, ts = 1671567023, count = 1}
20
20 = {fib = 6765, ts = 1671567035, count = 1}
15
15 = {fib = 610, ts = 1671567046, count = 1}
5
5 = {fib = 5, ts = 1671567057, count = 2}
90
90 = {fib = 2880067194370816120, ts = 1671567068, count = 1}
100
Error: 11: too big number
50
50 = {fib = 12586269025, ts = 1671567094, count = 1}
```
