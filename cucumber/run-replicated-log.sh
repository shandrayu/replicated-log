cd secondary
docker build --rm -t secondary-node .
cd -
cd master
docker build --rm -t master-node .
cd -
docker-compose up -d
