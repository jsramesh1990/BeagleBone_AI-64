# Getting Started with Yocto Platform

## Prerequisites

- Ubuntu 22.04 or later
- 100GB free disk space
- 8GB+ RAM
- 4+ CPU cores

## Quick Start

1. Clone this repository:
```bash
git clone <your-repo>
cd yocto-platform

 2.   Run setup:

bash

./setup.sh

 3.   Build your first image:

bash

./scripts/build.sh -d -m qemux86-64

 4.   Run in QEMU:

bash

runqemu nographic
