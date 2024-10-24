
#include <fltKernel.h>
#include "ioctl.h"
#include "Common.h"
#include "Util.h"


#define PAGE_ALIGN(Va) (((ULONG_PTR)(Va) & ~(PAGE_SIZE - 1)))
#define ROUND_UP_TO_PAGE_SIZE(size) (((size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))


EXTERN_C_START

typedef struct _DEVICE_EXTENSION
{
	ULONG  Test;
} DEVICE_EXTENSION, * PDEVICE_EXTENSION;

PDEVICE_OBJECT		g_DeviceObject = NULL;


#define MINITOOL_WIN32_DEVICE_NAME_A		"\\\\.\\MiniTool"
#define MINITOOL_WIN32_DEVICE_NAME_W		L"\\\\.\\MiniTool"
#define MINITOOL_DEVICE_NAME_A			"\\Device\\MiniTool"
#define MINITOOL_DEVICE_NAME_W			L"\\Device\\MiniTool"
#define MINITOOL_DOS_DEVICE_NAME_A		"\\DosDevices\\MiniTool"
#define MINITOOL_DOS_DEVICE_NAME_W		L"\\DosDevices\\MiniTool"

NTSTATUS MiniDeviceCtrlRoutine(
	_In_ PDEVICE_OBJECT		DeviceObject,
	_In_ PIRP					Irp
)
{
	NTSTATUS			status = STATUS_SUCCESS;
	PIO_STATUS_BLOCK	ioStatus;
	PIO_STACK_LOCATION	pIrpStack;
	PDEVICE_EXTENSION	deviceExtension;
	PVOID				inputBuffer, outputBuffer;
	ULONG				inputBufferLength, outputBufferLength;
	ULONG				ioControlCode;


	pIrpStack = IoGetCurrentIrpStackLocation(Irp);
	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	ioStatus = &Irp->IoStatus;
	ioStatus->Status = STATUS_SUCCESS;		// Assume success
	ioStatus->Information = 0;              // Assume nothing returned

	inputBuffer = Irp->AssociatedIrp.SystemBuffer;
	inputBufferLength = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	outputBuffer = Irp->AssociatedIrp.SystemBuffer;
	outputBufferLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	ioControlCode = pIrpStack->Parameters.DeviceIoControl.IoControlCode;


	switch (pIrpStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
	case IRP_MJ_CLOSE:
	case IRP_MJ_SHUTDOWN:
		break;

	case IRP_MJ_DEVICE_CONTROL:
		switch (ioControlCode)
		{
		case IOCTL_RD_PHYSMEM:
		{
			//waitting test
			if (!inputBuffer || !outputBuffer) {
				break;
			}

			PHYSICAL_ADDRESS PhysAddr;
			ULONG64 Size;

			Size = ((ULONG64*)inputBuffer)[1];
			ULONG64 readPhyAddr = ((ULONG64*)inputBuffer)[0];

			ULONG64 SourceAddr = PAGE_ALIGN(readPhyAddr);
			ULONG64 OffsetDur = readPhyAddr - SourceAddr;
			ULONG64 readSize = ROUND_UP_TO_PAGE_SIZE((readPhyAddr + Size - 1) - SourceAddr);

			PhysAddr.QuadPart = SourceAddr;

			DbgPrintEx(0, 0, "Reading Physical Address= %I64x, Size= %I64x \r\n", PhysAddr.QuadPart, Size);

			UCHAR* MappedAddress = (UCHAR*)MmMapIoSpace(PhysAddr, readSize, MmNonCached);//readSize
			if (!MappedAddress || !outputBuffer) {
				break;
			}

			//RtlCopyBytes(outputBuffer, MappedAddress + PAGE_ALIGN(readPhyAddr), Size);//+ PAGE_ALIGN(readPhyAddr)
			MemoryCopyValue(outputBuffer, MappedAddress + OffsetDur, Size);
			MmUnmapIoSpace(MappedAddress, readSize);
			MappedAddress = NULL;
			Irp->IoStatus.Information = Size;
		}
		break;
		case IOCTL_WR_PHYSMEM:
		{
			if (!inputBuffer || !outputBuffer) {
				break;
			}

			PHYSICAL_ADDRESS PhysAddr;

			ULONG64 Size;

			Size = ((ULONG64*)inputBuffer)[2];
			ULONG64 data = ((ULONG64*)inputBuffer)[1];
			ULONG64 readPhyAddr = ((ULONG64*)inputBuffer)[0];

			if (Size > 8)
			{
				break;
			}

			ULONG64 SourceAddr = PAGE_ALIGN(readPhyAddr);
			ULONG64 OffsetDur = readPhyAddr - SourceAddr;
			ULONG64 readSize = ROUND_UP_TO_PAGE_SIZE((readPhyAddr + Size - 1) - SourceAddr);

			PhysAddr.QuadPart = SourceAddr;

			DbgPrintEx(0, 0, "Reading Physical Address= %I64x, Size= %I64x \r\n", PhysAddr.QuadPart, Size);

			UCHAR* MappedAddress = (UCHAR*)MmMapIoSpace(PhysAddr, readSize, MmNonCached);//readSize
			if (!MappedAddress || !outputBuffer) {
				break;
			}

			MemoryCopyValue(outputBuffer, MappedAddress + OffsetDur, Size);

			MemoryCopyValue(MappedAddress + OffsetDur, &data, Size);
			MmUnmapIoSpace(MappedAddress, readSize);
			MappedAddress = NULL;
		}
		break;
		case IOCTL_RD_BAR_INFO:
		{
			//waitting test
			PCI_SLOT_NUMBER slot;
			ULONG Buf = *(ULONG*)inputBuffer;
			UINT32 OriginalBar = 0;
			UINT32 DummyBar = 0;
			PPCI_BAR_INFO BarInfo;

			if (!inputBuffer || !outputBuffer)
			{
				break;
			}
			BarInfo = (PPCI_BAR_INFO)outputBuffer;

			slot.u.AsULONG = 0;
			slot.u.bits.DeviceNumber = (Buf >> 16) & 0xFF;
			slot.u.bits.FunctionNumber = (Buf >> 8) & 0xFF;

			for (int i = 0; i < 6; i++)
			{
				UINT32 BarOffset = 0x10 + i * 4;
				UINT32 BarSize = sizeof(UINT32);

				if (!HalGetBusDataByOffset(
					PCIConfiguration,
					(Buf >> 24) & 0xFF,
					slot.u.AsULONG,
					&OriginalBar,
					BarOffset,
					BarSize))
				{
					continue;
				}

				if (OriginalBar == 0)
				{
					continue;
				}

				DummyBar = 0xFFFFFFFF;

				HalSetBusDataByOffset(
					PCIConfiguration,
					(Buf >> 24) & 0xFF,
					slot.u.AsULONG,
					&DummyBar,
					BarOffset,
					BarSize);

				HalGetBusDataByOffset(
					PCIConfiguration,
					(Buf >> 24) & 0xFF,
					slot.u.AsULONG,
					&DummyBar,
					BarOffset,
					BarSize);

				HalSetBusDataByOffset(
					PCIConfiguration,
					(Buf >> 24) & 0xFF,
					slot.u.AsULONG,
					&OriginalBar,
					BarOffset,
					BarSize);

				BarInfo[i].BarAddress = OriginalBar;
				BarInfo[i].BarSize = DummyBar;
			}

			Irp->IoStatus.Information = sizeof(PCI_BAR_INFO) * 6;
		}
		break;
		case IOCTL_RD_PCICFG:
		{
			//waitting test
			PCI_SLOT_NUMBER slot;
			ULONG Buf = *(ULONG*)inputBuffer;

			if (!inputBuffer || !outputBuffer) {
				break;
			}

			DbgPrintEx(0, 0, "Bus= %x, Dev= %x, Func= %x, Offset= %x \r\n",
				(Buf >> 24) & 0xFF,
				(Buf >> 16) & 0xFF,
				(Buf >> 8) & 0xFF,
				Buf & 0xFF);

			slot.u.AsULONG = 0;
			slot.u.bits.DeviceNumber = (Buf >> 16) & 0xFF;
			slot.u.bits.FunctionNumber = (Buf >> 8) & 0xFF;



			Irp->IoStatus.Information = outputBufferLength;
		}
		break;
		case IOCTL_RDMSR:
		{
			if (!inputBuffer || !outputBuffer) {
				break;
			}

			ULONG64 msr = 0;
			if (!UtilReadMsr64((Msr)((PULONG)inputBuffer)[0], &msr)) {
				DbgPrintEx(0, 0, "MSR Not support \r\n");
				Irp->IoStatus.Information = 0;
				break;
			}
			*((PULONG64)outputBuffer) = msr;
			Irp->IoStatus.Information = sizeof(ULONG64);
		}
		break;
		case IOCTL_WR_MSR:
		{
			if (!inputBuffer || !outputBuffer) {
				break;
			}

			ULONG64 msr = 0;
			ULONG msrAddr = ((PULONG)inputBuffer)[0];
			ULONG64 data = ((ULONG64*)inputBuffer)[1];

			if (!UtilReadMsr64((Msr)msrAddr, &msr)) {
				DbgPrintEx(0, 0, "MSR Not support \r\n");
				Irp->IoStatus.Information = 0;
				break;
			}

			UtilWriteMsr64((Msr)msrAddr, data);

			*((PULONG64)outputBuffer) = msr;
			Irp->IoStatus.Information = sizeof(ULONG64);
		}
		break;
		case IOCTL_RD_MMCFG:
		{
			ULONG64 dataBuf = ((ULONG64*)inputBuffer)[0];
			ULONG Buf = dataBuf;
			ULONG64 mcfgBaseAddress = ((ULONG64*)inputBuffer)[1];
			ULONG64 Offset = ((ULONG64*)inputBuffer)[2];
			ULONG64 Size = ((ULONG64*)inputBuffer)[3];
			ULONG BufferLength = 60;
			ULONG ReturnLength = 0;


			if (!inputBuffer || !outputBuffer)
			{
				DbgPrintEx(0, 0, "[ERROR] Get Input Buffer\r\n");
				break;
			}

			ULONG BusNumber = (Buf >> 24) & 0xFF;
			ULONG DeviceNumber = (Buf >> 16) & 0xFF;
			ULONG FunctionNumber = (Buf >> 8) & 0xFF;

			PHYSICAL_ADDRESS PhysAddr;
			ULONG64 BaseAddress = mcfgBaseAddress;
			ULONG64 pciConfigAddress = BaseAddress + (BusNumber << 20 | DeviceNumber << 15 | FunctionNumber << 12);

			ULONG64 readPhyAddr = pciConfigAddress;
			ULONG64 SourceAddr = PAGE_ALIGN(readPhyAddr);
			ULONG64 offsetDur = readPhyAddr - SourceAddr;
			ULONG64 readSize = ROUND_UP_TO_PAGE_SIZE((readPhyAddr + Size - 1) - SourceAddr);
			PhysAddr.QuadPart = SourceAddr;
			DbgPrintEx(0, 0, "pciConfigAddress Address= %I64x BaseAddress= %I64x\r\n", pciConfigAddress, BaseAddress);

			UCHAR* ConfigMappedAddress = (UCHAR*)MmMapIoSpace(PhysAddr, readSize, MmNonCached);

			if (!ConfigMappedAddress || !outputBuffer)
			{
				DbgPrintEx(0, 0, "[ERROR] MmMapIoSpace fail\r\n");
				break;
			}

			MemoryCopyValue(outputBuffer, ConfigMappedAddress + offsetDur, Size);//+ PAGE_ALIGN(readPhyAddr)
			MmUnmapIoSpace(ConfigMappedAddress, readSize);
			ConfigMappedAddress = NULL;
			Irp->IoStatus.Information = Size;

		}
		break;
		case IOCTL_WR_MMCFG:
		{
			ULONG64 dataBuf = ((ULONG64*)inputBuffer)[0];
			ULONG Buf = dataBuf;
			ULONG64 mcfgBaseAddress = ((ULONG64*)inputBuffer)[1];
			ULONG64 Data = ((ULONG64*)inputBuffer)[2];
			ULONG64 Size = ((ULONG64*)inputBuffer)[3];
			ULONG BufferLength = 60;
			ULONG ReturnLength = 0;


			if (!inputBuffer || !outputBuffer)
			{
				DbgPrintEx(0, 0, "[ERROR] Get Input Buffer\r\n");
				break;
			}

			if (Size > 8)
			{
				DbgPrintEx(0, 0, "[ERROR] large Size\r\n");
				break;
			}

			ULONG BusNumber = (Buf >> 24) & 0xFF;
			ULONG DeviceNumber = (Buf >> 16) & 0xFF;
			ULONG FunctionNumber = (Buf >> 8) & 0xFF;

			PHYSICAL_ADDRESS PhysAddr;
			ULONG64 BaseAddress = mcfgBaseAddress;
			ULONG64 pciConfigAddress = BaseAddress + (BusNumber << 20 | DeviceNumber << 15 | FunctionNumber << 12);

			ULONG64 readPhyAddr = pciConfigAddress;
			ULONG64 SourceAddr = PAGE_ALIGN(readPhyAddr);
			ULONG64 offsetDur = readPhyAddr - SourceAddr;
			ULONG64 readSize = ROUND_UP_TO_PAGE_SIZE((readPhyAddr + Size - 1) - SourceAddr);
			PhysAddr.QuadPart = SourceAddr;


			DbgPrintEx(0, 0, "pciConfigAddress Address= %I64x BaseAddress= %I64x\r\n", pciConfigAddress, BaseAddress);

			UCHAR* ConfigMappedAddress = (UCHAR*)MmMapIoSpace(PhysAddr, readSize, MmNonCached);

			if (!ConfigMappedAddress || !outputBuffer)
			{
				DbgPrintEx(0, 0, "[ERROR] MmMapIoSpace fail\r\n");
				break;
			}

			MemoryCopyValue(outputBuffer, ConfigMappedAddress + offsetDur, Size);//+ PAGE_ALIGN(readPhyAddr)
			MemoryCopyValue(ConfigMappedAddress + offsetDur, &Data, Size);
			MmUnmapIoSpace(ConfigMappedAddress, readSize);
			ConfigMappedAddress = NULL;
			Irp->IoStatus.Information = Size;

		}
		break;
		case IOCTL_RD_MBAR:
		{
			if (!inputBuffer || !outputBuffer) {
				break;
			}

			PHYSICAL_ADDRESS PhysAddr;
			ULONG64 Size;
			ULONG64 Offset;

			Size = ((ULONG64*)inputBuffer)[2];
			Offset = ((ULONG64*)inputBuffer)[1];
			ULONG64 readPhyAddr = ((ULONG64*)inputBuffer)[0];
			readPhyAddr = readPhyAddr + Offset;

			ULONG64 SourceAddr = PAGE_ALIGN(readPhyAddr);
			ULONG64 offsetDur = readPhyAddr - SourceAddr;
			ULONG64 readSize = ROUND_UP_TO_PAGE_SIZE((readPhyAddr + Size - 1) - SourceAddr);

			PhysAddr.QuadPart = SourceAddr;

			DbgPrintEx(0, 0, "Reading Bar Address with offset= %I64x, Size= %I64x \r\n", readPhyAddr, Size);

			UCHAR* MappedAddress = (UCHAR*)MmMapIoSpace(PhysAddr, readSize, MmNonCached);
			if (!MappedAddress || !outputBuffer) {
				break;
			}

			MemoryCopyValue(outputBuffer, MappedAddress + offsetDur, Size);
			MmUnmapIoSpace(MappedAddress, readSize);
			MappedAddress = NULL;
			Irp->IoStatus.Information = Size;
		}
		break;
		case IOCTL_WR_MBAR:
		{
			if (!inputBuffer || !outputBuffer) {
				break;
			}


			PHYSICAL_ADDRESS PhysAddr;
			ULONG64 Size;
			ULONG64 Data;

			Size = ((ULONG64*)inputBuffer)[2];
			Data = ((ULONG64*)inputBuffer)[1];
			ULONG64 readPhyAddr = ((ULONG64*)inputBuffer)[0];

			if (Size > 0)
			{
				break;
			}

			ULONG64 SourceAddr = PAGE_ALIGN(readPhyAddr);
			ULONG64 offsetDur = readPhyAddr - SourceAddr;
			ULONG64 readSize = ROUND_UP_TO_PAGE_SIZE((readPhyAddr + Size - 1) - SourceAddr);

			PhysAddr.QuadPart = SourceAddr;

			DbgPrintEx(0, 0, "Write Bar Address with offset= %I64x, Size= %I64x \r\n", readPhyAddr, Size);

			UCHAR* MappedAddress = (UCHAR*)MmMapIoSpace(PhysAddr, readSize, MmNonCached);
			if (!MappedAddress || !outputBuffer) {
				break;
			}

			MemoryCopyValue(outputBuffer, MappedAddress + offsetDur, Size);
			MemoryCopyValue(MappedAddress + offsetDur, &Data, Size);
			MmUnmapIoSpace(MappedAddress, readSize);

			MappedAddress = NULL;
			Irp->IoStatus.Information = Size;
		}
		break;
		case IOCTL_ENUM_PCIE:
		{
			PCI_SLOT_NUMBER slot;
			ULONG Buf = *(ULONG*)inputBuffer;
			PPCI_INFO PciInfo;
			if (!inputBuffer || !outputBuffer) {
				break;
			}

			DbgPrintEx(0, 0, "Bus= %x, Dev= %x, Func= %x, Offset= %x \r\n",
				(Buf >> 24) & 0xFF,
				(Buf >> 16) & 0xFF,
				(Buf >> 8) & 0xFF,
				Buf & 0xFF);

			PciInfo = (PPCI_INFO)outputBuffer;


			slot.u.AsULONG = 0;
			//slot.u.bits.DeviceNumber = (Buf >> 16) & 0xFF;
			//slot.u.bits.FunctionNumber = (Buf >> 8) & 0xFF;

			//ULONG startBus = (Buf >> 24) & 0xFF;

			UINT16 PCIVenderID = 0;
			UINT16 PCIDevID = 0;
			ULONG offset = 0;
			ULONG devOffset = 0x2;

			ULONG arrIndex = 0;

			for (ULONG indexBus = 0; indexBus < 256; indexBus++)
			{
				for (ULONG indexDevice = 0; indexDevice < 32; indexDevice++)
				{
					for (ULONG indexFunciton = 0; indexFunciton < 8; indexFunciton++)
					{
						slot.u.AsULONG = 0;
						slot.u.bits.DeviceNumber = indexDevice;
						slot.u.bits.FunctionNumber = indexFunciton;

						HalGetBusDataByOffset(
							PCIConfiguration,
							indexBus,
							slot.u.AsULONG,
							&PCIVenderID,
							offset,
							2);

						if (PCIVenderID != 0xFFFF)
						{
							HalGetBusDataByOffset(
								PCIConfiguration,
								indexBus,
								slot.u.AsULONG,
								&PCIDevID,
								devOffset,
								2);

							DbgPrintEx(0, 0, "Bus= %x, Dev= %x, Func= %x, PCIVenderID= %x, PCIDevID= %x \r\n",
								indexBus,
								indexDevice,
								indexFunciton,
								PCIVenderID,
								PCIDevID);

							//ULONG index = indexBus * 256 + indexDevice * 8 + indexFunciton;

							PciInfo[arrIndex].PCIID = (indexBus << 24) | (indexDevice << 16) | (indexFunciton << 8);
							PciInfo[arrIndex].u.VendorID = PCIVenderID;
							PciInfo[arrIndex].u.DeviceID = PCIDevID;
							++arrIndex;
						}
					}
				}
			}

			Irp->IoStatus.Information = sizeof(PCI_INFO) * 65536;
		}
		break;
		case IOCTL_ENUM_PCI:
		{
			PCI_SLOT_NUMBER slot;
			ULONG Buf = *(ULONG*)inputBuffer;
			PPCI_INFO PciInfo;
			if (!inputBuffer || !outputBuffer) {
				break;
			}

			DbgPrintEx(0, 0, "Bus= %x, Dev= %x, Func= %x, Offset= %x \r\n",
				(Buf >> 24) & 0xFF,
				(Buf >> 16) & 0xFF,
				(Buf >> 8) & 0xFF,
				Buf & 0xFF);

			PciInfo = (PPCI_INFO)outputBuffer;


			slot.u.AsULONG = 0;


			UINT16 PCIVenderID = 0;
			UINT16 PCIDevID = 0;
			ULONG offset = 0;
			ULONG devOffset = 0x2;

			ULONG arrIndex = 0;

			for (ULONG indexBus = 0; indexBus < 256; indexBus++)
			{
				for (ULONG indexDevice = 0; indexDevice < 32; indexDevice++)
				{
					for (ULONG indexFunciton = 0; indexFunciton < 8; indexFunciton++)
					{
						slot.u.AsULONG = 0;
						slot.u.bits.DeviceNumber = indexDevice & 0xFF;
						slot.u.bits.FunctionNumber = indexFunciton & 0xFF;

						PCIVenderID = ReadPCICfg(indexBus, indexDevice, indexFunciton, offset, 2);

						if (PCIVenderID != 0xffff)
						{

							PCIDevID = ReadPCICfg(indexBus, indexDevice, indexFunciton, devOffset, 2);
							//ULONG index = indexBus * 256 + indexDevice * 8 + indexFunciton;

							PciInfo[arrIndex].PCIID = (indexBus << 24) | (indexDevice << 16) | (indexFunciton << 8);
							PciInfo[arrIndex].u.VendorID = PCIVenderID;
							PciInfo[arrIndex].u.DeviceID = PCIDevID;
							++arrIndex;
						}
					}
				}
			}

			Irp->IoStatus.Information = sizeof(PCI_INFO) * 65536;
		}
		break;
		}
		break;
	}

	Irp->IoStatus.Status = ioStatus->Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return  status;
}

