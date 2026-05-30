#include "debug.h"

#include "main.h"

void debug_putwc(CHAR16 wc)
{
	CHAR16 out[2];
	out[0] = wc;
	out[1] = '\0';
	GST->ConOut->OutputString(GST->ConOut, out);
}

void debug_putws(CHAR16 *ws)
{
	GST->ConOut->OutputString(GST->ConOut, ws);
}

void debug_putnhex(unsigned long long number, unsigned long long width)
{
	const CHAR16 num2hex_lut[] = L"0123456789ABCDEF";
	while (width > 0) {
		debug_putwc(num2hex_lut[(number >> ((width - 1) * 4)) & 0xF]);
		width -= 1;
	}
}
