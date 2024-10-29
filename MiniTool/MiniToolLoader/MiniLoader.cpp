
#include <stdio.h>
#include "MiniDrvCtrl.h"  
#include "..\MiniTool\ioctl.h"
#include "Log.h"
#include "Firmwaretable.h"


MiniDrvCtrl miniDriverCtrl;

#define DRIVER_PATH		"C:\\MiniTool.sys"
#define SERVICE_NAME	"MiniTool"
#define NT_SYMBOLICLINK "\\\\.\\MiniTool"
#define DISPLAY_NAME	SERVICE_NAME

#define DRIVER_REQUEST(a,b,c,d,e,f) miniDriverCtrl.IoControl(NT_SYMBOLICLINK, a,b,c,d,e,f);

using namespace std;

EXTERN_C_START

VOID
ReadPhysicalMemory(
	ULONG64 PhysicalAddress,
	ULONG64 ByteCount,
	UINT32 Alignment
)
{
	ULONG RetBytes = 0;
	PUCHAR Buffer;
	ULONG64 InputParam[2];

	InputParam[0] = PhysicalAddress;
	InputParam[1] = ByteCount;

	Buffer = (PUCHAR)malloc(ByteCount);
	if (Buffer == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return;
	}

	RtlZeroMemory(Buffer, ByteCount);

	DRIVER_REQUEST(IOCTL_RD_PHYSMEM, (void*)InputParam, sizeof(ULONG64) * 2, Buffer, ByteCount, &RetBytes);

	DbgPrint("Memory return from kernel from physical Address= 0x%I64x Length= %I64d \r\n", PhysicalAddress, ByteCount);

	for (int i = 0; i < ByteCount; i += max(1, Alignment))
	{
		if (i != 0 && (i % 16 == 0))
		{
			DbgPrint("\r\n");
		}

		switch (Alignment)
		{
		case 2:
			DbgPrint("%.4X ", ((PUSHORT)Buffer)[i / 2]);
			break;
		case 4:
			DbgPrint("%.8X ", ((PULONG)Buffer)[i / 4]);
			break;
		case 8:
			DbgPrint("%.16llX ", ((PULONG64)Buffer)[i / 8]);
			break;
		default:
			DbgPrint("%.2X ", Buffer[i]);
			break;
		}
	}

	if (Buffer)
	{
		free(Buffer);
	}
}


VOID WritePhysicalMemory(
	ULONG64 PhysicalAddress,
	ULONG64 Data,
	ULONG64 ByteCount,
	UINT32 Alignment)
{
	ULONG RetBytes = 0;
	PUCHAR Buffer;
	ULONG64 InputParam[3];

	InputParam[0] = PhysicalAddress;
	InputParam[1] = Data;
	InputParam[2] = ByteCount;


	Buffer = (PUCHAR)malloc(ByteCount);

	if (Buffer == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return;
	}

	RtlZeroMemory(Buffer, ByteCount);

	DRIVER_REQUEST(IOCTL_WR_PHYSMEM, (void*)InputParam, sizeof(ULONG64) * 3, Buffer, ByteCount, &RetBytes);

	DbgPrint("Memory return from kernel from physical Address= 0x%I64x, Old Data= 0x%I64x, Length= %I64d \r\n", PhysicalAddress, Buffer, ByteCount);

	if (Buffer)
	{
		free(Buffer);
	}
}

VOID
ReadBarPhysicalMemory(
	ULONG64 PhysicalAddress,
	ULONG64 Offset,
	ULONG64 ByteCount,
	UINT32 Alignment
)
{
	ULONG RetBytes = 0;
	PUCHAR Buffer;
	ULONG64 InputParam[3];

	InputParam[0] = PhysicalAddress;
	InputParam[1] = Offset;
	InputParam[2] = ByteCount;

	Buffer = (PUCHAR)malloc(ByteCount);
	if (Buffer == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return;
	}

	RtlZeroMemory(Buffer, ByteCount);

	DRIVER_REQUEST(IOCTL_RD_PHYSMEM, (void*)InputParam, sizeof(ULONG64) * 3, Buffer, ByteCount, &RetBytes);

	DbgPrint("Memory return from kernel from bar physical Address= 0x%I64x Length= %I64d \r\n", PhysicalAddress, ByteCount);

	for (int i = 0; i < ByteCount; i += max(1, Alignment))
	{
		if (i != 0 && (i % 16 == 0))
		{
			DbgPrint("\r\n");
		}

		switch (Alignment)
		{
		case 2:
			DbgPrint("%.4X ", ((PUSHORT)Buffer)[i / 2]);
			break;
		case 4:
			DbgPrint("%.8X ", ((PULONG)Buffer)[i / 4]);
			break;
		case 8:
			DbgPrint("%.16llX ", ((PULONG64)Buffer)[i / 8]);
			break;
		default:
			DbgPrint("%.2X ", Buffer[i]);
			break;
		}
	}

	if (Buffer)
	{
		free(Buffer);
	}
}

