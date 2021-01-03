#!/bin/bash

#set sdk path
export XDC_OBJECT_TYPE="LINUX-ARM"
export XDC_PROJECT_DIR=$(pwd)
export XDC_THIRDPARTY_DIR=$XDC_PROJECT_DIR/thirdparty
export XDC_FEATURE_DIR=$XDC_PROJECT_DIR/feature
export XDC_PROTOCOL_DIR=$XDC_PROJECT_DIR/protocol

source $XDC_FEATURE_DIR/arm_toolchain/environment-setup-cortexa7hf-neon-poky-linux-gnueabi


# export SDKTARGETSYSROOT=$ARM_LINUX_GCC_HOME/sysroot
# export PATH=$SDKTARGETSYSROOT/gcc-arm-linux-gnueabihf/bin:$PATH
# export CCACHE_PATH=$SDKTARGETSYSROOT/gcc-arm-linux-gnueabihf/bin:$CCACHE_PATH
# export CC="arm-linux-gnueabihf-gcc -mcpu=cortex-a9"
# export CXX="arm-linux-gnueabihf-g++ -mcpu=cortex-a9"
# export CPP="arm-linux-gnueabihf-gcc -E -mcpu=cortex-a9"
# export AS="arm-linux-gnueabihf-as"
# export LD="arm-linux-gnueabihf-ld  --sysroot=$SDKTARGETSYSROOT"
# export GDB=arm-linux-gnueabihf-gdb
# export STRIP=arm-linux-gnueabihf-strip
# export RANLIB=arm-linux-gnueabihf-ranlib
# export OBJCOPY=arm-linux-gnueabihf-objcopy
# export OBJDUMP=arm-linux-gnueabihf-objdump
# export AR=arm-linux-gnueabihf-ar
# export NM=arm-linux-gnueabihf-nm
# export M4=m4
# export TARGET_PREFIX=arm-linux-gnueabihf-
# export CFLAGS=" -O2 -pipe -g "
# export CXXFLAGS=" -O2 -pipe -g "
# export LDFLAGS="-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed"
# export CPPFLAGS=""
# export KCFLAGS="--sysroot=$SDKTARGETSYSROOT"
# export ARCH=armv7
# export CROSS_COMPILE=arm-linux-gnueabihf-
# export CPLUS_INCLUDE_PATH="$SDKTARGETSYSROOT/arm-linux-gnueabihf/include/c++/5.3.1/arm-linux-gnueabihf"


