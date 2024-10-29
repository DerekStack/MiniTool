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

typedef struct _PCI_DEVICE
{
	UINT16 VendorID;
	UINT16 DeviceID;
	UINT16 Command;
	UINT16 Status;
	UINT8 RevisionID;
	UINT8 ProgIF;
	UINT8 Subclass;
	UINT8 Class;
	UINT8 Cache;
	UINT8 LatencyTimer;
	UINT8 HeaderType;
	UINT8 BIST;
	UINT32 BaseAddress[6];
	UINT32 Cardbus;
	UINT16 SubVendorID;
	UINT16 SubsystemID;
	UINT32 EROMBaseAddress;
	UINT8 Capabilities;
	UINT8 Reserved[3];
	UINT32 Reserved2;
	UINT8 InterruptLine;
	UINT8 Interrupt;
	UINT8 MinGrant;
	UINT8 MaxLatency;

}PCI_DEVICE, * PPCI_DEVICE;


typedef struct _PCI_BRIDGE
{
	UINT16 VendorID;
	UINT16 DeviceID;
	UINT16 Command;
	UINT16 Status;
	UINT8 RevisionID;
	UINT8 ProgIF;
	UINT8 Subclass;
	UINT8 Class;
	UINT8 Cache;
	UINT8 LatencyTimer;
	UINT8 HeaderType;
	UINT8 BIST;
	UINT32 BaseAddress[2];
	UINT8 PrimaryBus;
	UINT8 SecondaryBus;
	UINT8 Subordinate;
	UINT8 SecondaryLT;					//Secondary Latency Timer
	UINT8 IOBase;
	UINT8 IOLimit;
	UINT16 SecondaryStatus;
	UINT16 Memorybase;
	UINT16 MemoryLimit;
	UINT16 PreMemoryBase;				// Prefetchable Memory Base
	UINT16 PreMemoryLimit;				// Prefetchable Memory Limit
	UINT32 PreBaseUpper;				// Prefetchable Base Upper 32 Bits
	UINT32 PreLimitUpper;				// Prefetchable Limit Upper 32 Bits
	UINT16 IOBaseUpper;					// I/O Base Upper 16 Bits
	UINT16 IOLimitUpper;				// I/O Limit Upper 16 Bits
	UINT8 Capability;					// Capability Pointer
	UINT8 Reserved[3];
	UINT32 EROMBaseAddress;				// Expansion ROM base address
	UINT8 InterruptLine;
	UINT8 InterruptPIN;
	UINT16 BridgeControl;

}PCI_BRIDGE, * PPCI_BRIDGE;

typedef struct _PCI_CARDBUS
{
	UINT16 VendorID;
	UINT16 DeviceID;
	UINT16 Command;
	UINT16 Status;
	UINT8 RevisionID;
	UINT8 ProgIF;
	UINT8 Subclass;
	UINT8 Class;
	UINT8 Cache;
	UINT8 LatencyTimer;
	UINT8 HeaderType;
	UINT8 BIST;
	UINT32 ExCaBaseAddress;
	UINT8 Offset;
	UINT8 Reserved;
	UINT16 SecondaryStatus;
	UINT8 PCIBusNum;
	UINT8 CardBusNum;
	UINT8 SubordinateBusNum;
	UINT8 CardBusLatency;
	UINT32 MemoryBaseAddress0;
	UINT32 MemoryLimit0;
	UINT32 MemoryBaseAddress1;
	UINT32 MemoryLimit1;
	UINT32 IOBaseAddress0;
	UINT32 IOLimit0;
	UINT32 IOBaseAddress1;
	UINT32 IOLimit1;
	UINT8 InterruptLine;
	UINT8 InterruptPIN;
	UINT16 BridgeControl;
	UINT16 SubsystemDevID;
	UINT16 SubsystemVendorID;
	UINT32 ModeBaseAddress;//16-bit PC Card legacy mode base address

}PCI_CARDBUS, * PPCI_CARDBUS;


#pragma pack(pop)