int
ReadBarInfo(
	UCHAR Bus,
	UCHAR Dev,
	UCHAR Func
)
{
	PCI_BAR_INFO BarInfo[6];
	ULONG RetBytes = 0;
	ULONG Buffer = 0;

	Buffer |= (Bus << 24);
	Buffer |= (Dev << 16);
	Buffer |= (Func << 8);
	Buffer |= 0;

	RtlZeroMemory(&BarInfo, sizeof(BarInfo));

	DRIVER_REQUEST(IOCTL_RD_BAR_INFO, (void*)&Buffer, 0x4, BarInfo, sizeof(BarInfo), &RetBytes);

	DbgPrint("-----------------------------\r\n");
	DbgPrint("| Bar Address   | Bar Size   |\r\n");
	DbgPrint("-----------------------------\r\n");

	for (int i = 0; i < 6; i++)
	{
		DbgPrint("|%-15x|%-11x|\r\n", BarInfo[i].BarAddress, BarInfo[i].BarSize);
	}

	DbgPrint("-----------------------------\r\n");

	return 0;
}

int ReadPciCfg(
	UCHAR Bus,
	UCHAR Dev,
	UCHAR Func,
	UINT64 ByteCount,
	UCHAR Alignment
)
{
	ULONG RetBytes = 0;
	ULONG Buffer = 0;
	PUCHAR Value;

	Buffer |= (Bus << 24);
	Buffer |= (Dev << 16);
	Buffer |= (Func << 8);
	Buffer |= 0;

	if (ByteCount == 0)
	{
		ByteCount = 256;
	}

	Value = (PUCHAR)malloc(ByteCount);
	if (Value == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return 0;
	}

	RtlZeroMemory(Value, ByteCount);


	DRIVER_REQUEST(IOCTL_RD_PCICFG, (void*)&Buffer, 0x4, Value, ByteCount, &RetBytes);

	DbgPrint("\r\n");
	for (int i = 0; i < ByteCount; i += max(1, Alignment))
	{
		if (i != 0 && (i % 16 == 0))
		{
			DbgPrint("\r\n");
		}

		switch (Alignment)
		{
		case 2:
			DbgPrint("%.4X ", ((PUSHORT)Value)[i / 2]);
			break;
		case 4:
			DbgPrint("%.8X ", ((PULONG)Value)[i / 4]);
			break;
		case 8:
			DbgPrint("%.16llX ", ((PULONG64)Value)[i / 8]);
			break;
		default:
			DbgPrint("%.2X ", Value[i]);
			break;
		}
	}

	if (Value)
	{
		free(Value);
	}

	return RetBytes;
}


VOID GetMCFGTable(ULONG64* baseAddress)
{
	DWORD bytesWritten = 0;
	DWORD mcfgBuffSize = GetSystemFirmwareTable('ACPI', 'GFCM', NULL, 0);
	char* buff = (char*)malloc(mcfgBuffSize * sizeof(char));
	if (!buff)
	{
		printf("Error for malloc");
		return;
	}

	bytesWritten = GetSystemFirmwareTable('ACPI', 'GFCM', buff, mcfgBuffSize);

	if (bytesWritten != mcfgBuffSize) 
	{

		printf("Error for malloc");
		return;
	}

	ACPI_DATA* tableBuff = (ACPI_DATA*)buff;

	ULONG64 cfgBaseAddress = tableBuff->Configs.BASEADDRESS;

	if (buff)
	{
		free(buff);
	}

	*baseAddress = cfgBaseAddress;
}

