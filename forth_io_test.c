#include "forth.h"
#include "wstring.h"
#include "forth_io_test.h"

const CHAR16 test_wtype_str[] = L"HELLO WORLD\r\n";

void forth_test_io(void)
{
	// EARLY-WEMIT
#if 0
    LIT(L'\n'); COMPILE(L"EARLY-WEMIT", L"PZCHK");
    LIT(L'T'); COMPILE(L"EARLY-WEMIT", L"PZCHK");
    LIT(L'\n'); COMPILE(L"EARLY-WEMIT", L"PZCHK");
#endif

	// EARLY-WKEY
#if 0
    BEGIN();
    COMPILE(L"EARLY-WKEY", L"EARLY-WEMIT", L"PZCHK");
    AGAIN();
#endif

	// WEMIT WKEY
#if 0
    BEGIN();
    COMPILE(L"WKEY", L"WEMIT", L"PZCHK");
    AGAIN();
#endif

	// WTYPE
	COMPILE(L"$A", L"$0", L"WTYPE", L"PZCHK");
#if 0
    LIT((uintptr_t)&test_wtype_str[0]); LIT(wstrlen(test_wtype_str));
    COMPILE(L"WTYPE", L"PZCHK");
#endif

	// NUM4BIT>WHEX
	COMPILE(L"$0", L"NUM4BIT>WHEX");
	LIT(L'0');
	COMPILE(L"=CHK");
	COMPILE(L"$1", L"NUM4BIT>WHEX");
	LIT(L'1');
	COMPILE(L"=CHK");
	COMPILE(L"$8", L"NUM4BIT>WHEX");
	LIT(L'8');
	COMPILE(L"=CHK");
	COMPILE(L"$9", L"NUM4BIT>WHEX");
	LIT(L'9');
	COMPILE(L"=CHK");
	COMPILE(L"$A", L"NUM4BIT>WHEX");
	LIT(L'A');
	COMPILE(L"=CHK");
	COMPILE(L"$B", L"NUM4BIT>WHEX");
	LIT(L'B');
	COMPILE(L"=CHK");
	COMPILE(L"$E", L"NUM4BIT>WHEX");
	LIT(L'E');
	COMPILE(L"=CHK");
	COMPILE(L"$F", L"NUM4BIT>WHEX");
	LIT(L'F');
	COMPILE(L"=CHK", L"PZCHK");

	// NHEX. XHEX.
	COMPILE(L"$F", L"$0", L"NHEX.", L"PZCHK");
#if 0
    LIT(0x7654321089ABCDEFULL);
    COMPILE(L"DUP", L"$1", L"NHEX.");
    COMPILE(L"DUP", L"$2", L"NHEX.");
    COMPILE(L"DUP", L"$4", L"NHEX.");
    COMPILE(L"DUP", L"$8", L"NHEX.");
    COMPILE(L"DUP", L"$F", L"NHEX.");
    COMPILE(L"DUP", L"$F", L"1+", L"NHEX.");
    COMPILE(L"XHEX.", L"PZCHK");
#endif

	// .
#if 0
    COMPILE(L"$0", L".", L"PZCHK");
    LIT(0x7654321089ABCDEFULL);
    COMPILE(L".", L"PZCHK");
#endif

	// .S
#if 0
    COMPILE(L".S", L"PZCHK");
    COMPILE(L"$1", L".S", L"$2", L".S", L"DROP", L".S", L"DROP", L".S", L"PZCHK");
#endif
}
