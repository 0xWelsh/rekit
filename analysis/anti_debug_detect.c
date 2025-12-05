#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct {
    int ptrace_check;
    int timing_check;
    int breakpoint_check;
    int parent_check;
    int ld_preload_check;
    int int3_count;
    int suspicious_strings;
} AntiDebugResults;

void check_ptrace_strings(void *base, size_t size, AntiDebugResults *results) {
    const char *patterns[] = {
        "ptrace", "PTRACE", "PT_DENY_ATTACH",
        "debugger", "DEBUGGER", "IsDebuggerPresent",
        NULL
    };
    
    for (int i = 0; patterns[i]; i++) {
        if (memmem(base, size, patterns[i], strlen(patterns[i]))) {
            results->ptrace_check = 1;
            results->suspicious_strings++;
        }
    }
}

void check_timing_strings(void *base, size_t size, AntiDebugResults *results) {
    const char *patterns[] = {
        "rdtsc", "RDTSC", "clock_gettime", "gettimeofday",
        "QueryPerformanceCounter", "GetTickCount",
        NULL
    };
    
    for (int i = 0; patterns[i]; i++) {
        if (memmem(base, size, patterns[i], strlen(patterns[i]))) {
            results->timing_check = 1;
            results->suspicious_strings++;
        }
    }
}

void check_parent_strings(void *base, size_t size, AntiDebugResults *results) {
    const char *patterns[] = {
        "getppid", "PPID", "/proc/self/status", "TracerPid",
        NULL
    };
    
    for (int i = 0; patterns[i]; i++) {
        if (memmem(base, size, patterns[i], strlen(patterns[i]))) {
            results->parent_check = 1;
            results->suspicious_strings++;
        }
    }
}

void check_ld_preload_strings(void *base, size_t size, AntiDebugResults *results) {
    const char *patterns[] = {
        "LD_PRELOAD", "LD_DEBUG", "/proc/self/maps",
        NULL
    };
    
    for (int i = 0; patterns[i]; i++) {
        if (memmem(base, size, patterns[i], strlen(patterns[i]))) {
            results->ld_preload_check = 1;
            results->suspicious_strings++;
        }
    }
}

void check_breakpoints(void *base, size_t size, AntiDebugResults *results) {
    unsigned char *data = (unsigned char *)base;
    
    // Look for INT3 (0xCC) instructions
    for (size_t i = 0; i < size; i++) {
        if (data[i] == 0xCC) {
            results->int3_count++;
        }
    }
    
    if (results->int3_count > 10) {  // More than 10 INT3s is suspicious
        results->breakpoint_check = 1;
    }
}

void analyze_elf_sections(void *base, Elf64_Ehdr *ehdr, AntiDebugResults *results) {
    Elf64_Shdr *shdr = (Elf64_Shdr *)(base + ehdr->e_shoff);
    Elf64_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];
    char *strtab = base + shstrtab->sh_offset;
    
    for (int i = 0; i < ehdr->e_shnum; i++) {
        char *name = strtab + shdr[i].sh_name;
        
        // Check for packed/obfuscated sections
        if (strstr(name, "UPX") || strstr(name, ".packed")) {
            results->suspicious_strings++;
        }
        
        // Check executable sections for anti-debug code
        if (shdr[i].sh_flags & SHF_EXECINSTR) {
            void *section_data = base + shdr[i].sh_offset;
            size_t section_size = shdr[i].sh_size;
            
            // Scan for anti-debug patterns
            check_breakpoints(section_data, section_size, results);
        }
    }
}

void print_results(const char *filename, AntiDebugResults *results, int json_mode) {
    int total_score = results->ptrace_check + results->timing_check + 
                      results->breakpoint_check + results->parent_check + 
                      results->ld_preload_check;
    
    if (json_mode) {
        printf("{\n");
        printf("  \"file\": \"%s\",\n", filename);
        printf("  \"anti_debug_detected\": %s,\n", total_score > 0 ? "true" : "false");
        printf("  \"techniques\": {\n");
        printf("    \"ptrace_detection\": %s,\n", results->ptrace_check ? "true" : "false");
        printf("    \"timing_checks\": %s,\n", results->timing_check ? "true" : "false");
        printf("    \"breakpoint_detection\": %s,\n", results->breakpoint_check ? "true" : "false");
        printf("    \"parent_process_check\": %s,\n", results->parent_check ? "true" : "false");
        printf("    \"ld_preload_check\": %s\n", results->ld_preload_check ? "true" : "false");
        printf("  },\n");
        printf("  \"statistics\": {\n");
        printf("    \"int3_instructions\": %d,\n", results->int3_count);
        printf("    \"suspicious_strings\": %d,\n", results->suspicious_strings);
        printf("    \"risk_score\": %d\n", total_score);
        printf("  }\n");
        printf("}\n");
    } else {
        printf("=== Anti-Debug Detection Report ===\n");
        printf("File: %s\n\n", filename);
        
        printf("Techniques Detected:\n");
        printf("  [%c] Ptrace detection\n", results->ptrace_check ? 'X' : ' ');
        printf("  [%c] Timing checks\n", results->timing_check ? 'X' : ' ');
        printf("  [%c] Breakpoint detection\n", results->breakpoint_check ? 'X' : ' ');
        printf("  [%c] Parent process check\n", results->parent_check ? 'X' : ' ');
        printf("  [%c] LD_PRELOAD check\n", results->ld_preload_check ? 'X' : ' ');
        
        printf("\nStatistics:\n");
        printf("  INT3 instructions: %d\n", results->int3_count);
        printf("  Suspicious strings: %d\n", results->suspicious_strings);
        
        printf("\nRisk Assessment:\n");
        if (total_score == 0) {
            printf("  ✓ No anti-debug techniques detected\n");
        } else if (total_score <= 2) {
            printf("  ⚠ LOW - Basic anti-debug detected\n");
        } else if (total_score <= 4) {
            printf("  ⚠ MEDIUM - Multiple techniques detected\n");
        } else {
            printf("  ⚠ HIGH - Heavily protected binary\n");
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary> [--json]\n", argv[0]);
        fprintf(stderr, "Detects anti-debugging techniques in binaries\n");
        return 1;
    }
    
    int json_mode = (argc > 2 && strcmp(argv[2], "--json") == 0);
    
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        if (json_mode) {
            printf("{\"error\": \"Cannot open file\"}\n");
        } else {
            fprintf(stderr, "Error: Cannot open file\n");
        }
        return 1;
    }
    
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        if (json_mode) {
            printf("{\"error\": \"Cannot stat file\"}\n");
        } else {
            fprintf(stderr, "Error: Cannot stat file\n");
        }
        return 1;
    }
    
    void *base = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (base == MAP_FAILED) {
        close(fd);
        if (json_mode) {
            printf("{\"error\": \"Memory mapping failed\"}\n");
        } else {
            fprintf(stderr, "Error: Memory mapping failed\n");
        }
        return 1;
    }
    
    AntiDebugResults results = {0};
    
    // Check if ELF
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)base;
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0) {
        analyze_elf_sections(base, ehdr, &results);
    }
    
    // Check entire binary for anti-debug strings
    check_ptrace_strings(base, st.st_size, &results);
    check_timing_strings(base, st.st_size, &results);
    check_parent_strings(base, st.st_size, &results);
    check_ld_preload_strings(base, st.st_size, &results);
    
    print_results(argv[1], &results, json_mode);
    
    munmap(base, st.st_size);
    close(fd);
    
    return 0;
}
