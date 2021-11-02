FROM ubuntu:20.04

RUN apt-get update && apt-get install -y libssl-dev

EXPOSE 35353

ENV WORKDIR="/opt" \
    APP_BIN="replicated_log_master" \
    OPTS="--host=0.0.0.0 --port=35353 --secondarynodes=0.0.0.0:45454" \
    LD_LIBRARY_PATH="/lib"

COPY bin/$APP_BIN $WORKDIR
# TODO: how to manage linking to library properly?
COPY build/_deps/cpr-build/cpr/libcpr.so $LD_LIBRARY_PATH
COPY build/_deps/cpr-build/cpr/libcpr.so.1 $LD_LIBRARY_PATH
COPY build/_deps/cpr-build/cpr/libcpr.so.1.6 $LD_LIBRARY_PATH
COPY build/_deps/curl-build/lib/libcurl-d.so $LD_LIBRARY_PATH

WORKDIR $WORKDIR
ENTRYPOINT ./$APP_BIN $OPTS
