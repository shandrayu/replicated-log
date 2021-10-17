# Replicated log - secondary node imlementation

## Functionality

- `POST` - writes the message from request to internal memory
- `GET` - returns the string with concatenated messages

## Docker

### Run container and expose the port to localhost

```bash
docker run -p 4545:45454 secondary-node
```

### Send the packet to localhost, port 4545

```bash
curl -iv -X POST "http://localhost:4545" -d 'Test da86fdsfsd' -H "Connection: close"
```

## TODO

- json parsing
- indempotent add for messages
- format output as a json
- refactor the code
- Dockerfile - install dependencies and build binary inside the docker
