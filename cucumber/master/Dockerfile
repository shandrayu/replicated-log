FROM ubuntu:20.04

EXPOSE 35353

ENV WORKDIR="/opt" \
    APP_BIN="replicated_log_master" \
    OPTS="--host=0.0.0.0 --port 35353"

COPY bin/$APP_BIN $WORKDIR

WORKDIR $WORKDIR
ENTRYPOINT ./$APP_BIN $OPTS
