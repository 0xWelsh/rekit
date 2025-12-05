#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

void print_elf_header(Elf64_Ehdr *ehdr) {
    printf("\n=== ELF Header ===\n");
    printf("Magic:           %02x %02x %02x %02x\n", 
           ehdr->e_ident[0], ehdr->e_ident[1], ehdr->e_ident[2], ehdr->e_ident[3]);
    
    printf("Class:           ");
    if (ehdr->e_ident[EI_CLASS] == ELFCLASS32) printf("ELF32\n");
    else if (ehdr->e_ident[EI_CLASS] == ELFCLASS64) printf("ELF64\n");
    else printf("Unknown\n");
    
    printf("Data:            ");
    if (ehdr->e_ident[EI_DATA] == ELFDATA2LSB) printf("Little Endian\n");
    else if (ehdr->e_ident[EI_DATA] == ELFDATA2MSB) printf("Big Endian\n");
    else printf("Unknown\n");
    
    printf("Type:            ");
    switch (ehdr->e_type) {
        case ET_EXEC: printf("Executable\n"); break;
        case ET_DYN:  printf("Shared Object\n"); break;
        case ET_REL:  printf("Relocatable\n"); break;
        default:      printf("Unknown (0x%x)\n", ehdr->e_type);
    }
    
    printf("Machine:         ");
    switch (ehdr->e_machine) {
        case EM_386:    printf("x86\n"); break;
        case EM_X86_64: printf("x86-64\n"); break;
        case EM_ARM:    printf("ARM\n"); break;
        case EM_AARCH64: printf("ARM64\n"); break;
        default:        printf("Unknown (0x%x)\n", ehdr->e_machine);
    }
    
    printf("Entry Point:     0x%lx\n", ehdr->e_entry);
    printf("Program Headers: %d (offset: 0x%lx)\n", ehdr->e_phnum, ehdr->e_phoff);
    printf("Section Headers: %d (offset: 0x%lx)\n", ehdr->e_shnum, ehdr->e_shoff);
}

void print_section_headers(void *base, Elf64_Ehdr *ehdr) {
    Elf64_Shdr *shdr = (Elf64_Shdr *)(base + ehdr->e_shoff);
    Elf64_Shdr *shstrtab = &shdr[ehdr->e_shstrndx];
    char *strtab = base + shstrtab->sh_offset;
    
    printf("\n=== Section Headers ===\n");
    printf("%-20s %-12s %-12s %-10s %s\n", "Name", "Type", "Address", "Size", "Flags");
    printf("─────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < ehdr->e_shnum; i++) {
        char *name = strtab + shdr[i].sh_name;
        
        char type[12];
        switch (shdr[i].sh_type) {
            case SHT_NULL:     strcpy(type, "NULL"); break;
            case SHT_PROGBITS: strcpy(type, "PROGBITS"); break;
            case SHT_SYMTAB:   strcpy(type, "SYMTAB"); break;
            case SHT_STRTAB:   strcpy(type, "STRTAB"); break;
            case SHT_RELA:     strcpy(type, "RELA"); break;
            case SHT_HASH:     strcpy(type, "HASH"); break;
            case SHT_DYNAMIC:  strcpy(type, "DYNAMIC"); break;
            case SHT_NOBITS:   strcpy(type, "NOBITS"); break;
            case SHT_DYNSYM:   strcpy(type, "DYNSYM"); break;
            default:           sprintf(type, "0x%x", shdr[i].sh_type);
        }
        
        char flags[8] = "";
        if (shdr[i].sh_flags & SHF_WRITE) strcat(flags, "W");
        if (shdr[i].sh_flags & SHF_ALLOC) strcat(flags, "A");
        if (shdr[i].sh_flags & SHF_EXECINSTR) strcat(flags, "X");
        if (strlen(flags) == 0) strcpy(flags, "-");
        
        printf("%-20s %-12s 0x%010lx %-10lu %s\n", 
               name, type, shdr[i].sh_addr, shdr[i].sh_size, flags);
    }
}

