# Replicated log - master node implementation

## Functionality

- `POST` - writes the message to internal memory and sends the message to all registered secondaries. The new message with the same ID has no effect
- `GET` - returns the json with stored messages

## Docker

### Run container and expose the port to localhost

```bash
docker run -p 8888:8888 master-node
```

### Send the packet to localhost, port 88888

```bash
curl -iv -X POST "http://localhost:8888" -d '{"id" : 9, "message": "dsfdsmsg_log"}' -H "Connection: close"
```

### Get list of messages in json format

```bash
curl -iv -X GET "http://localhost:8888" -H "Connection: close" 
```

## TODO

- Refactor application class out of main.cpp
