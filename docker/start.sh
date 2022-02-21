#! /bin/bash

DIR=$(dirname $0)

. /opt/esp/entrypoint.sh
make -C ${DIR}/.. new-build
