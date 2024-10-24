#pragma once
#include <Windows.h>

#define ACPI_TYPE_VALUE 0x41435049
#define Firm_TYPE_VALUE 0x4649524D
#define Rsmb_TYPE_VALUE 0x52534D42

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)
struct RawSMBIOSData
{
    CHAR    Used20CallingMethod;
    CHAR    SMBIOSMajorVersion;
    CHAR    SMBIOSMinorVersion;
    CHAR    DmiRevision;
    DWORD   Length;
    CHAR    SMBIOSTableData[];
};

typedef struct _CONFIG_DATA
{
    ULONG64 BASEADDRESS;
    CHAR PCISEGMENTGROUP[2];
    CHAR PCISTART;
    CHAR PCIEND;
    CHAR RESERVED2[4];
}CONFIG_DATA;

typedef struct _ACPI_DATA
{
    CHAR Signature[4];
    DWORD   Length;
    CHAR Revision;
    CHAR Checksum;
    CHAR OEMID[6];
    CHAR OEMTABLEID[8];
    CHAR OEMREVISION[4];
    CHAR CREATORID[4];
    CHAR CREATORREVISION[4];
    CHAR RESERVED[8];
    CONFIG_DATA Configs;
}ACPI_DATA;
#pragma pack(pop)