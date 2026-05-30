#include "forth.h"
#include "forth_test.h"
#include "forth_core_test.h"
#include "forth_io_test.h"
#include "forth_interpret_test.h"

void forth_init_test(void)
{
	DFWH(L"SELFTEST");
	FORTH_SELFTEST = FORTH_DP;
	forth_test_core();
	forth_test_io();
	forth_test_interpret();
	// NOOP HALT
	COMPILE(L"NOOP", L"HALT");
	ENDW();
}
