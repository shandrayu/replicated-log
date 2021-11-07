FROM ubuntu:20.04

EXPOSE 45454

ENV DEBIAN_FRONTEND=noninteractive \
    WORKDIR="/opt" \
    APP_BIN="replicated_log_secondary" \
    OPTS="--host=0.0.0.0 --port 45454"

RUN apt-get update && \
    apt-get install -y git cmake wget build-essential python3 zlib1g-dev
RUN cd $WORKDIR && \
    git clone https://github.com/shandrayu/replicated-log.git && \
    cd replicated-log && \
    git checkout update_cucumber_scripts && \
    cd cucumber && \
    ./setup.sh
RUN cd $WORKDIR/replicated-log/cucumber/third_party/mif && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make
RUN cd $WORKDIR/replicated-log/cucumber/secondary && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

WORKDIR $WORKDIR
ENTRYPOINT .$WORKDIR/replicated-log/cucumber/secondary/bin/$APP_BIN $OPTS
