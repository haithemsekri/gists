#!/bin/bash -aux

CURR_DIR="$(dirname $(realpath $0))"

env -i bash --init-file  <(echo export CURR_DIR=$CURR_DIR; cat $CURR_DIR/*.env; cat $CURR_DIR/../*.env)
