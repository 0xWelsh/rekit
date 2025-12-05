#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

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
            printf("{\"error\": \"Cannot open file: %s\"}\n", strerror(errno));
        } else {
            fprintf(stderr, "Error: Cannot open file '%s': %s\n", filename, strerror(errno));
        }
        return;
    }
    
    if (fseek(f, 0, SEEK_END) != 0) {
        if (json_mode) {
            printf("{\"error\": \"Cannot seek file\"}\n");
        } else {
            fprintf(stderr, "Error: Cannot seek file\n");
        }
        fclose(f);
        return;
    }
    
    long size = ftell(f);
    if (size < 0) {
        if (json_mode) {
            printf("{\"error\": \"Cannot determine file size\"}\n");
        } else {
            fprintf(stderr, "Error: Cannot determine file size\n");
        }
        fclose(f);
        return;
    }
    
    if (size == 0) {
        if (json_mode) {
            printf("{\"tool\": \"strings\", \"file\": \"%s\", \"strings\": []}\n", filename);
        } else {
            fprintf(stderr, "Warning: File is empty\n");
        }
        fclose(f);
        return;
    }
    
    if (size > 100 * 1024 * 1024) {  // 100MB limit
        if (json_mode) {
            printf("{\"error\": \"File too large (max 100MB)\"}\n");
        } else {
            fprintf(stderr, "Error: File too large (max 100MB)\n");
        }
        fclose(f);
        return;
    }
    
    fseek(f, 0, SEEK_SET);
    
    unsigned char *data = malloc(size);
    if (!data) {
        if (json_mode) {
            printf("{\"error\": \"Memory allocation failed\"}\n");
        } else {
            fprintf(stderr, "Error: Memory allocation failed\n");
        }
        fclose(f);
        return;
    }
    
    size_t bytes_read = fread(data, 1, size, f);
    fclose(f);
    
    if (bytes_read != (size_t)size) {
        if (json_mode) {
            printf("{\"error\": \"Failed to read complete file\"}\n");
        } else {
            fprintf(stderr, "Error: Failed to read complete file\n");
        }
        free(data);
        return;
    }
    
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
        fprintf(stderr, "Usage: %s <file> [min_length] [--json]\n", argv[0]);
        fprintf(stderr, "  min_length: minimum string length (default: 4)\n");
        fprintf(stderr, "  --json: output in JSON format\n");
        return 1;
    }
    
    int min_len = 4;
    int json_mode = 0;
    
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0) {
            json_mode = 1;
        } else {
            min_len = atoi(argv[i]);
            if (min_len < 1 || min_len > 1024) {
                if (json_mode) {
                    printf("{\"error\": \"Invalid min_length (must be 1-1024)\"}\n");
                } else {
                    fprintf(stderr, "Error: Invalid min_length (must be 1-1024)\n");
                }
                return 1;
            }
        }
    }
    
    extract_strings(argv[1], min_len, json_mode);
    return 0;
}
