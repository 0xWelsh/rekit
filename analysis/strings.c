#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void extract_strings(const char *filename, int min_len) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned char *data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);
    
    char buf[1024];
    int len = 0;
    long offset = 0;
    
    for (long i = 0; i < size; i++) {
        if (isprint(data[i]) || data[i] == '\t' || data[i] == '\n') {
            if (len == 0) offset = i;
            buf[len++] = data[i];
            if (len >= sizeof(buf) - 1) {
                buf[len] = '\0';
                printf("0x%08lx: %s\n", offset, buf);
                len = 0;
            }
        } else {
            if (len >= min_len) {
                buf[len] = '\0';
                printf("0x%08lx: %s\n", offset, buf);
            }
            len = 0;
        }
    }
    
    free(data);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file> [min_length]\n", argv[0]);
        return 1;
    }
    
    int min_len = argc > 2 ? atoi(argv[2]) : 4;
    extract_strings(argv[1], min_len);
    return 0;
}
