#pragma once
#ifdef _KERNEL_MODE
#include <fltKernel.h>
#else
#include <windows.h>
#endif


#define FILE_DEVICE_HIDE	0x8000

#define IOCTL_BASE			 0x800

#define CTL_CODE_HIDE(i)	\
	CTL_CODE(FILE_DEVICE_HIDE, IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_RD_PCICFG			  CTL_CODE_HIDE(100)
#define IOCTL_RD_PHYSMEM		  CTL_CODE_HIDE(101)   
#define IOCTL_RDMSR				  CTL_CODE_HIDE(102)
#define IOCTL_RD_MMCFG			  CTL_CODE_HIDE(103)
#define IOCTL_RD_MBAR			  CTL_CODE_HIDE(104)
#define IOCTL_RD_BAR_INFO		  CTL_CODE_HIDE(105) 

#define IOCTL_WR_PHYSMEM		  CTL_CODE_HIDE(110)
#define IOCTL_WR_MSR			  CTL_CODE_HIDE(111)
#define IOCTL_WR_MBAR			  CTL_CODE_HIDE(112)
#define IOCTL_WR_MMCFG			  CTL_CODE_HIDE(113)

#define IOCTL_ENUM_PCI			  CTL_CODE_HIDE(120)
#define IOCTL_ENUM_PCIE			  CTL_CODE_HIDE(121)

typedef struct _PCI_BAR_INFO
{
	UINT32 BarAddress;
	UINT32 BarSize;

}PCI_BAR_INFO, * PPCI_BAR_INFO;

typedef struct _PCI_INFO
{
	UINT32 PCIID;
	union {
		struct {
			UINT16   VendorID : 16;
			UINT16   DeviceID : 16;
		};
		UINT32 AsUINT32;
	}u;

}PCI_INFO, * PPCI_INFO;
