# Replicated log - secondary node imlementation

## Functionality

- `POST` - writes the message to internal memory. The new message with the same ID has no effect
- `GET` - returns the json with stored messages

## Docker

### Run container and expose the port to localhost

```bash
docker run -p 4545:45454 secondary-node
```

### Send the packet to localhost, port 4545

```bash
curl -iv -X POST "http://localhost:4545" -d '{"id" : 9, "message": "dsfdsmsg_log"}' -H "Connection: close"
```

### Get list of messages in json format

```bash
curl -iv -X GET "http://localhost:4545" -H "Connection: close" 
```

## TODO

- refactor the code
- Dockerfile - install dependencies and build binary inside the docker
- Add faulty message handling to the nodes
