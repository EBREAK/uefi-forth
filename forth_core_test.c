#include "forth.h"
#include "forth_core.h"

static const uint64_t load_test_val = 0xDEADBEEF89ABCDEFULL;

void forth_test_core(void)
{
	// CALL
	COMPILE(L"CALL");

	// TRUE FALSE IF THEN
	COMPILE(L"TRUE");
	IF();
	COMPILE(L"FALSE");
	THEN();
	IF();
	COMMA(FORTH_BAD_PATTERN);
	THEN();

	// LIT TEST BEGIN UNTIL 1- DUP INVERT 0<> PZCHK
	BEGIN();
	COMPILE(L"TRUE");
	UNTIL();
	COMPILE(L"LIT");
	COMMA(2);
	BEGIN();
	COMPILE(L"1-", L"DUP", L"0<>");
	UNTIL();
	COMPILE(L"DROP", L"PZCHK");

	// EXECUTE =CHK
	COMPILE(L"LIT", L"TRUE", L"EXECUTE", L"TRUE", L"=CHK", L"PZCHK");

	// DBGOFF DBGON
	//COMPILE(L"DBGOFF", L"DBGON", L"DBGOFF", L"PZCHK");

	// SWAP
	COMPILE(L"$0", L"$1", L"SWAP", L"$0", L"=CHK", L"$1", L"=CHK",
		L"PZCHK");
	COMPILE(L"$A", L"$B", L"SWAP", L"$A", L"=CHK", L"$B", L"=CHK",
		L"PZCHK");

	// NIP
	COMPILE(L"$2", L"$3", L"NIP", L"$3", L"=CHK", L"PZCHK");
	COMPILE(L"$C", L"$D", L"NIP", L"$D", L"=CHK", L"PZCHK");

	// OVER
	COMPILE(L"$4", L"$5", L"OVER", L"$4", L"=CHK", L"$5", L"=CHK", L"$4",
		L"=CHK", L"PZCHK");
	COMPILE(L"$E", L"$F", L"OVER", L"$E", L"=CHK", L"$F", L"=CHK", L"$E",
		L"=CHK", L"PZCHK");

	// >R R>
	COMPILE(L"$0", L"$1", L">R", L"$0", L"=CHK", L"R>", L"$1", L"=CHK",
		L"PZCHK");
	COMPILE(L"$A", L"$B", L">R", L"$A", L"=CHK", L"R>", L"$B", L"=CHK",
		L"PZCHK");

	// PICK
	COMPILE(L"$1", L"$0", L"PICK", L"=CHK", L"PZCHK");
	COMPILE(L"$A", L"$0", L"PICK", L"=CHK", L"PZCHK");
	COMPILE(L"$B", L"$C", L"$1", L"PICK", L"$B", L"=CHK", L"$C", L"=CHK",
		L"$B", L"=CHK", L"PZCHK");
	COMPILE(L"$3", L"$4", L"$1", L"PICK", L"$3", L"=CHK", L"$4", L"=CHK",
		L"$3", L"=CHK", L"PZCHK");

	// DEPTH
	COMPILE(L"$1", L"DEPTH", L"=CHK", L"PZCHK");
	COMPILE(L"$1", L"$2", L"DEPTH", L"=CHK", L"$1", L"=CHK", L"PZCHK");

	// ROT
	COMPILE(L"$1", L"$2", L"$3", L"ROT", L"$1", L"=CHK", L"$3", L"=CHK",
		L"$2", L"=CHK", L"PZCHK");
	COMPILE(L"$A", L"$B", L"$C", L"ROT", L"$A", L"=CHK", L"$C", L"=CHK",
		L"$B", L"=CHK", L"PZCHK");

	// -ROT
	COMPILE(L"$1", L"$2", L"$3", L"-ROT", L"$2", L"=CHK", L"$1", L"=CHK",
		L"$3", L"=CHK", L"PZCHK");
	COMPILE(L"$A", L"$B", L"$C", L"-ROT", L"$B", L"=CHK", L"$A", L"=CHK",
		L"$C", L"=CHK", L"PZCHK");

	// 2DROP
	COMPILE(L"$1", L"$2", L"$3", L"2DROP", L"$1", L"=CHK", L"PZCHK");
	COMPILE(L"$A", L"$B", L"$C", L"2DROP", L"$A", L"=CHK", L"PZCHK");

	// 2DUP
	COMPILE(L"$1", L"$2", L"2DUP", L"$2", L"=CHK", L"$1", L"=CHK", L"$2",
		L"=CHK", L"$1", L"=CHK", L"PZCHK");
	COMPILE(L"$A", L"$B", L"2DUP", L"$B", L"=CHK", L"$A", L"=CHK", L"$B",
		L"=CHK", L"$A", L"=CHK", L"PZCHK");

	// 2SWAP
	COMPILE(L"$1", L"$2", L"$3", L"$4", L"2SWAP", L"$2", L"=CHK", L"$1",
		L"=CHK", L"$4", L"=CHK", L"$3", L"=CHK", L"PZCHK");
	COMPILE(L"$A", L"$B", L"$C", L"$D", L"2SWAP", L"$B", L"=CHK", L"$A",
		L"=CHK", L"$D", L"=CHK", L"$C", L"=CHK", L"PZCHK");

	// 2OVER
	COMPILE(L"$1", L"$2", L"$3", L"$4", L"2OVER", L"$2", L"=CHK", L"$1",
		L"=CHK", L"$4", L"=CHK", L"$3", L"=CHK", L"$2", L"=CHK", L"$1",
		L"=CHK", L"PZCHK");
	COMPILE(L"$A", L"$B", L"$C", L"$D", L"2OVER", L"$B", L"=CHK", L"$A",
		L"=CHK", L"$D", L"=CHK", L"$C", L"=CHK", L"$B", L"=CHK", L"$A",
		L"=CHK", L"PZCHK");

	// 2ROT
	COMPILE(L"$1", L"$2", L"$3", L"$4", L"$5", L"$6", L"2ROT");
	COMPILE(L"$2", L"=CHK", L"$1", L"=CHK");
	COMPILE(L"$6", L"=CHK", L"$5", L"=CHK");
	COMPILE(L"$4", L"=CHK", L"$3", L"=CHK", L"PZCHK");

	// 2-ROT
	COMPILE(L"$1", L"$2", L"$3", L"$4", L"$5", L"$6", L"2-ROT");
	COMPILE(L"$4", L"=CHK", L"$3", L"=CHK");
	COMPILE(L"$2", L"=CHK", L"$1", L"=CHK");
	COMPILE(L"$6", L"=CHK", L"$5", L"=CHK", L"PZCHK");

	// + 1+ - 1-
	COMPILE(L"$5", L"$6", L"+", L"DUP", L"$B", L"=CHK");
	COMPILE(L"1+", L"DUP", L"$C", L"=CHK");
	COMPILE(L"$3", L"-", L"DUP", L"$9", L"=CHK");
	COMPILE(L"1-", L"$8", L"=CHK", L"PZCHK");

	// * 2* / 2/
	COMPILE(L"$2", L"$4", L"*", L"DUP", L"$8", L"=CHK");
	COMPILE(L"2*", L"DUP", L"LIT");
	COMMA(16);
	COMPILE(L"=CHK");
	COMPILE(L"$2", L"/", L"DUP", L"$8", L"=CHK");
	COMPILE(L"2/", L"$4", L"=CHK", L"PZCHK");

	// LSHIFT RSHIFT
	COMPILE(L"$5", L"$1", L"RSHIFT", L"DUP", L"$2", L"=CHK");
	COMPILE(L"$2", L"LSHIFT", L"$8", L"=CHK", L"PZCHK");

	// AND
	COMPILE(L"$0", L"$0", L"AND", L"$0", L"=CHK");
	COMPILE(L"$1", L"$0", L"AND", L"$0", L"=CHK");
	COMPILE(L"$0", L"$1", L"AND", L"$0", L"=CHK");
	COMPILE(L"$1", L"$1", L"AND", L"$1", L"=CHK", L"PZCHK");

	// OR
	COMPILE(L"$0", L"$0", L"OR", L"$0", L"=CHK");
	COMPILE(L"$1", L"$0", L"OR", L"$1", L"=CHK");
	COMPILE(L"$0", L"$1", L"OR", L"$1", L"=CHK");
	COMPILE(L"$1", L"$1", L"OR", L"$1", L"=CHK", L"PZCHK");

	// XOR
	COMPILE(L"$0", L"$0", L"XOR", L"$0", L"=CHK");
	COMPILE(L"$1", L"$0", L"XOR", L"$1", L"=CHK");
	COMPILE(L"$0", L"$1", L"XOR", L"$1", L"=CHK");
	COMPILE(L"$1", L"$1", L"XOR", L"$0", L"=CHK", L"PZCHK");

	// NEGATE
	COMPILE(L"LIT");
	COMMA(0);
	COMPILE(L"NEGATE", L"LIT");
	COMMA(0);
	COMPILE(L"=CHK");
	COMPILE(L"LIT");
	COMMA(1);
	COMPILE(L"NEGATE", L"LIT");
	COMMA(-1);
	COMPILE(L"=CHK");
	COMPILE(L"LIT");
	COMMA(666);
	COMPILE(L"NEGATE", L"LIT");
	COMMA(-666);
	COMPILE(L"=CHK", L"PZCHK");

	// = <>
	COMPILE(L"$1", L"$2", L"=", L"FALSE", L"=CHK");
	COMPILE(L"$3", L"$3", L"=", L"TRUE", L"=CHK");
	COMPILE(L"$1", L"$2", L"<>", L"TRUE", L"=CHK");
	COMPILE(L"$3", L"$3", L"<>", L"FALSE", L"=CHK", L"PZCHK");

	// 0= 0<>
	COMPILE(L"$1", L"0=", L"FALSE", L"=CHK");
	COMPILE(L"$0", L"0=", L"TRUE", L"=CHK");
	COMPILE(L"$1", L"0<>", L"TRUE", L"=CHK");
	COMPILE(L"$0", L"0<>", L"FALSE", L"=CHK", L"PZCHK");

	// <
	COMPILE(L"$1", L"$2", L"<", L"TRUE", L"=CHK");
	COMPILE(L"$3", L"$3", L"<", L"FALSE", L"=CHK");
	COMPILE(L"$5", L"$4", L"<", L"FALSE", L"=CHK");
	LIT(-1);
	LIT(0);
	COMPILE(L"<", L"TRUE", L"=CHK");
	LIT(-1);
	LIT(1);
	COMPILE(L"<", L"TRUE", L"=CHK");
	LIT(-1);
	LIT(-2);
	COMPILE(L"<", L"FALSE", L"=CHK", L"PZCHK");

	// >
	COMPILE(L"$1", L"$2", L">", L"FALSE", L"=CHK");
	COMPILE(L"$3", L"$3", L">", L"FALSE", L"=CHK");
	COMPILE(L"$5", L"$4", L">", L"TRUE", L"=CHK");
	LIT(-1);
	LIT(0);
	COMPILE(L">", L"FALSE", L"=CHK");
	LIT(-1);
	LIT(1);
	COMPILE(L">", L"FALSE", L"=CHK");
	LIT(-1);
	LIT(-2);
	COMPILE(L">", L"TRUE", L"=CHK", L"PZCHK");

	// U<
	COMPILE(L"$1", L"$2", L"U<", L"TRUE", L"=CHK");
	COMPILE(L"$3", L"$3", L"U<", L"FALSE", L"=CHK");
	COMPILE(L"$5", L"$4", L"U<", L"FALSE", L"=CHK");
	LIT(-1);
	LIT(0);
	COMPILE(L"U<", L"FALSE", L"=CHK");
	LIT(-1);
	LIT(1);
	COMPILE(L"U<", L"FALSE", L"=CHK");
	LIT(-1);
	LIT(-2);
	COMPILE(L"U<", L"FALSE", L"=CHK", L"PZCHK");

	// U>
	COMPILE(L"$1", L"$2", L"U>", L"FALSE", L"=CHK");
	COMPILE(L"$3", L"$3", L"U>", L"FALSE", L"=CHK");
	COMPILE(L"$5", L"$4", L"U>", L"TRUE", L"=CHK");
	LIT(-1);
	LIT(0);
	COMPILE(L"U>", L"TRUE", L"=CHK");
	LIT(-1);
	LIT(1);
	COMPILE(L"U>", L"TRUE", L"=CHK");
	LIT(-1);
	LIT(-2);
	COMPILE(L"U>", L"TRUE", L"=CHK", L"PZCHK");

	// DP UP@ STA@
	COMPILE(L"DP", L"0=", L"FALSE", L"=CHK", L"PZCHK");
	COMPILE(L"UP@", L"0=", L"FALSE", L"=CHK", L"PZCHK");
	COMPILE(L"STA@", L"DROP", L"PZCHK");

	// C@ W@ L@ X@
	LIT(0xEF);
	LIT((uintptr_t)&load_test_val);
	COMPILE(L"C@", L"=CHK", L"PZCHK");
	LIT(0xCDEF);
	LIT((uintptr_t)&load_test_val);
	COMPILE(L"W@", L"=CHK", L"PZCHK");
	LIT(0x89ABCDEF);
	LIT((uintptr_t)&load_test_val);
	COMPILE(L"L@", L"=CHK", L"PZCHK");
	LIT(0xDEADBEEF89ABCDEFULL);
	LIT((uintptr_t)&load_test_val);
	COMPILE(L"X@", L"=CHK", L"PZCHK");

	// HERE PAD
	COMPILE(L"HERE", L"DP", L"X@", L"=CHK", L"PZCHK");
	COMPILE(L"HERE", L"PAD", L"<", L"TRUE", L"=CHK", L"PZCHK");

	// C! W! L! X!
	LIT(0x89);
	COMPILE(L"PAD", L"2DUP", L"C!", L"C@", L"=CHK", L"PZCHK");
	LIT(0xABCD);
	COMPILE(L"PAD", L"2DUP", L"W!", L"W@", L"=CHK", L"PZCHK");
	LIT(0xBAADB01A);
	COMPILE(L"PAD", L"2DUP", L"L!", L"L@", L"=CHK", L"PZCHK");
	LIT(0x89ABCDEF01234567ULL);
	COMPILE(L"PAD", L"2DUP", L"X!", L"X@", L"=CHK", L"PZCHK");

	// ALLOT
	COMPILE(L"HERE", L"$A", L"ALLOT", L"HERE", L"OVER", L"-", L"$A",
		L"=CHK");
	COMPILE(L"$A", L"NEGATE", L"ALLOT", L"HERE", L"=CHK", L"PZCHK");

	// C, W, L, X,
	COMPILE(L"HERE", L"$F", L"C,", L"DUP", L"C@", L"$F", L"=CHK", L"$1",
		L"NEGATE", L"ALLOT", L"HERE", L"=CHK");
	COMPILE(L"HERE", L"$E", L"W,", L"DUP", L"W@", L"$E", L"=CHK", L"$2",
		L"NEGATE", L"ALLOT", L"HERE", L"=CHK");
	COMPILE(L"HERE", L"$D", L"L,", L"DUP", L"L@", L"$D", L"=CHK", L"$4",
		L"NEGATE", L"ALLOT", L"HERE", L"=CHK");
	COMPILE(L"HERE", L"$C", L"X,", L"DUP", L"X@", L"$C", L"=CHK", L"$8",
		L"NEGATE", L"ALLOT", L"HERE", L"=CHK");

	// MOD
	COMPILE(L"$A", L"$6", L"MOD", L"$4", L"=CHK", L"PZCHK");

	// PSP-RST
	COMPILE(L"$5", L"$7", L"PSP-RST", L"PZCHK");
}
