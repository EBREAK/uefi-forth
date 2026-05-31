#include "forth.h"
#include "wstring.h"
#include "forth_interpret_test.h"

const CHAR16 test_find_0[] = L"";
const CHAR16 test_find_1[] = L"NOOP";
const CHAR16 test_find_2[] = L"FORTHISAWSOME";
const CHAR16 test_number_0[] = L"";
const CHAR16 test_number_1[] = L"$";
const CHAR16 test_number_2[] = L"$1";
const CHAR16 test_number_3[] = L"$19AF";
const CHAR16 test_number_4[] = L"$19AFM";

const CHAR16 FORTH_MOTD[] =
	L" _____ ___  ____ _____ _   _ \r\n"
	L"|  ___/ _ \\|  _ \\_   _| | | |\r\n"
	L"| |_ | | | | |_) || | | |_| |\r\n"
	L"|  _|| |_| |  _ < | | |  _  |\r\n"
	L"|_|   \\___/|_| \\_\\|_| |_| |_|\r\n"
	L" UEFI Event-Driven Multitasking Forth Interpreter \r\n"
	L" STACK DEPTH: 128 | DICT RAM: 8MIB \r\n"
	L"\r\n";

void forth_test_interpret(void)
{
	// FIND
	LIT(&test_find_0[0]);
	LIT(wstrlen(test_find_0));
	COMPILE(L"FIND", L"$0", L"=CHK");
	LIT(&test_find_1[0]);
	LIT(wstrlen(test_find_1));
	COMPILE(L"FIND");
	LIT((uintptr_t)forth_find(L"NOOP"));
	COMPILE(L"=CHK");
	LIT(&test_find_1[0]);
	LIT(FORTH_NAME_MAXLEN + 1);
	COMPILE(L"FIND", L"$0", L"=CHK");
	LIT(&test_find_2[0]);
	LIT(wstrlen(test_find_2));
	COMPILE(L"FIND", L"$0", L"=CHK", L"PZCHK");

	// [ ] COMPILE?
	COMPILE(L"]", L"COMPILE?", L"TRUE", L"=CHK");
	COMPILE(L"[", L"COMPILE?", L"FALSE", L"=CHK", L"PZCHK");

	// IMMEDIATE?
	LIT((uintptr_t)forth_find(L"NOOP"));
	COMPILE(L"IMMEDIATE?", L"FALSE", L"=CHK");
	LIT((uintptr_t)forth_find(L"["));
	COMPILE(L"IMMEDIATE?", L"TRUE", L"=CHK", L"PZCHK");

	// WIB WIN WIB-RST WIB-PUSH FORTH-NAME-MAXLEN
	COMPILE(L"WIB-RST", L"WIN", L"X@", L"$0", L"=CHK");
	COMPILE(L"$5", L"WIB-PUSH", L"WIB", L"W@", L"$5", L"=CHK", L"WIN",
		L"X@", L"$1", L"=CHK");
	COMPILE(L"$A", L"WIB-PUSH", L"WIB", L"$2", L"+", L"W@", L"$A", L"=CHK",
		L"WIN", L"X@", L"$2", L"=CHK");
	COMPILE(L"FORTH-NAME-MAXLEN", L"WIN", L"X!");
	COMPILE(L"$F", L"WIB-PUSH", L"WIB", L"W@", L"$F", L"=CHK", L"WIN",
		L"X@", L"$1", L"=CHK");
	COMPILE(L"WIB-RST", L"WIN", L"X@", L"$0", L"=CHK", L"PZCHK");

	// DELIM?
	COMPILE(L"$A", L"DELIM?", L"TRUE", L"=CHK");
	COMPILE(L"$D", L"DELIM?", L"TRUE", L"=CHK");
	LIT(L' ');
	COMPILE(L"DELIM?", L"TRUE", L"=CHK");
	LIT(L'A');
	COMPILE(L"DELIM?", L"FALSE", L"=CHK", L"PZCHK");

// WIB-WORD
#if 0
    BEGIN();
    COMPILE(L"WIB-WORD", L"WIB", L"WIN", L"X@", L"WTYPE");
    AGAIN();
#endif

	// NUMBER? NUMBER
	LIT((uintptr_t)test_number_0);
	LIT(wstrlen(test_number_0));
	COMPILE(L"NUMBER?", L"FALSE", L"=CHK");
	LIT((uintptr_t)test_number_1);
	LIT(wstrlen(test_number_1));
	COMPILE(L"NUMBER?", L"FALSE", L"=CHK");
	LIT((uintptr_t)test_number_2);
	LIT(wstrlen(test_number_2));
	COMPILE(L"NUMBER?", L"TRUE", L"=CHK");
	LIT((uintptr_t)test_number_3);
	LIT(wstrlen(test_number_3));
	COMPILE(L"NUMBER?", L"TRUE", L"=CHK");
	LIT((uintptr_t)test_number_4);
	LIT(wstrlen(test_number_4));
	COMPILE(L"NUMBER?", L"FALSE", L"=CHK", L"PZCHK");
	LIT((uintptr_t)test_number_2);
	LIT(wstrlen(test_number_2));
	COMPILE(L">NUMBER");
	LIT(0x1);
	COMPILE(L"=CHK", L"PZCHK");
	LIT((uintptr_t)test_number_3);
	LIT(wstrlen(test_number_3));
	COMPILE(L">NUMBER");
	LIT(0x19AF);
	COMPILE(L"=CHK", L"PZCHK");

	// XT>WNAME XT>WNLEN XT>PREV XT>BODY
	struct forth_word *wtmp;
	wtmp = forth_find(L"NOOP");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>WNAME");
	LIT(wtmp->name);
	COMPILE(L"=CHK");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>WNLEN");
	LIT(wstrlen(wtmp->name));
	COMPILE(L"=CHK");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>PREV");
	LIT(wtmp->prev);
	COMPILE(L"=CHK");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>BODY");
	LIT(wtmp->body);
	COMPILE(L"=CHK");

	wtmp = forth_find(L"DUP");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>WNAME");
	LIT(wtmp->name);
	COMPILE(L"=CHK");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>WNLEN");
	LIT(wstrlen(wtmp->name));
	COMPILE(L"=CHK");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>PREV");
	LIT(wtmp->prev);
	COMPILE(L"=CHK");
	LIT((uintptr_t)wtmp);
	COMPILE(L"XT>BODY");
	LIT(wtmp->body);
	COMPILE(L"=CHK", L"PZCHK");

	// SHOW MOTD
	LIT(&FORTH_MOTD[0]);
	LIT(wstrlen(FORTH_MOTD));
	COMPILE(L"WTYPE");

	// DEFAULT ENABLE EARLY INPUT ECHO
	COMPILE(L"EARLY-ECHO-ON");

	// INTERPRET INTERPRET-FIX INTERPRET-LOOP
	COMPILE(L"INTERPRET-LOOP");
}
