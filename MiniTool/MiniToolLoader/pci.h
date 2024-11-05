#ifndef __PCI_H__
#define __PCI_H__

#define NO_OF_VENDOR		991
#define NO_OF_CLASS         26//0xFF
#define NO_OF_SUBCLASS      111//0xFF
#define NO_OF_PROGIF        67//0xFF

typedef struct _PCI_VENDOR
{
	unsigned short VendorID;
	const char* VendorName;
}PCI_VENDOR, * PPCI_VENDOR;


typedef struct _PCI_CLASS
{
	unsigned short ClassCode;
	const char* ClassName;
	bool SubClassExist;
}PCI_CLASS, * PPCI_CLASS;

typedef struct _PCI_SUBCLASS
{
	unsigned short SubclassCode;
	const char* SubclassName;
	bool ProgIfExist;
}PCI_SUBCLASS, * PPCI_SUBCLASS;

typedef struct _PCI_PROGIF
{
	unsigned int ProgIfCode;
	const char* ProgIfName;
}PCI_PROGIF, * PPCI_PROGIF;


typedef struct _PCI_DEVICE_SUBTYPE
{
	PCI_SUBCLASS SubclassObj;
	const PCI_PROGIF* ProgIfObjs;

}PCI_DEVICE_SUBTYPE, * PPCI_DEVICE_SUBTYPE;

typedef struct _PCI_DEVICE_TYPE
{
	PCI_CLASS ClassObj;
	PCI_DEVICE_SUBTYPE* SubclassObjs;

}PCI_DEVICE_TYPE,* PPCI_DEVICE_TYPE;


PCI_VENDOR* FindVendorData(unsigned short id);
PCI_CLASS* FindClassData(unsigned short classCode);
PCI_SUBCLASS* FindSubClassData(unsigned short subClassCode);
PCI_PROGIF* FindProgifData(unsigned int progIfCode);

#endif
