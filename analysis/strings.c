#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_json_string(long offset, const char *str, int *first) {
    if (!*first) printf(",\n");
    *first = 0;
    
    printf("    {\"offset\": \"0x%08lx\", \"value\": \"", offset);
    for (int i = 0; str[i]; i++) {
        if (str[i] == '"' || str[i] == '\\') printf("\\");
        else if (str[i] == '\n') { printf("\\n"); continue; }
        else if (str[i] == '\t') { printf("\\t"); continue; }
        printf("%c", str[i]);
    }
    printf("\"}");
}

void extract_strings(const char *filename, int min_len, int json_mode) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        if (json_mode) {
            printf("{\"error\": \"Cannot open file\"}\n");
        } else {
            perror("fopen");
        }
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
    int first = 1;
    
    if (json_mode) {
        printf("{\n  \"tool\": \"strings\",\n  \"file\": \"%s\",\n  \"strings\": [\n", filename);
    }
    
    for (long i = 0; i < size; i++) {
        if (isprint(data[i]) || data[i] == '\t' || data[i] == '\n') {
            if (len == 0) offset = i;
            buf[len++] = data[i];
            if (len >= sizeof(buf) - 1) {
                buf[len] = '\0';
                if (json_mode) {
                    print_json_string(offset, buf, &first);
                } else {
                    printf("0x%08lx: %s\n", offset, buf);
                }
                len = 0;
            }
        } else {
            if (len >= min_len) {
                buf[len] = '\0';
                if (json_mode) {
                    print_json_string(offset, buf, &first);
                } else {
                    printf("0x%08lx: %s\n", offset, buf);
                }
            }
            len = 0;
        }
    }
    
    if (json_mode) {
        printf("\n  ]\n}\n");
    }
    
    free(data);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file> [min_length] [--json]\n", argv[0]);
        return 1;
    }
    
    int min_len = 4;
    int json_mode = 0;
    
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0) {
            json_mode = 1;
        } else {
            min_len = atoi(argv[i]);
        }
    }
    
    extract_strings(argv[1], min_len, json_mode);
    return 0;
}
