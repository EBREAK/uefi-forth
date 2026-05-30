#pragma once

#include <efi.h>
#include <stdbool.h>

extern EFI_HANDLE GIH;
extern EFI_SYSTEM_TABLE *GST;

extern struct fifo16 *early_wkey_fifo;
extern bool early_echo;
