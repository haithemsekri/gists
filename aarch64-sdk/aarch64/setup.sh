#!/bin/bash

CURR_DIR="$(dirname $(realpath $0))"

SetupToolchain() {
  rm -rf $CURR_DIR/gcc-aarch64-linux
  [[ ! -f $CURR_DIR/gcc-aarch64-linux.tar.xz ]] && wget https://github.com/haithemsekri/gists/releases/download/v0.1/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu.tar.xz -O $CURR_DIR/gcc-aarch64-linux.tar.xz
  mkdir $CURR_DIR/gcc-aarch64-linux.tmp
  tar -xf $CURR_DIR/gcc-aarch64-linux.tar.xz -C $CURR_DIR/gcc-aarch64-linux.tmp/
  dir_ls="$CURR_DIR/gcc-aarch64-linux.tmp/$(ls $CURR_DIR/gcc-aarch64-linux.tmp/)"
  [[  -d $dir_ls ]] && mv $dir_ls $CURR_DIR/gcc-aarch64-linux || mv $CURR_DIR/gcc-aarch64-linux.tmp $CURR_DIR/gcc-aarch64-linux
  rm -rf $CURR_DIR/gcc-aarch64-linux.tmp
}

[[ ! -d $CURR_DIR/gcc-aarch64-linux/bin ]] && SetupToolchain

env -i bash --init-file  <(echo export CURR_DIR=$CURR_DIR; cat $CURR_DIR/setup.env; cat $CURR_DIR/build.env; cat $CURR_DIR/../*.env)
