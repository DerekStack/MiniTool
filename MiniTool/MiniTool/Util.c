#include <ntddk.h>
#include <ntstrsafe.h>

#include "Util.h"


void MemoryCopyValue(
	void* Dst, 
	void* Src, 
	ULONG64 Len
)
{
	//check 8 bytes
	ULONG roundBy8;
	ULONG roundBy4;
	ULONG64 remainLen;
	ULONG64 byteCopied;

	if (Len == 0)
	{
		return;
	}

	remainLen = Len;
	roundBy8 = remainLen / 8;

	ULONG64* DstValueBy8 = (ULONG64*)Dst;
	ULONG64* SrcValueBy8 = (ULONG64*)Src;


	for (int i = 0; i < roundBy8; i++)
	{
		DstValueBy8[roundBy8 - i - 1] = (SrcValueBy8[roundBy8 - i - 1]);
	}

	remainLen = remainLen - roundBy8 * 8;

	byteCopied = roundBy8 * 8;

	if (remainLen <= 0)
	{
		return;
	}


	ULONG* DstValueBy4 = (ULONG*)(DstValueBy8 + roundBy8);
	ULONG* SrcValueBy4 = (ULONG*)(SrcValueBy8 + roundBy8);

	//check 4 bytes
	roundBy4 = remainLen / 4;

	for (int i = 0; i < roundBy4; i++)
	{
		DstValueBy4[roundBy4 - i - 1] = SrcValueBy4[roundBy4 - i - 1];
	}

	remainLen = remainLen - roundBy4 * 4;

	byteCopied = byteCopied + roundBy4 * 4;

	if (remainLen <= 0)
	{
		return;
	}

	CHAR* DstValue = (CHAR*)(DstValueBy4 + roundBy4);
	CHAR* SrcValue = (CHAR*)(SrcValueBy4 + roundBy4);

	for (int i = 0; i < remainLen; i++)
	{
		DstValue[remainLen - i - 1] = SrcValue[remainLen - i - 1];
	}

	byteCopied = byteCopied + remainLen;
}

UINT32 ReadPCICfg(
	UINT8 bus,
	UINT8 dev,
	UINT8 fun,
	UINT8 off,
	UINT8 len // 1, 2, 4 bytes
)
{
	unsigned int result = 0;
	unsigned int pci_addr = (0x80000000 | (bus << 16) | (dev << 11) | (fun << 8) | (off & ~3));
	unsigned short cfg_data_port = (UINT16)(0xCFC + (off & 0x3));
	if (1 == len) result = (ReadPCIByte(pci_addr, cfg_data_port) & 0xFF);
	else if (2 == len) result = (ReadPCIWord(pci_addr, cfg_data_port) & 0xFFFF);
	else if (4 == len) result = ReadPCIDword(pci_addr, cfg_data_port);
	return result;
}

VOID WritePCICfg(
	UINT8 bus,
	UINT8 dev,
	UINT8 fun,
	UINT8 off,
	UINT8 len, // 1, 2, 4 bytes
	UINT32 val
)
{
	UINT32 pci_addr = (0x80000000 | (bus << 16) | (dev << 11) | (fun << 8) | (off & ~3));
	UINT16 cfg_data_port = (UINT16)(0xCFC + (off & 0x3));
	if (1 == len) WritePCIByte(pci_addr, cfg_data_port, (UINT8)(val & 0xFF));
	else if (2 == len) WritePCIWord(pci_addr, cfg_data_port, (UINT16)(val & 0xFFFF));
	else if (4 == len) WritePCIDword(pci_addr, cfg_data_port, val);
}
