#pragma once

#include <efi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FORTH_RAM_SIZE (8 * 1024 * 1024)
#define FORTH_NAME_MAXLEN 127
#define FORTH_STACK_DEPTH 128
#define FORTH_TOS_INIT 0xDEADBEEFULL
#define FORTH_BAD_PATTERN 0xA50FDEADULL
#define FORTH_TRUE -1
#define FORTH_FALSE 0

struct forth_word {
	struct forth_word *prev;
	CHAR16 *name;
	CHAR16 *desc;
	uintptr_t flags;
	uintptr_t opcode;
	uintptr_t blen;
	uintptr_t body;
};

enum {
	FORTH_WFLG_IMMED = (1 << 0),
};

extern void forth_coloncode(CHAR16 *name, uintptr_t opcode);
extern void forth_colon(CHAR16 *name);
extern void forth_desc(CHAR16 *str);
extern void forth_semicolon(void);
extern struct forth_word *forth_find(CHAR16 *name);
extern void forth_comma(uintptr_t val);
extern void forth_lit(uintptr_t val);
extern void forth_find_comma(CHAR16 *name);
extern void forth_find_comma_multi(CHAR16 *name0, ...);
extern void forth_constant(CHAR16 *name, uintptr_t val);
extern void forth_begin(void);
extern void forth_again(void);
extern void forth_until(void);
extern void forth_if(void);
extern void forth_then(void);
extern void forth_immediate(void);
extern void forth_dump_word(struct forth_word *word);

#define DFWL(name, opcode) forth_coloncode((name), (opcode))
#define DFWH(name) forth_colon((name))
#define DESC(str) forth_desc(str)
#define ENDW() forth_semicolon()
#define COMMA(x) forth_comma((x))
#define LIT(x) forth_lit((x))
#define COMPILE(...) forth_find_comma_multi(__VA_ARGS__, NULL)
#define CONSTANT(name, val) forth_constant((name), (val))
#define IF() forth_if()
#define THEN() forth_then()
#define BEGIN() forth_begin()
#define AGAIN() forth_again()
#define UNTIL() forth_until()
#define IMMEDIATE() forth_immediate()

extern volatile uintptr_t FORTH_DP;
extern volatile uintptr_t FORTH_SELFTEST;
extern volatile struct forth_word *FORTH_LATEST;

struct forth_context {
	struct forth_context *next;
	uintptr_t w;
	uintptr_t x;
	uintptr_t ip;
	uintptr_t tos;
	uintptr_t ps0;
	uintptr_t psp;
	uintptr_t rs0;
	uintptr_t rsp;
	uintptr_t sta;

	uintptr_t wib;
	uintptr_t win;
	uintptr_t gop_sel;

	uintptr_t xt_wemit;
	uintptr_t xt_wkey;
	uintptr_t xt_dot;

	int wait_state;
	void *save;
	bool delayus_done;
};

enum {
	FORTH_STA_HALT = (1 << 0),
	FORTH_STA_PSER = (1 << 1),
	FORTH_STA_RSER = (1 << 2),
	FORTH_STA_CSER = (1 << 3),
	FORTH_STA_DUMP = (1 << 4),
	FORTH_STA_COMP = (1 << 5),
};

enum {
	FORTH_WAIT_NOOP = 0,
	FORTH_WAIT_EARLY_WKEY,
	FORTH_WAIT_DELAYUS,
};

extern uintptr_t forth_ppop(struct forth_context *fctx);
extern void forth_ppush(struct forth_context *fctx, uintptr_t val);

extern void forth_init(void);
extern void forth_run(struct forth_context *fctx);
extern void forth_run_all(void);

enum {
	// CORE WORDS
	O_NOOP,
	O_HALT,
	O_BRANCH,
	O_ZBRANCH,
	O_EXIT,
	O_CALL,
	O_LIT,
	O_DOCONST,
	O_EXECUTE,
	O_PZCHK,
	O_EQCHK,
	O_DBGOFF,
	O_DBGON,
	O_DROP,
	O_DUP,
	O_SWAP,
	O_NIP,
	O_OVER,
	O_TOR,
	O_FROMR,
	O_PICK,
	O_DEPTH,
	O_PLUS,
	O_MINUS,
	O_1PLUS,
	O_1MINUS,
	O_MULTI,
	O_2MULTI,
	O_DIVID,
	O_2DIVID,
	O_LSHIFT,
	O_RSHIFT,
	O_AND,
	O_OR,
	O_XOR,
	O_INVERT,
	O_NEGATE,
	O_MOD,
	O_EQ,
	O_NE,
	O_EQZ,
	O_NEZ,
	O_LT,
	O_GT,
	O_ULT,
	O_UGT,
	O_DP,
	O_UPLOAD,
	O_STALOAD,
	O_CLOAD,
	O_WLOAD,
	O_LLOAD,
	O_XLOAD,
	O_CSTORE,
	O_WSTORE,
	O_LSTORE,
	O_XSTORE,
	O_PSP_RST,
	O_EARLY_ECHO_OFF,
	O_EARLY_ECHO_ON,
	O_TASK_NEW,
	O_CMOVE,
	O_WMOVE,
	O_LMOVE,
	O_XMOVE,
	O_CFILL,
	O_WFILL,
	O_LFILL,
	O_XFILL,
	O_ALLOCATE,
	O_FREE,
	O_EARLY_CLR,

	// IO WORDS
	O_EARLY_WEMIT,
	O_EARLY_WKEY,
	O_USER_WEMIT,
	O_USER_WKEY,
	O_USER_DOT,
	O_DELAYUS,

	// INTERPRET WORDS
	O_FIND,
	O_COMPOFF,
	O_COMPON,
	O_COMPSTA,
	O_ISIMMEDIATE,
	O_WIB,
	O_WIN,
	O_ISNUMBER,
	O_TONUMBER,
	O_WORD_NEW,
	O_XT2WNAME,
	O_XT2WNLEN,
	O_XT2PREV,
	O_XT2BODY,
	O_XT2WDESC,
	O_XT2WDLEN,
	O_BLEN_LOAD,
	O_BLEN_STORE,

	// UEFI GOP WORDS
	O_GOP_INIT,
	O_GOP_CNT,
	O_GOP_SEL,
	O_GOP_CUR,
	O_GOP_MODE_CNT,
	O_GOP_MODE_SEL,
	O_GOP_MODE_CUR,
	O_GOP_HRES,
	O_GOP_VRES,
	O_GOP_PLOT,
	O_GOP_SOLID,
	O_GOP_HLINE,
	O_GOP_VLINE,
	O_GOP_FRAME,
	O_GOP_TOPIXEL,
	O_GOP_TOCOLOR,
};