void print_program_headers(void *base, Elf64_Ehdr *ehdr) {
    Elf64_Phdr *phdr = (Elf64_Phdr *)(base + ehdr->e_phoff);
    
    printf("\n=== Program Headers ===\n");
    printf("%-12s %-12s %-12s %-10s %s\n", "Type", "Offset", "VirtAddr", "FileSize", "Flags");
    printf("─────────────────────────────────────────────────────────────────────\n");
    
    for (int i = 0; i < ehdr->e_phnum; i++) {
        char type[12];
        switch (phdr[i].p_type) {
            case PT_NULL:    strcpy(type, "NULL"); break;
            case PT_LOAD:    strcpy(type, "LOAD"); break;
            case PT_DYNAMIC: strcpy(type, "DYNAMIC"); break;
            case PT_INTERP:  strcpy(type, "INTERP"); break;
            case PT_NOTE:    strcpy(type, "NOTE"); break;
            case PT_PHDR:    strcpy(type, "PHDR"); break;
            case PT_GNU_STACK: strcpy(type, "GNU_STACK"); break;
            default:         sprintf(type, "0x%x", phdr[i].p_type);
        }
        
        char flags[4] = "";
        if (phdr[i].p_flags & PF_R) strcat(flags, "R");
        if (phdr[i].p_flags & PF_W) strcat(flags, "W");
        if (phdr[i].p_flags & PF_X) strcat(flags, "X");
        
        printf("%-12s 0x%010lx 0x%010lx %-10lu %s\n",
               type, phdr[i].p_offset, phdr[i].p_vaddr, phdr[i].p_filesz, flags);
    }
}

void print_symbols(void *base, Elf64_Ehdr *ehdr) {
    Elf64_Shdr *shdr = (Elf64_Shdr *)(base + ehdr->e_shoff);
    
    // Find symbol table and string table
    Elf64_Shdr *symtab = NULL;
    char *strtab = NULL;
    
    for (int i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB || shdr[i].sh_type == SHT_DYNSYM) {
            symtab = &shdr[i];
            strtab = base + shdr[symtab->sh_link].sh_offset;
            break;
        }
    }
    
    if (!symtab) {
        printf("\n=== Symbols ===\n");
        printf("No symbol table found\n");
        return;
    }
    
    Elf64_Sym *sym = (Elf64_Sym *)(base + symtab->sh_offset);
    int sym_count = symtab->sh_size / sizeof(Elf64_Sym);
    
    printf("\n=== Symbols (showing first 50) ===\n");
    printf("%-40s %-12s %-8s\n", "Name", "Address", "Type");
    printf("─────────────────────────────────────────────────────────────────────\n");
    
    int shown = 0;
    for (int i = 0; i < sym_count && shown < 50; i++) {
        if (sym[i].st_name == 0 || sym[i].st_value == 0) continue;
        
        char *name = strtab + sym[i].st_name;
        
        char type[8];
        switch (ELF64_ST_TYPE(sym[i].st_info)) {
            case STT_NOTYPE:  strcpy(type, "NOTYPE"); break;
            case STT_OBJECT:  strcpy(type, "OBJECT"); break;
            case STT_FUNC:    strcpy(type, "FUNC"); break;
            case STT_SECTION: strcpy(type, "SECTION"); break;
            case STT_FILE:    strcpy(type, "FILE"); break;
            default:          sprintf(type, "%d", ELF64_ST_TYPE(sym[i].st_info));
        }
        
        printf("%-40s 0x%010lx %-8s\n", name, sym[i].st_value, type);
        shown++;
    }
    
    if (shown == 50 && sym_count > 50) {
        printf("... (%d more symbols)\n", sym_count - 50);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <elf_file>\n", argv[0]);
        return 1;
    }
    
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }
    
    void *base = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (base == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)base;
    
    // Verify ELF magic
    if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Error: Not a valid ELF file\n");
        munmap(base, st.st_size);
        close(fd);
        return 1;
    }
    
    printf("ELF File: %s\n", argv[1]);
    
    print_elf_header(ehdr);
    print_program_headers(base, ehdr);
    print_section_headers(base, ehdr);
    print_symbols(base, ehdr);
    
    munmap(base, st.st_size);
    close(fd);
    
    return 0;
}
