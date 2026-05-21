#!/bin/bash

# Setup build environment
export PATH=/home/yocto/poky/scripts:$PATH

# Source build environment if exists
if [ -f /home/yocto/poky/oe-init-build-env ]; then
    source /home/yocto/poky/oe-init-build-env /home/yocto/build > /dev/null 2>&1
fi

# Execute command
exec "$@"