void DestroyDeviceAndSymbolicLink()
{
	UNICODE_STRING		dosDeviceName;
	RtlInitUnicodeString(&dosDeviceName, MINITOOL_DOS_DEVICE_NAME_W);
	IoDeleteDevice(g_DeviceObject);
	g_DeviceObject = nullptr;
	IoDeleteSymbolicLink(&dosDeviceName);
}

void CreateDeviceAndSymbolicLink(
	_In_ DRIVER_OBJECT* driver_object
)
{
	UNICODE_STRING ntDeviceName = { 0 };
	UNICODE_STRING dosDeviceName = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION deviceExtension = NULL;
	RtlInitUnicodeString(&ntDeviceName, MINITOOL_DEVICE_NAME_W);
	RtlInitUnicodeString(&dosDeviceName, MINITOOL_DOS_DEVICE_NAME_W);

	status = IoCreateDevice(
		driver_object,
		sizeof(DEVICE_EXTENSION),		// DeviceExtensionSize
		&ntDeviceName,					// DeviceName
		FILE_DEVICE_UNKNOWN,			// DeviceType
		0,								// DeviceCharacteristics
		TRUE,							// Exclusive
		&g_DeviceObject					// [OUT]
	);

	if (!NT_SUCCESS(status) || !g_DeviceObject)
	{
		DbgPrintEx(0, 0, "[$XTR] IoCreateDevice failed(0x%x).\n", status);
		return;
	}

	g_DeviceObject->Flags |= DO_BUFFERED_IO;

	deviceExtension = (PDEVICE_EXTENSION)g_DeviceObject->DeviceExtension;

	status = IoCreateSymbolicLink(&dosDeviceName, &ntDeviceName);

	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(0, 0, "[$XTR] IoCreateSymbolicLink failed(0x%x).\n", status);
		return;
	}

	driver_object->MajorFunction[IRP_MJ_CREATE] =
		driver_object->MajorFunction[IRP_MJ_CLOSE] =
		driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MiniDeviceCtrlRoutine;

}

void MiniToolUnLoad(
	_In_ DRIVER_OBJECT* DriverObject
)
{
	UNREFERENCED_PARAMETER(DriverObject);

	DestroyDeviceAndSymbolicLink();

	//
	// Release all the necessary data structure needed for Intel SST
	//
}

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = MiniToolUnLoad;

	CreateDeviceAndSymbolicLink(DriverObject);

	return STATUS_SUCCESS;
}


EXTERN_C_END