VOID ReadMMCFG(
	UCHAR Bus,
	UCHAR Dev,
	UCHAR Func,
	UINT64 Offset,
	UINT64 ByteCount,
	UCHAR Alignment
)
{
	ULONG RetBytes = 0;
	PUCHAR Buffer;
	PUCHAR Value;
	ULONG64 InputParam[3];
	ULONG64 mcfgTableBase = 0;

	GetMCFGTable(&mcfgTableBase);

	if (mcfgTableBase == 0)
	{
		DbgPrint("cannot GetMCFGTable address \r\n");
		return;
	}

	Buffer = (PUCHAR)malloc(ByteCount);
	if (Buffer == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return;
	}

	ULONG64 BaseAddress = mcfgTableBase;
	ULONG64 pciConfigAddress = BaseAddress + (Bus << 20 | Dev << 15 | Func << 12);


	InputParam[0] = pciConfigAddress;
	InputParam[1] = Offset;
	InputParam[2] = ByteCount;



	Value = (PUCHAR)malloc(ByteCount);
	if (Value == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return;
	}

	RtlZeroMemory(Value, ByteCount);

	DRIVER_REQUEST(IOCTL_RD_PHYSMEM, (void*)InputParam, sizeof(ULONG64) * 3, Buffer, ByteCount, &RetBytes);

	DbgPrint("\r\n");
	for (int i = 0; i < ByteCount; i += max(1, Alignment))
	{
		if (i != 0 && (i % 16 == 0))
		{
			DbgPrint("\r\n");
		}

		switch (Alignment)
		{
		case 2:
			DbgPrint("%.4X ", ((PUSHORT)Value)[i / 2]);
			break;
		case 4:
			DbgPrint("%.8X ", ((PULONG)Value)[i / 4]);
			break;
		case 8:
			DbgPrint("%.16llX ", ((PULONG64)Value)[i / 8]);
			break;
		default:
			DbgPrint("%.2X ", Value[i]);
			break;
		}
	}

	if (Value)
	{
		free(Value);
	}

}


VOID EnumPCIIOAll()
{
	PCI_INFO PCIInfo;

	ULONG RetBytes = 0;
	ULONG Buffer = 0;

	DbgPrint("--------------------------------------\r\n");
	DbgPrint("| PCI ID   | Vendor ID   | Device ID | \r\n");
	DbgPrint("--------------------------------------\r\n");

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				Buffer = 0;
				Buffer |= (i << 24);
				Buffer |= (j << 16);
				Buffer |= (k << 8);
				Buffer |= 0;

				RtlZeroMemory(&PCIInfo, sizeof(PCI_INFO));
				DRIVER_REQUEST(IOCTL_ENUM_PCI, (void*)&Buffer, 0x4, &PCIInfo, sizeof(PCIInfo), &RetBytes);

				UINT32 PCIID = PCIInfo.PCIID;
				if (PCIID != 0)
				{
					UINT32 Bus = (PCIID >> 24) & 0xFF;
					UINT32 Dev = (PCIID >> 16) & 0xFF;
					UINT32 Fun = (PCIID >> 8) & 0xFF;
					DbgPrint("|B:%x D:%x F:%x|%-11x|%-11x|\r\n", Bus, Dev, Fun, PCIInfo.u.VendorID, PCIInfo.u.DeviceID);
				}
				
			}
		}
	}
}

VOID EnumPCIEAll()
{
	ULONG RetBytes = 0;
	ULONG Buffer = 0;

	PCI_INFO PCIInfo;

	DbgPrint("--------------------------------------\r\n");
	DbgPrint("| PCI ID   | Vendor ID   | Device ID | \r\n");
	DbgPrint("--------------------------------------\r\n");

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				Buffer = 0;
				Buffer |= (i << 24);
				Buffer |= (j << 16);
				Buffer |= (k << 8);
				Buffer |= 0;

				RtlZeroMemory(&PCIInfo, sizeof(PCI_INFO));
				DRIVER_REQUEST(IOCTL_ENUM_PCIE, (void*)&Buffer, 0x4, &PCIInfo, sizeof(PCIInfo), &RetBytes);

				UINT32 PCIID = PCIInfo.PCIID;

				if (PCIID != 0)
				{
					UINT32 Bus = (PCIID >> 24) & 0xFF;
					UINT32 Dev = (PCIID >> 16) & 0xFF;
					UINT32 Fun = (PCIID >> 8) & 0xFF;
					DbgPrint("|B:%x D:%x F:%x|%-11x|%-11x|\r\n", Bus, Dev, Fun, PCIInfo.u.VendorID, PCIInfo.u.DeviceID);
				}
			}
		}
	}
}

