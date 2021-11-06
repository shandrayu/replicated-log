# C++ implementation with dependencies

## Prerequisites

Run `setup.sh`

The script downloads all third-party libraries and runs build.

## How to build

```bash
cd secondary
mkdir build
cd build
cmake ..
make
```

## How to test

### Send message to the node

```bash
curl -iv -X POST "http://localhost:55555/" -d 'json with message' -H "Connection: close"
```

### Docker compose

```bash
    docker-compose up -d
```

### Get list of messages

```bash
curl -iv -X GET "http://localhost:55555/" -H "Connection: close"
```

## TODO

- Implements correct handling for mif library
