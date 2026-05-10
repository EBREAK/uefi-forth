#include <efi.h>
#include <efilib.h>

EFI_HANDLE GIH = NULL;
EFI_SYSTEM_TABLE *GST = NULL;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle,
			   EFI_SYSTEM_TABLE *SystemTable)
{
	GST = SystemTable;
	GIH = ImageHandle;
	GST->ConOut->OutputString(GST->ConOut, L"HELLO WORLD");
	while(1);
}