void ReadPciDevice(UCHAR Bus,
	UCHAR Dev,
	UCHAR Func,
	PCI_DEVICE* pciDevice)
{
	ULONG RetBytes = 0;
	ULONG Buffer = 0;
	ULONG ByteCount = 256;

	Buffer |= (Bus << 24);
	Buffer |= (Dev << 16);
	Buffer |= (Func << 8);
	Buffer |= 0;

	DRIVER_REQUEST(IOCTL_RD_PCICFG, (void*)&Buffer, 0x4, pciDevice, ByteCount, &RetBytes);

}

int ReadPciDeviceType(
	UCHAR Bus,
	UCHAR Dev,
	UCHAR Func)
{

	PCI_DEVICE* Value;
	ULONG ByteCount = 256;

	Value = (PCI_DEVICE*)malloc(ByteCount);
	if (Value == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return 0;
	}

	RtlZeroMemory(Value, ByteCount);

	ReadPciDevice(Bus,Dev,Func,(PCI_DEVICE*)Value);

	DbgPrint("\r\n");
	//check Header type
	PCI_DEVICE* pciDevice = (PCI_DEVICE*)Value;
	DbgPrint("Class: %x , SubClass:%x", pciDevice->Class, pciDevice->Subclass);

	DbgPrint("\r\n");

	if (Value)
	{
		free(Value);
	}
}

int ReadPciBarType(
	UCHAR Bus,
	UCHAR Dev,
	UCHAR Func)
{
	PCI_BAR_INFO BarInfo[6];
	ULONG RetBytes = 0;
	ULONG Buffer = 0;

	Buffer |= (Bus << 24);
	Buffer |= (Dev << 16);
	Buffer |= (Func << 8);
	Buffer |= 0;

	RtlZeroMemory(&BarInfo, sizeof(BarInfo));

	DRIVER_REQUEST(IOCTL_RD_BAR_INFO, (void*)&Buffer, 0x4, BarInfo, sizeof(BarInfo), &RetBytes);

	DbgPrint("------------------------------------------\r\n");
	DbgPrint("| Bar Address   | Bar Type   | Address   |\r\n");
	DbgPrint("------------------------------------------\r\n");

	for (int i = 0; i < 6; i++)
	{
		UINT32 BarAddress = BarInfo[i].BarAddress;

		UINT32 barType = BarAddress & 0x1;

		UINT64 baseAddr = 0;
		if (barType == 0)
		{
			UINT32 memType = BarAddress & 0x6;
			if (memType == 0x0)
			{
				baseAddr = BarAddress & 0xFFFFFFF0;
			}
			else if (memType == 0x2)
			{
				++i;
				UINT64 NextBarAddress = BarInfo[i].BarAddress;
				baseAddr = ((BarAddress & 0xFFFFFFF0) + ((NextBarAddress & 0xFFFFFFFF) << 32));
			}
			
		}
		else if (barType == 1)
		{
			baseAddr = BarAddress & 0xFFFFFFFC;
		}

		DbgPrint("|%-15x|%-11x|%-16x|\r\n", BarAddress, barType, baseAddr);
	}

	DbgPrint("-----------------------------\r\n");

	return 0;
}


