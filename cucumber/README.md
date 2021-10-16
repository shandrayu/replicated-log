# C++ implementation with dependencies

## Prerequisites

Run `download_third_party.sh`

## How to build

```bash
cd secondary
mkdir build
cd build
cmake ..
make
```

## How to test

### Send message

```bash
curl -iv -X POST "http://localhost:55555/" -d 'json with message' -H "Connection: close"
```

### Get list of messages (string with concatenated data from all messages, undtructured)

```bash
curl -iv -X GET "http://localhost:55555/" -H "Connection: close"
```
