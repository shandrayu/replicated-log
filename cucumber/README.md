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
curl -iv -X POST "http://localhost:55555/" -d '{"write_concern": 4, "message": "my cool text message"}' -H "Connection: close"
```

### Get list of messages

```bash
curl -iv -X GET "http://localhost:55555/" -H "Connection: close"
```

## TODO

- Waiting for the response is implemented as "retry with increasing timeout"
  - CountDownLatch can be implemented
- Code organization
  - Code is in one file and has to be refactored for the further use
- No synchronization
  - Node's internal data storage is not protected by mutex or other synchronization primitive. While it's not an issue in this setup, the internal storage shall be guarded with mutex
- Docker networking
  - container builds and runs but static ip address and communication between containers is needed
- Add faulty message handling to the nodes

## Useful commands

### Docker

#### Run container and expose the port to localhost

```bash
docker run -p 4545:45454 secondary-node
```

#### Docker compose

```bash
docker-compose up -d
```

#### Build image

```bash
docker build -t secondary-node:1.1 .
```

#### Run image in interactive mode with a custom command

```bash
docker run -it master-node:1.1 bash
```

### Bash

#### Copy git changes preserving the directories

##### OSX

```bash
git status --porcelain | awk 'match($1, "M"){print $2}' | xargs -I % rsync -R % 
```

##### Linux

```bash
git status --porcelain | awk 'match($1, "M"){print $2}' | xargs -I % cp --parents % target/
```
