
## sdk/docs/examples.md

```markdown
# BBB AI-64 SDK Examples

## Overview

This document provides examples for developing applications for the BBB AI-64 platform using the SDK.

## Example 1: Hello World

### C Program

```c
// hello.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

void print_system_info() {
    struct utsname info;
    if (uname(&info) == 0) {
        printf("System: %s\n", info.sysname);
        printf("Node: %s\n", info.nodename);
        printf("Release: %s\n", info.release);
        printf("Version: %s\n", info.version);
        printf("Machine: %s\n", info.machine);
    }
}

int main(int argc, char *argv[]) {
    printf("\n");
    printf("========================================\n");
    printf("  BBB AI-64 Hello World\n");
    printf("========================================\n");
    printf("\n");
    
    printf("Hello from the BBB AI-64!\n");
    printf("\n");
    
    print_system_info();
    printf("\n");
    
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    printf("\n");
    
    return 0;
}
