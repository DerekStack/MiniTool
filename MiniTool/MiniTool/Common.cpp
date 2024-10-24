
#include <fltKernel.h>
#include "Common.h"



bool UtilReadMsr64(
	_In_ Msr msr,
	_Out_ ULONG64* value
)
{
	bool ret = true;
	__try {
		*value = (ULONG64)(__readmsr((unsigned long)(msr)));
	}
	__except (1) {
		DbgPrintEx(0, 0, "[WARN] MSR(0x%X) not exist \r\n", msr);
		ret = false;
	}
	return ret;
}

bool UtilWriteMsr64(
	_In_ Msr msr,
	_In_ ULONG64 value
)
{
	bool ret = true;
	__try {
		__writemsr((unsigned long)(msr), value);
	}
	__except (1) {
		DbgPrintEx(0, 0, "[WARN] MSR(0x%X) not exist \r\n", msr);
		ret = false;
	}
	return ret;
}