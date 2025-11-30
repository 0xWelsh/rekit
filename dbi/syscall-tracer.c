#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>

const char *syscall_names[] = {
    [0] = "read", [1] = "write", [2] = "open", [3] = "close",
    [4] = "stat", [5] = "fstat", [9] = "mmap", [10] = "mprotect",
    [11] = "munmap", [12] = "brk", [21] = "access", [32] = "dup",
    [33] = "dup2", [39] = "getpid", [41] = "socket", [42] = "connect",
    [56] = "clone", [57] = "fork", [59] = "execve", [60] = "exit",
    [61] = "wait4", [62] = "kill", [257] = "openat"
};

void trace_syscalls(pid_t pid) {
    struct user_regs_struct regs;
    int status, in_syscall = 0;
    
    printf("[*] Tracing syscalls for PID %d\n\n", pid);
    
    while (1) {
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) break;
        
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);
        
        if (!in_syscall) {
            const char *name = (regs.orig_rax < sizeof(syscall_names)/sizeof(char*) && 
                               syscall_names[regs.orig_rax]) ? 
                               syscall_names[regs.orig_rax] : "unknown";
            
            printf("%s(%lld, 0x%llx, 0x%llx, 0x%llx)", 
                   name, regs.orig_rax, regs.rdi, regs.rsi, regs.rdx);
            in_syscall = 1;
        } else {
            printf(" = %lld\n", (long long)regs.rax);
            in_syscall = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], &argv[1]);
        exit(1);
    }
    
    int status;
    waitpid(pid, &status, 0);
    trace_syscalls(pid);
    return 0;
}
