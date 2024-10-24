#pragma once


#ifndef UTIL_H
#define UTIL_H

#include <ntddk.h> 


EXTERN_C_START
//PCI

UINT32 ReadPCICfg(
	UINT8 bus,
	UINT8 dev,
	UINT8 fun,
	UINT8 off,
	UINT8 len // 1, 2, 4 bytes
);

VOID WritePCICfg(
	UINT8 bus,
	UINT8 dev,
	UINT8 fun,
	UINT8 off,
	UINT8 len, // 1, 2, 4 bytes
	UINT32 val
);

//Memory
void MemoryCopyValue(
	void* Dst, 
	void* Src, 
	ULONG64 Len);


extern UINT8 ReadPCIByte(
	UINT32 pci_reg,
	UINT16 cfg_data_port);

extern UINT16 ReadPCIWord(
	UINT32 pci_reg,
	UINT16 cfg_data_port);

extern UINT32 ReadPCIDword(
	UINT32 pci_reg,
	UINT16 cfg_data_port
);

extern void WritePCIByte(
	UINT32 pci_reg,
	UINT16 cfg_data_port,
	UINT8  byte_value
);

extern void WritePCIWord(
	UINT32 pci_reg,
	UINT16 cfg_data_port,
	UINT16 word_value
);

extern void WritePCIDword(
	UINT32 pci_reg,
	UINT16 cfg_data_port,
	UINT32 dword_value);

EXTERN_C_END


#endif