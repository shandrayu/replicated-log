# draft for docker image with build options
mkdir -p build || cd build
cmake ..
make
docker build -t secondary-node . 
