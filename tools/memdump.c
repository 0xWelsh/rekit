#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

void dump_memory(pid_t pid, unsigned long addr, size_t size, const char *output) {
    FILE *out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "Error: Cannot create output file '%s': %s\n", output, strerror(errno));
        return;
    }
    
    printf("[*] Dumping 0x%lx bytes from 0x%lx\n", size, addr);
    
    for (size_t i = 0; i < size; i += sizeof(long)) {
        errno = 0;
        long data = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        if (errno != 0) {
            fprintf(stderr, "Error: Cannot read memory at 0x%lx: %s\n", addr + i, strerror(errno));
            fclose(out);
            return;
        }
        if (fwrite(&data, sizeof(long), 1, out) != 1) {
            fprintf(stderr, "Error: Write failed: %s\n", strerror(errno));
            fclose(out);
            return;
        }
    }
    
    fclose(out);
    printf("[+] Saved to %s\n", output);
}

void hex_dump(unsigned char *data, size_t size, unsigned long base_addr) {
    for (size_t i = 0; i < size; i += 16) {
        printf("0x%08lx: ", base_addr + i);
        
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            printf("%02x ", data[i + j]);
        }
        
        for (size_t j = size - i; j < 16; j++) {
            printf("   ");
        }
        
        printf(" |");
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            unsigned char c = data[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("|\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <pid> <address> <size> [output_file]\n", argv[0]);
        fprintf(stderr, "  pid: process ID to dump from\n");
        fprintf(stderr, "  address: memory address (hex, e.g., 0x400000)\n");
        fprintf(stderr, "  size: number of bytes (hex, e.g., 0x1000)\n");
        fprintf(stderr, "  output_file: optional file to save dump\n");
        fprintf(stderr, "\nExample: %s 1234 0x400000 0x1000 dump.bin\n", argv[0]);
        return 1;
    }
    
    pid_t pid = atoi(argv[1]);
    if (pid <= 0) {
        fprintf(stderr, "Error: Invalid PID\n");
        return 1;
    }
    
    // Check if process exists
    if (kill(pid, 0) != 0) {
        fprintf(stderr, "Error: Process %d not found or no permission: %s\n", pid, strerror(errno));
        return 1;
    }
    
    unsigned long addr = strtoul(argv[2], NULL, 16);
    size_t size = strtoul(argv[3], NULL, 16);
    
    if (size == 0) {
        fprintf(stderr, "Error: Invalid size\n");
        return 1;
    }
    
    if (size > 10 * 1024 * 1024) {  // 10MB limit
        fprintf(stderr, "Error: Size too large (max 10MB)\n");
        return 1;
    }
    
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        fprintf(stderr, "Error: Cannot attach to process: %s\n", strerror(errno));
        fprintf(stderr, "Hint: Try running with sudo\n");
        return 1;
    }
    
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "Error: waitpid failed: %s\n", strerror(errno));
        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        return 1;
    }
    
    if (argc > 4) {
        dump_memory(pid, addr, size, argv[4]);
    } else {
        unsigned char *data = malloc(size);
        if (!data) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            ptrace(PTRACE_DETACH, pid, NULL, NULL);
            return 1;
        }
        
        for (size_t i = 0; i < size; i += sizeof(long)) {
            errno = 0;
            long word = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
            if (errno != 0) {
                fprintf(stderr, "Error: Cannot read memory at 0x%lx: %s\n", addr + i, strerror(errno));
                free(data);
                ptrace(PTRACE_DETACH, pid, NULL, NULL);
                return 1;
            }
            memcpy(data + i, &word, sizeof(long));
        }
        hex_dump(data, size, addr);
        free(data);
    }
    
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    return 0;
}
