FROM ubuntu:20.04

EXPOSE 45454

ENV WORKDIR="/opt" \
    APP_BIN="replicated_log_secondary" \
    OPTS="--host=0.0.0.0 --port 45454"

COPY bin/$APP_BIN $WORKDIR

WORKDIR $WORKDIR
ENTRYPOINT ./$APP_BIN $OPTS
