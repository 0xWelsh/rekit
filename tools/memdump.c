#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

void dump_memory(pid_t pid, unsigned long addr, size_t size, const char *output) {
    FILE *out = fopen(output, "wb");
    if (!out) {
        perror("fopen");
        return;
    }
    
    printf("[*] Dumping 0x%lx bytes from 0x%lx\n", size, addr);
    
    for (size_t i = 0; i < size; i += sizeof(long)) {
        long data = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
        fwrite(&data, sizeof(long), 1, out);
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
        printf("Usage: %s <pid> <address> <size> [output_file]\n", argv[0]);
        printf("Example: %s 1234 0x400000 0x1000 dump.bin\n", argv[0]);
        return 1;
    }
    
    pid_t pid = atoi(argv[1]);
    unsigned long addr = strtoul(argv[2], NULL, 16);
    size_t size = strtoul(argv[3], NULL, 16);
    
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("ptrace attach");
        return 1;
    }
    
    waitpid(pid, NULL, 0);
    
    if (argc > 4) {
        dump_memory(pid, addr, size, argv[4]);
    } else {
        unsigned char *data = malloc(size);
        for (size_t i = 0; i < size; i += sizeof(long)) {
            long word = ptrace(PTRACE_PEEKDATA, pid, addr + i, NULL);
            memcpy(data + i, &word, sizeof(long));
        }
        hex_dump(data, size, addr);
        free(data);
    }
    
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
    return 0;
}
