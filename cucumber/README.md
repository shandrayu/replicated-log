# C++ implementation with dependencies

## Prerequisites

1. Run `setup.sh`. The script downloads all third-party libraries.

2. Build `mif` library.

```bash
cd third_party/mif
mkdir build
cd build
cmake ..
make
```

## How to build

```bash
./build_nodes.sh
```

## How to test

### Send message to the node

```bash
curl -iv -X POST "http://localhost:55555/" -d 'json with message' -H "Connection: close"
```

### Get list of messages

```bash
curl -iv -X GET "http://localhost:55555/" -H "Connection: close"
```

## Docker - useful commands

### Run container and expose the port to localhost

```bash
docker run -p 4545:45454 secondary-node
```

### Docker compose

```bash
docker-compose up -d
```

### Build image

```bash
docker build -t secondary-node:1.1 .
```

## TODO

- Implements correct handling for mif library dependencies
- Replace mif library to other http server
- Fix docker networking
- Refactor the code (master and secondary)
- Refactor application class out of main.cpp
- Add faulty message handling to the nodes
- Create two Message classes: internal and external
- Introduce GRPC for internal communication
- Pass write concern as a message parameter (or as a constant parameter in classes)
- Write concern: how to remove messages from a secondary node if write concern is not met? Create one more buffer?
