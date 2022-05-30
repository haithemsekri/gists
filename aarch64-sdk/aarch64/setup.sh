#!/bin/bash

export CURR_DIR="$(dirname $(realpath $0))"
##export ARCH=aarch64
_link="$CURR_DIR/gcc-$ARCH-linux"

SetupToolchain() {
  local _arch="$1"
  [[ "$_arch" == "aarch64" ]] && local _url="https://snapshots.linaro.org/gnu-toolchain/11.2-2022.02-1/aarch64-linux-gnu/gcc-linaro-11.2.1-2022.02-x86_64_aarch64-linux-gnu.tar.xz"
  [[ "$_arch" == "arm" ]] && local _url="https://snapshots.linaro.org/gnu-toolchain/11.2-2022.02-1/arm-linux-gnueabihf/gcc-linaro-11.2.1-2022.02-x86_64_arm-linux-gnueabihf.tar.xz"
  local _tar="$CURR_DIR/$(basename $_url)"
  local _dir="$(echo $_tar | sed 's|\.tar\.xz||g')"

  [[ -d $_link/bin ]] && return 0
  unlink $_link
  rm -rf $_dir
  [[ ! -f $_tar ]] && wget $_url -O $_tar
  tar -xf $_tar -C $CURR_DIR/
  ln -srf $_dir $_link
}

SetupToolchain $ARCH

[[ ! -d $_link/bin ]] && SetupToolchain
export S="$_link-sysroot"
unlink $S
ln -srf "$(realpath $($_link/bin/${ARCH}-*-gcc -print-sysroot))"  $S

$_link/bin/${ARCH}-*-gcc --version
echo TOOLCHAIN:$(ls -l $_link | awk '{print $9, $10, $11, $12}' | sed "s|$CURR_DIR/||g")
echo SYSROOT:$(ls -l $S | awk '{print $9, $10, $11, $12}' | sed "s|$CURR_DIR/||g")

env -i bash --init-file <(
  echo export CURR_DIR=$CURR_DIR;
  echo export ARCH=$ARCH;
  echo . $CURR_DIR/../*.env;
  echo export PATH=$(realpath $CURR_DIR/../bin):$(realpath $CURR_DIR/native_bin):$(realpath $_link/bin);
  echo export S=$S;
  echo . $CURR_DIR/build.env;
)