VOID EnumPciDevice(ULONG bus,int level)
{
	ULONG dev = 0;
	ULONG fun = 0;

	PCI_DEVICE* Value;
	ULONG ByteCount = 256;

	Value = (PCI_DEVICE*)malloc(ByteCount);
	RtlZeroMemory(Value, ByteCount);

	if (Value == NULL)
	{
		DbgPrint("cannot allocate memory \r\n");
		return;
	}

	for (dev = 0; dev < 32; dev++)
	{
		for (fun = 0; fun < 8; fun++)
		{
			RtlZeroMemory(Value, ByteCount);
			ReadPciDevice(bus, dev, fun, Value);
			if (Value == NULL)
			{
				DbgPrint("cannot allocate memory \r\n");
				return;
			}

			UINT32 vendorID = Value->VendorID;
			UINT8 classCode = Value->Class;
			UINT8 subClassCode = Value->Subclass;
			UINT8 headerType = Value->HeaderType;
			if (vendorID != 0xffff)
			{
				for (int i = 0; i < level; i++)
				{
					DbgPrint("\t");
				}
				DbgPrint("Bus (%x,%x,%x), VendorID:%x,Class:%x,SubClass:%d,HeaderType:%x\r\n", bus, dev, fun, vendorID, classCode, subClassCode, headerType);

				if (headerType == 0x1 || headerType == 0x81)
				{
					PCI_BRIDGE* bridgeValue = (PCI_BRIDGE*)(Value);
					UINT8 secondBus = bridgeValue->SecondaryBus;
					EnumPciDevice(secondBus, level+1);
				}
			}
		}
	}

	if (Value)
	{
		free(Value);
	}
}

VOID EnumPciTree()
{
	ULONG bus = 0;

	EnumPciDevice(bus,0);
}

ULONG64 ReadMsr(ULONG64 index)
{
	ULONG RetBytes = 0;
	ULONG64 Index = index;
	ULONG64 Buffer = 0;
	DRIVER_REQUEST(IOCTL_RDMSR, (void*)&Index, sizeof(ULONG64), (void*)&Buffer, sizeof(ULONG64), &RetBytes);

	return Buffer;
}
EXTERN_C_END

void Unload(char* ServiceName)
{
	DbgPrint("[+] Uninstalling Driver Service ... \r\n");
	if (!miniDriverCtrl.Stop(ServiceName)) {
		LOG_LAST_ERROR(L"[-] UnLoading Driver Failed \r\n");
		return;
	}
	if (!miniDriverCtrl.Remove(ServiceName)) {
		LOG_LAST_ERROR(L"[-] Removing Driver Failed \r\n");
		return;
	}
	DbgPrint("[+] Uninstall Driver Service Successfully ServiceName= %s \r\n", ServiceName);
}

void Load(char* DrvPath, char* ServiceName)
{
	DbgPrint("[+] Installing Driver Service ...  \r\n");
	miniDriverCtrl.Install(DrvPath, ServiceName, DISPLAY_NAME);
	if (!miniDriverCtrl.Start(ServiceName))
	{
		LOG_LAST_ERROR(L"[-] Install Driver Failure \r\n");
		miniDriverCtrl.Remove(ServiceName);
		return;
	}
	DbgPrint("[+] Install Driver Successfully DrvPath= %s ServiceName= %s \r\n", DrvPath, ServiceName);
}

void Unload()
{
	Unload(SERVICE_NAME);
}

void Load()
{
	CHAR Dir[512] = { 0 };
	CHAR DriverName[] = "\\MiniTool.sys";
	int Index = GetCurrentDirectoryA(512, Dir);
	strcpy_s(&Dir[Index], sizeof(DriverName), DriverName);
	DbgPrint("DriverPath= %s \r\n SerivceName= MiniTool\r\n", Dir);
	Load(Dir, SERVICE_NAME);
}

