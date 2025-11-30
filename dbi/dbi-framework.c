#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <errno.h>
#include <capstone/capstone.h>

#define MAX_BREAKPOINTS 256

typedef struct {
    unsigned long addr;
    unsigned long orig_data;
    int active;
} Breakpoint;

typedef struct {
    pid_t pid;
    Breakpoint breakpoints[MAX_BREAKPOINTS];
    int bp_count;
    csh cs_handle;
} DBI_Context;

void dbi_init(DBI_Context *ctx, pid_t pid) {
    ctx->pid = pid;
    ctx->bp_count = 0;
    cs_open(CS_ARCH_X86, CS_MODE_64, &ctx->cs_handle);
    cs_option(ctx->cs_handle, CS_OPT_DETAIL, CS_OPT_ON);
}

int dbi_set_breakpoint(DBI_Context *ctx, unsigned long addr) {
    if (ctx->bp_count >= MAX_BREAKPOINTS) return -1;
    
    long data = ptrace(PTRACE_PEEKTEXT, ctx->pid, addr, NULL);
    if (data == -1) return -1;
    
    ctx->breakpoints[ctx->bp_count].addr = addr;
    ctx->breakpoints[ctx->bp_count].orig_data = data;
    ctx->breakpoints[ctx->bp_count].active = 1;
    
    long trap = (data & ~0xFF) | 0xCC;
    ptrace(PTRACE_POKETEXT, ctx->pid, addr, trap);
    
    ctx->bp_count++;
    return ctx->bp_count - 1;
}

void dbi_dump_registers(DBI_Context *ctx) {
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, ctx->pid, NULL, &regs);
    
    printf("  RAX: 0x%016llx  RBX: 0x%016llx\n", regs.rax, regs.rbx);
    printf("  RCX: 0x%016llx  RDX: 0x%016llx\n", regs.rcx, regs.rdx);
    printf("  RSI: 0x%016llx  RDI: 0x%016llx\n", regs.rsi, regs.rdi);
    printf("  RBP: 0x%016llx  RSP: 0x%016llx\n", regs.rbp, regs.rsp);
    printf("  RIP: 0x%016llx\n", regs.rip);
}

void dbi_dump_stack(DBI_Context *ctx, int count) {
    struct user_regs_struct regs;
    ptrace(PTRACE_GETREGS, ctx->pid, NULL, &regs);
    
    printf("  Stack (RSP=0x%llx):\n", regs.rsp);
    for (int i = 0; i < count; i++) {
        long data = ptrace(PTRACE_PEEKDATA, ctx->pid, regs.rsp + (i * 8), NULL);
        printf("    [RSP+0x%02x] 0x%016lx\n", i * 8, data);
    }
}

void dbi_disassemble_at(DBI_Context *ctx, unsigned long addr, int count) {
    unsigned char code[64];
    
    for (int i = 0; i < 64; i += 8) {
        long data = ptrace(PTRACE_PEEKTEXT, ctx->pid, addr + i, NULL);
        memcpy(code + i, &data, 8);
    }
    
    cs_insn *insn;
    size_t n = cs_disasm(ctx->cs_handle, code, 64, addr, count, &insn);
    
    printf("  Disassembly at 0x%lx:\n", addr);
    for (size_t i = 0; i < n; i++) {
        printf("    0x%lx: %-12s %s\n", insn[i].address, insn[i].mnemonic, insn[i].op_str);
    }
    cs_free(insn, n);
}

void dbi_trace_syscalls(DBI_Context *ctx) {
    struct user_regs_struct regs;
    int status;
    int in_syscall = 0;
    
    printf("\n[*] Tracing syscalls (Ctrl+C to stop)...\n\n");
    
    while (1) {
        ptrace(PTRACE_SYSCALL, ctx->pid, NULL, NULL);
        waitpid(ctx->pid, &status, 0);
        
        if (WIFEXITED(status)) break;
        
        ptrace(PTRACE_GETREGS, ctx->pid, NULL, &regs);
        
        if (in_syscall == 0) {
            printf("syscall(%lld) args: rdi=0x%llx rsi=0x%llx rdx=0x%llx\n",
                   regs.orig_rax, regs.rdi, regs.rsi, regs.rdx);
            in_syscall = 1;
        } else {
            printf("  -> return: 0x%llx\n\n", regs.rax);
            in_syscall = 0;
        }
    }
}

void dbi_run(DBI_Context *ctx) {
    int status;
    
    printf("\n[*] Starting traced process...\n");
    ptrace(PTRACE_CONT, ctx->pid, NULL, NULL);
    
    while (1) {
        waitpid(ctx->pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("\n[*] Process exited with status %d\n", WEXITSTATUS(status));
            break;
        }
        
        if (WIFSTOPPED(status)) {
            struct user_regs_struct regs;
            ptrace(PTRACE_GETREGS, ctx->pid, NULL, &regs);
            
            printf("\n[*] Breakpoint hit at 0x%llx\n", regs.rip - 1);
            
            for (int i = 0; i < ctx->bp_count; i++) {
                if (ctx->breakpoints[i].active && 
                    ctx->breakpoints[i].addr == regs.rip - 1) {
                    
                    regs.rip--;
                    ptrace(PTRACE_SETREGS, ctx->pid, NULL, &regs);
                    
                    ptrace(PTRACE_POKETEXT, ctx->pid, ctx->breakpoints[i].addr, 
                           ctx->breakpoints[i].orig_data);
                    
                    dbi_dump_registers(ctx);
                    dbi_dump_stack(ctx, 8);
                    dbi_disassemble_at(ctx, regs.rip, 5);
                    
                    ptrace(PTRACE_SINGLESTEP, ctx->pid, NULL, NULL);
                    waitpid(ctx->pid, &status, 0);
                    
                    long trap = (ctx->breakpoints[i].orig_data & ~0xFF) | 0xCC;
                    ptrace(PTRACE_POKETEXT, ctx->pid, ctx->breakpoints[i].addr, trap);
                    break;
                }
            }
            
            ptrace(PTRACE_CONT, ctx->pid, NULL, NULL);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <program> <breakpoint_addr>\n", argv[0]);
        printf("Example: %s /bin/ls 0x401000\n", argv[0]);
        return 1;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl(argv[1], argv[1], NULL);
        perror("execl");
        exit(1);
    }
    
    int status;
    waitpid(pid, &status, 0);
    
    DBI_Context ctx;
    dbi_init(&ctx, pid);
    
    unsigned long bp_addr = strtoul(argv[2], NULL, 16);
    printf("[*] Setting breakpoint at 0x%lx\n", bp_addr);
    dbi_set_breakpoint(&ctx, bp_addr);
    
    dbi_run(&ctx);
    
    cs_close(&ctx.cs_handle);
    return 0;
}
