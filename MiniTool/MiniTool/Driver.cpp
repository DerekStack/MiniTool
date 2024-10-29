
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

			ULONG busIndex = (Buf >> 24) & 0xFF;

			HalGetBusDataByOffset(
				PCIConfiguration,
				busIndex,
				slot.u.AsULONG,
				outputBuffer,
				0,
				256);

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

			UINT16 PCIVenderID = 0;
			UINT16 PCIDevID = 0;
			ULONG offset = 0;
			ULONG devOffset = 0x2;

			ULONG indexBus = (Buf >> 24) & 0xFF;
			ULONG indexDevice = (Buf >> 16) & 0xFF;
			ULONG indexFunciton = (Buf >> 8) & 0xFF;

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

					PciInfo->PCIID = (indexBus << 24) | (indexDevice << 16) | (indexFunciton << 8);
					PciInfo->u.VendorID = PCIVenderID;
					PciInfo->u.DeviceID = PCIDevID;
			}

			Irp->IoStatus.Information = sizeof(PCI_INFO);
		}
		break;
		case IOCTL_ENUM_PCI:
		{
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

			UINT16 PCIVenderID = 0;
			UINT16 PCIDevID = 0;
			ULONG offset = 0;
			ULONG devOffset = 0x2;

			ULONG indexBus = (Buf >> 24) & 0xFF;
			ULONG indexDevice = (Buf >> 16) & 0xFF;
			ULONG indexFunciton = (Buf >> 8) & 0xFF;

			PCIVenderID = ReadPCICfg(indexBus, indexDevice, indexFunciton, offset, 2);

			if (PCIVenderID != 0xffff)
			{
				PCIDevID = ReadPCICfg(indexBus, indexDevice, indexFunciton, devOffset, 2);
				PciInfo->PCIID = (indexBus << 24) | (indexDevice << 16) | (indexFunciton << 8);
				PciInfo->u.VendorID = PCIVenderID;
				PciInfo->u.DeviceID = PCIDevID;	
				
			}

			Irp->IoStatus.Information = sizeof(PCI_INFO);
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