void PrintMenu()
{
	DbgPrint("---------------------------------------------------------------------------------------------------------------- \r\n");
	DbgPrint("| Mini Tools                                                                                                   | \r\n");
	DbgPrint("|                                                                                                              | \r\n");
	DbgPrint("|--------------------------------------------------------------------------------------------------------------| \r\n");
	DbgPrint("| Option | Parameters                           | Description                                                  | \r\n");
	DbgPrint("---------------------------------------------------------------------------------------------------------------| \r\n");
	DbgPrint("|  -h                                                              Show This Menu                              | \r\n");
	DbgPrint("|  -l                                                              Load MiniTool                               | \r\n");
	DbgPrint("|  -l      <DriverPath> <SerciceName>                              Load Kernel Driver                          | \r\n");
	DbgPrint("|  -u      <ServiceName>                                           Unload Driver                               | \r\n");
	DbgPrint("|  -r     -readpa <physical address> <bytes count>                 Read the number of bytes for a given PA     | \r\n");
	DbgPrint("|  -r     -rdmsr  <index>                                          Read MSR OS                                 | \r\n");
	DbgPrint("|  -r     -pcicfg <Bus> <Device> <Function>                        Read the PCI config                         | \r\n");
	DbgPrint("|  -r     -readbarpa <physical address> <offset> <bytes count>     Read Bar Address                            | \r\n");
	DbgPrint("|  -r     -mmcfg <Bus> <Device> <Function> <offset> <bytes count>  Read mmcfg                                  | \r\n");
	DbgPrint("|  -r     -enumpciio                                               Enum PCI Device(CFC)                        | \r\n");
	DbgPrint("|  -r     -enumpcie                                                Enum PCI Device                             | \r\n");
	DbgPrint("|  -r     -pcitree                                                 Enum PCI Tree                               | \r\n");
	DbgPrint("|  -r     -devicetype <Bus> <Device> <Function>                    Read Device Type                            | \r\n");
	DbgPrint("|  -r     -bartype <Bus> <Device> <Function>                       Read Bar Type                               | \r\n");
	DbgPrint("|  -q                                                              Quit  Application                           | \r\n");
	DbgPrint("---------------------------------------------------------------------------------------------------------------- \r\n");

}

int ParseParam(char* param, char** ret)
{
	int i = 0;
	char* next = nullptr;
	char* ptr = strtok_s(param, " ", &next);
	while (ptr != NULL)
	{
		///DbgPrint("'%s'\n", ptr);
		strcpy_s(ret[i], 256, ptr);
		ptr = strtok_s(NULL, " ", &next);
		i++;
	}
	return i;
}

char** GetParameter(char* param, int* _count)
{
	char** x = (char**)(malloc(sizeof(ULONG_PTR) * 128));
	if (!x) {
		return x;
	}

	for (int i = 0; i < 128; i++) {
		x[i] = (char*)malloc(256);
	}

	int count = ParseParam(param, x);

#ifdef DBGSTRING
	for (int i = 0; i < count; i++) {
		DbgPrint("str= %s \r\n", x[i]);
	}
#endif

	* _count = count;
	return x;
}

void FreeParameter(char** x) {
	for (int i = 0; i < 60; i++) 
	{
		free(x[i]);
		x[i] = nullptr;
	}
	free(x);
}

