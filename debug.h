#pragma once

#include <efi.h>

extern void debug_putwc(CHAR16 wc);
extern void debug_putws(CHAR16 *ws);
#define debug_cr() debug_putws(L"\r\n")
extern void debug_putnhex(unsigned long long number, unsigned long long width);
#define debug_puthex(x) debug_putnhex((x), sizeof(x) * 2);
