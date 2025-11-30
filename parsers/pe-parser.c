#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t e_magic;
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    uint32_t e_lfanew;
} DOS_HEADER;

typedef struct {
    uint16_t Machine;
    uint16_t NumberOfSections;
    uint32_t TimeDateStamp;
    uint32_t PointerToSymbolTable;
    uint32_t NumberOfSymbols;
    uint16_t SizeOfOptionalHeader;
    uint16_t Characteristics;
} FILE_HEADER;

typedef struct {
    uint16_t Magic;
    uint8_t  MajorLinkerVersion;
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;
    uint32_t BaseOfCode;
    uint64_t ImageBase;
    uint32_t SectionAlignment;
    uint32_t FileAlignment;
} OPTIONAL_HEADER_START;

typedef struct {
    char     Name[8];
    uint32_t VirtualSize;
    uint32_t VirtualAddress;
    uint32_t SizeOfRawData;
    uint32_t PointerToRawData;
    uint32_t PointerToRelocations;
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;
} SECTION_HEADER;
#pragma pack(pop)

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <exe_file>\n", argv[0]);
        return 1;
    }
    
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    DOS_HEADER dos;
    fread(&dos, sizeof(dos), 1, f);
    
    if (dos.e_magic != 0x5A4D) {
        printf("ERROR: Not a valid PE file (missing MZ signature)\n");
        fclose(f);
        return 1;
    }
    
    printf("===========================================\n");
    printf("         PE File Structure\n");
    printf("===========================================\n\n");
    
    printf("DOS Header:\n");
    printf("   Magic: MZ (0x%04X)\n", dos.e_magic);
    printf("   PE Offset: 0x%X\n\n", dos.e_lfanew);
    
    fseek(f, dos.e_lfanew, SEEK_SET);
    uint32_t pe_sig;
    fread(&pe_sig, 4, 1, f);
    
    if (pe_sig != 0x4550) {
        printf("ERROR: Invalid PE signature\n");
        fclose(f);
        return 1;
    }
    
    printf("PE Signature: PE (0x%08X)\n\n", pe_sig);
    
    FILE_HEADER file_hdr;
    fread(&file_hdr, sizeof(file_hdr), 1, f);
    
    printf("COFF File Header:\n");
    printf("   Machine: 0x%X (%s)\n", file_hdr.Machine,
           file_hdr.Machine == 0x8664 ? "x64" :
           file_hdr.Machine == 0x14c ? "x86" : "Unknown");
    printf("   Sections: %d\n", file_hdr.NumberOfSections);
    printf("   Timestamp: 0x%X\n", file_hdr.TimeDateStamp);
    printf("   Characteristics: 0x%X %s\n\n", file_hdr.Characteristics,
           file_hdr.Characteristics & 0x2000 ? "(DLL)" : "(EXE)");
    
    OPTIONAL_HEADER_START opt;
    fread(&opt, sizeof(opt), 1, f);
    
    printf("Optional Header:\n");
    printf("   Magic: 0x%X (%s)\n", opt.Magic,
           opt.Magic == 0x20B ? "PE32+" : opt.Magic == 0x10B ? "PE32" : "Unknown");
    printf("   Entry Point: 0x%X\n", opt.AddressOfEntryPoint);
    printf("   Image Base: 0x%lX\n", opt.ImageBase);
    printf("   Code Size: 0x%X\n\n", opt.SizeOfCode);
    
    fseek(f, dos.e_lfanew + 4 + sizeof(FILE_HEADER) + file_hdr.SizeOfOptionalHeader, SEEK_SET);
    
    printf("Sections:\n");
    for (int i = 0; i < file_hdr.NumberOfSections; i++) {
        SECTION_HEADER sec;
        fread(&sec, sizeof(sec), 1, f);
        
        char name[9] = {0};
        memcpy(name, sec.Name, 8);
        
        printf("   [%d] %-8s\n", i, name);
        printf("       Virtual Address: 0x%08X\n", sec.VirtualAddress);
        printf("       Virtual Size: 0x%X\n", sec.VirtualSize);
        printf("       Raw Size: 0x%X\n", sec.SizeOfRawData);
        printf("       Raw Offset: 0x%X\n", sec.PointerToRawData);
        printf("       Flags: 0x%08X ", sec.Characteristics);
        if (sec.Characteristics & 0x20000000) printf("(Execute) ");
        if (sec.Characteristics & 0x40000000) printf("(Read) ");
        if (sec.Characteristics & 0x80000000) printf("(Write)");
        printf("\n\n");
    }
    
    fclose(f);
    return 0;
}