int main()
{
	int count = 0;
	UCHAR Buffer[4096] = { 0 };
	PrintMenu();
	Load();
	while (1)
	{
		DbgPrint("\nInput Command [-q to quit] : \r\n");
		char param[4096] = { 0 };
		fgets(param, 4096, stdin);
		size_t len = strlen(param);
		if (len > 0 && param[len - 1] == '\n') 
		{
			param[--len] = '\0';
		}
		if (!strncmp(param, "-l", 2)) 
		{
			char** x = GetParameter(param, &count);
			if (count < 0) 
			{
				continue;
			}

			if (count == 1) 
			{
				Load();
				continue;
			}

			if (!strlen(x[1])) 
			{
				DbgPrint("Please Input Driver Path to be loaded \r\n");
				continue;
			}
			if (!strlen(x[2])) 
			{
				DbgPrint("Please Input Service Name to be started \r\n");
				continue;
			}

			Load(x[1], x[2]);
			FreeParameter(x);
			x = nullptr;
		}
		else if (!strncmp(param, "-u", 2)) 
		{
			char** x = GetParameter(param, &count);
			if (count <= 0) {
				DbgPrint("Please Input Service Name to be started \r\n");
				continue;
			}
			if (count == 1) {
				Unload();
				continue;
			}

			Unload(x[1]);
			FreeParameter(x);
			x = nullptr;
		}
		else if (!strncmp(param, "-q", 2)) 
		{
			Unload();
			TerminateProcess(GetCurrentProcess(), 0);
		}
		else if (!strncmp(param, "-h", 2)) 
		{
			PrintMenu();
		}
		else if (!strncmp(param, "-r", 2)) 
		{
			char** x = GetParameter(param, &count);
			if (count <= 0) 
			{
				DbgPrint("Please Input Service Name to be started \r\n");
				continue;
			}

			if (!strcmp(x[1], "-rdmsr")) 
			{
				ULONGLONG index = strtoull(x[2], NULL, 0);
				ReadMsr(index);
			}

			if (!strcmp(x[1], "-pcicfg")) 
			{
				UCHAR Bus = strtoull(x[2], NULL, 0) & 0xFF;
				UCHAR Dev = strtoull(x[3], NULL, 0) & 0xFF;
				UCHAR Func = strtoull(x[4], NULL, 0) & 0xFF;
				ULONG64 BytePrinted = strtoull(x[5], NULL, 0);
				ULONG64 Alignment = strtoull(x[6], NULL, 0);
				ReadBarInfo(Bus, Dev, Func);
				ReadPciCfg(Bus, Dev, Func, BytePrinted, Alignment);
			}

			if (!strcmp(x[1], "-readpa")) 
			{

				ULONG64 Address = strtoull(x[2], NULL, 0);
				ULONG64 ByteCount = strtoull(x[3], NULL, 0);
				ULONG64 Alignment = strtoull(x[4], NULL, 0);
				ReadPhysicalMemory(Address, ByteCount, Alignment);
			}

			if (!strcmp(x[1], "-mmcfg"))
			{
				UCHAR Bus = strtoull(x[2], NULL, 0) & 0xFF;
				UCHAR Dev = strtoull(x[3], NULL, 0) & 0xFF;
				UCHAR Func = strtoull(x[4], NULL, 0) & 0xFF;
				ULONG64 Offset = strtoull(x[5], NULL, 0);
				ULONG64 BytePrinted = strtoull(x[6], NULL, 0);
				ULONG64 Alignment = strtoull(x[7], NULL, 0);

				ReadMMCFG(Bus, Dev, Func, Offset, BytePrinted, Alignment);
			}

			if (!strcmp(x[1], "-readbarpa")) 
			{

				ULONG64 Address = strtoull(x[2], NULL, 0);
				ULONG64 Offset = strtoull(x[3], NULL, 0);
				ULONG64 ByteCount = strtoull(x[4], NULL, 0);
				ULONG64 Alignment = strtoull(x[5], NULL, 0);
				ReadBarPhysicalMemory(Address, Offset, ByteCount, Alignment);
			}

			if (!strcmp(x[1], "-enumpciio"))
			{
				EnumPCIIOAll();
			}

			if (!strcmp(x[1], "-enumpcie"))
			{
				EnumPCIEAll();
			}

			if (!strcmp(x[1], "-pcitree"))
			{
				EnumPciTree();
			}

			if (!strcmp(x[1], "-devicetype"))
			{
				UCHAR Bus = strtoull(x[2], NULL, 0) & 0xFF;
				UCHAR Dev = strtoull(x[3], NULL, 0) & 0xFF;
				UCHAR Func = strtoull(x[4], NULL, 0) & 0xFF;

				ReadPciDeviceType(Bus, Dev, Func);
			}

			if (!strcmp(x[1], "-bartype"))
			{
				UCHAR Bus = strtoull(x[2], NULL, 0) & 0xFF;
				UCHAR Dev = strtoull(x[3], NULL, 0) & 0xFF;
				UCHAR Func = strtoull(x[4], NULL, 0) & 0xFF;

				ReadPciBarType(Bus,Dev,Func);
			}


			FreeParameter(x);
		}
		else if (!strncmp(param, "-w", 2)) 
		{
			char** x = GetParameter(param, &count);
			if (count <= 0)
			{
				DbgPrint("Please Input Service Name to be started \r\n");
				continue;
			}

			if (!strcmp(x[1], "-writepa"))
			{
				ULONG64 Address = strtoull(x[2], NULL, 0);
				ULONG64 Data = strtoull(x[3], NULL, 0);
				ULONG64 ByteCount = strtoull(x[4], NULL, 0);
				ULONG64 Alignment = strtoull(x[5], NULL, 0);
				WritePhysicalMemory(Address, Data, ByteCount, Alignment);
			}
		}


	}
	return 0;
}
