/**
 * Hello World Example for BBB AI-64
 * 
 * This is a simple C program that demonstrates
 * cross-compilation for the BBB AI-64 platform.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

/**
 * Print system information
 */
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

/**
 * Main entry point
 */
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
    
    printf("Running on: ");
    #ifdef __arm__
    printf("ARM architecture\n");
    #endif
    #ifdef __aarch64__
    printf("ARM64 architecture\n");
    #endif
    #ifdef __x86_64__
    printf("x86_64 architecture\n");
    #endif
    
    printf("\n");
    printf("SDK Version: 1.0.0\n");
    printf("Build Date: %s %s\n", __DATE__, __TIME__);
    printf("\n");
    
    return 0;
}
