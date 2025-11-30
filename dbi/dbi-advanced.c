#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <capstone/capstone.h>
#include <elf.h>
#include <fcntl.h>

typedef struct {
    char *name;
    unsigned long addr;
    unsigned long orig_data;
} Hook;

typedef struct {
    pid_t pid;
    Hook hooks[256];
    int hook_count;
    csh cs_handle;
} DBI;

void dbi_init(DBI *dbi, pid_t pid) {
    dbi->pid = pid;
    dbi->hook_count = 0;
    cs_open(CS_ARCH_X86, CS_MODE_64, &dbi->cs_handle);
}

unsigned long find_function(pid_t pid, const char *func_name) {
    char maps_path[64], line[512];
    sprintf(maps_path, "/proc/%d/maps", pid);
    
    FILE *maps = fopen(maps_path, "r");
    if (!maps) return 0;
    
    unsigned long base = 0;
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, "r-xp") && strstr(line, "/")) {
            sscanf(line, "%lx", &base);
            break;
        }
    }
    fclose(maps);
    
    if (!base) return 0;
    
    char exe_path[256];
    sprintf(exe_path, "/proc/%d/exe", pid);
    
    int fd = open(exe_path, O_RDONLY);
    if (fd < 0) return 0;
    
    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    
    lseek(fd, ehdr.e_shoff, SEEK_SET);
    Elf64_Shdr shdr[ehdr.e_shnum];
    read(fd, shdr, sizeof(Elf64_Shdr) * ehdr.e_shnum);
    
    char *shstrtab = malloc(shdr[ehdr.e_shstrndx].sh_size);
    lseek(fd, shdr[ehdr.e_shstrndx].sh_offset, SEEK_SET);
    read(fd, shstrtab, shdr[ehdr.e_shstrndx].sh_size);
    
    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB || shdr[i].sh_type == SHT_DYNSYM) {
            int sym_count = shdr[i].sh_size / sizeof(Elf64_Sym);
            Elf64_Sym *syms = malloc(shdr[i].sh_size);
            lseek(fd, shdr[i].sh_offset, SEEK_SET);
            read(fd, syms, shdr[i].sh_size);
            
            char *strtab = malloc(shdr[shdr[i].sh_link].sh_size);
            lseek(fd, shdr[shdr[i].sh_link].sh_offset, SEEK_SET);
            read(fd, strtab, shdr[shdr[i].sh_link].sh_size);
            
            for (int j = 0; j < sym_count; j++) {
                if (syms[j].st_name && strcmp(&strtab[syms[j].st_name], func_name) == 0) {
                    unsigned long addr = base + syms[j].st_value;
                    free(syms);
                    free(strtab);
                    free(shstrtab);
                    close(fd);
                    return addr;
                }
            }
            free(syms);
            free(strtab);
        }
    }
    
    free(shstrtab);
    close(fd);
    return 0;
}

void dbi_hook_function(DBI *dbi, const char *func_name) {
    unsigned long addr = find_function(dbi->pid, func_name);
    if (!addr) {
        printf("[-] Function '%s' not found\n", func_name);
        return;
    }
    
    long data = ptrace(PTRACE_PEEKTEXT, dbi->pid, addr, NULL);
    
    dbi->hooks[dbi->hook_count].name = strdup(func_name);
    dbi->hooks[dbi->hook_count].addr = addr;
    dbi->hooks[dbi->hook_count].orig_data = data;
    
    long trap = (data & ~0xFF) | 0xCC;
    ptrace(PTRACE_POKETEXT, dbi->pid, addr, trap);
    
    printf("[+] Hooked %s at 0x%lx\n", func_name, addr);
    dbi->hook_count++;
}

void dbi_trace(DBI *dbi) {
    int status;
    struct user_regs_struct regs;
    
    printf("\n[*] Starting trace...\n\n");
    ptrace(PTRACE_CONT, dbi->pid, NULL, NULL);
    
    while (1) {
        waitpid(dbi->pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("\n[*] Process exited\n");
            break;
        }
        
        if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
            ptrace(PTRACE_GETREGS, dbi->pid, NULL, &regs);
            
            for (int i = 0; i < dbi->hook_count; i++) {
                if (dbi->hooks[i].addr == regs.rip - 1) {
                    printf("[HOOK] %s()\n", dbi->hooks[i].name);
                    printf("  RDI: 0x%llx  RSI: 0x%llx  RDX: 0x%llx\n", 
                           regs.rdi, regs.rsi, regs.rdx);
                    
                    regs.rip--;
                    ptrace(PTRACE_SETREGS, dbi->pid, NULL, &regs);
                    ptrace(PTRACE_POKETEXT, dbi->pid, dbi->hooks[i].addr, dbi->hooks[i].orig_data);
                    ptrace(PTRACE_SINGLESTEP, dbi->pid, NULL, NULL);
                    waitpid(dbi->pid, &status, 0);
                    
                    long trap = (dbi->hooks[i].orig_data & ~0xFF) | 0xCC;
                    ptrace(PTRACE_POKETEXT, dbi->pid, dbi->hooks[i].addr, trap);
                    break;
                }
            }
            ptrace(PTRACE_CONT, dbi->pid, NULL, NULL);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program> [functions...]\n", argv[0]);
        printf("Example: %s /bin/cat open read write close\n", argv[0]);
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl(argv[1], argv[1], NULL);
        exit(1);
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    DBI dbi;
    dbi_init(&dbi, pid);
    
    for (int i = 2; i < argc; i++) {
        dbi_hook_function(&dbi, argv[i]);
    }
    
    dbi_trace(&dbi);
    cs_close(&dbi.cs_handle);
    return 0;
}
