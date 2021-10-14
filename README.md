# replicated-log

The Replicated Log is a course project for distributed systems course in Ukrainian Catholic University. It has the following deployment architecture: one Master and any number of Secondaries. Master accepts write requests and replicates the message to all Secondaries.

## How it works

- External communication: REST
  - Master
    - POST - Append message
    - GET - returns list of messages
  - Secondary
    - GET - returns list of messages
- Node communication: REST

## Message format

`json` with the followind fields:

- message id, integer
- data payload, string

## Directory sctucture

Project contains one directory for each team member. Every directory contains master and secondary directory with corresponding implementation inside.
