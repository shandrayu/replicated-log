cd secondary
docker build --rm -t secondary-node:1.1 .
cd -
cd master
docker build --rm -t master-node:1.0 .
cd -
docker-compose up -d
