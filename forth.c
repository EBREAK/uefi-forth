#include "forth.h"
#include "efidef.h"
#include "forth_core.h"
#include "forth_io.h"
#include "forth_interpret.h"
#include "forth_test.h"
#include "fifo.h"
#include "main.h"
#include "debug.h"
#include "wstring.h"

#include <stdarg.h>

struct forth_context *forth_task_root = NULL;
uint64_t forth_task_nums = 0;

uint8_t *FORTH_RAM = NULL;
volatile uintptr_t FORTH_DP = 0;
volatile uintptr_t FORTH_SELFTEST;
volatile struct forth_word *FORTH_LATEST = NULL;

struct forth_word *forth_word_new(CHAR16 *name, uintptr_t opcode)
{
	struct forth_word *word;
	word = FORTH_DP;
	FORTH_DP += sizeof(struct forth_word);
	GST->BootServices->SetMem(word, sizeof(struct forth_word), 0);
	word->prev = FORTH_LATEST;
	word->name = FORTH_DP;
	word->name[0] = '\0';
	FORTH_DP += sizeof(CHAR16);
	if (name != NULL) {
		GST->BootServices->CopyMem(word->name, name,
					   wstrbytes(name) + sizeof(CHAR16));
		FORTH_DP += wstrbytes(word->name);
	}
	word->desc = NULL;
	word->opcode = opcode;
	word->flags = 0;
	word->blen = 0;
	word->body = FORTH_DP;
	return word;
}

void forth_ppush(struct forth_context *fctx, uintptr_t val)
{
	fctx->psp -= sizeof(uintptr_t);
	if (fctx->psp < (fctx->ps0 - (FORTH_STACK_DEPTH * sizeof(uintptr_t)))) {
		fctx->sta |= FORTH_STA_PSER;
		return;
	}
	*(uintptr_t *)fctx->psp = val;
}

uintptr_t forth_ppop(struct forth_context *fctx)
{
	uintptr_t ret;
	if (fctx->psp > (fctx->ps0 - sizeof(uintptr_t))) {
		fctx->sta |= FORTH_STA_PSER;
		return FORTH_BAD_PATTERN;
	}
	ret = *(uintptr_t *)fctx->psp;
	fctx->psp += sizeof(uintptr_t);
	return ret;
}

void forth_rpush(struct forth_context *fctx, uintptr_t val)
{
	fctx->rsp -= sizeof(uintptr_t);
	if (fctx->rsp < (fctx->rs0 - (FORTH_STACK_DEPTH * sizeof(uintptr_t)))) {
		fctx->sta |= FORTH_STA_RSER;
		return;
	}
	*(uintptr_t *)fctx->rsp = val;
}

uintptr_t forth_rpop(struct forth_context *fctx)
{
	uintptr_t ret;
	if (fctx->rsp > (fctx->rs0 - sizeof(uintptr_t))) {
		fctx->sta |= FORTH_STA_RSER;
		return FORTH_BAD_PATTERN;
	}
	ret = *(uintptr_t *)fctx->rsp;
	fctx->rsp += sizeof(uintptr_t);
	return ret;
}

uintptr_t forth_cstk[FORTH_STACK_DEPTH];
uintptr_t forth_csp = &forth_cstk[FORTH_STACK_DEPTH];
uintptr_t forth_cs0 = &forth_cstk[FORTH_STACK_DEPTH];

void forth_csp_rst(void)
{
	forth_csp = forth_cs0;
}

void forth_cpush(uintptr_t val)
{
	forth_csp -= sizeof(uintptr_t);
	if (forth_csp < (forth_cs0 - (FORTH_STACK_DEPTH * sizeof(uintptr_t)))) {
		debug_putws(L"\r\n!!! FORTH COMPILE STACK OVERFLOW !!!\r\n");
		return;
	}
	*(uintptr_t *)forth_csp = val;
}

uintptr_t forth_cpop(void)
{
	uintptr_t ret;
	if (forth_csp > (forth_cs0 - sizeof(uintptr_t))) {
		debug_putws(L"\r\n!!! FORTH COMPILE STACK UNDERFLOW !!!\r\n");
		return FORTH_BAD_PATTERN;
	}
	ret = *(uintptr_t *)forth_csp;
	forth_csp += sizeof(uintptr_t);
	return ret;
}

void forth_coloncode(CHAR16 *name, uintptr_t opcode)
{
	struct forth_word *word;
	word = forth_word_new(name, opcode);
	forth_cpush((uintptr_t)word);
}

void forth_colon(CHAR16 *name)
{
	return forth_coloncode(name, O_CALL);
}

void forth_desc(CHAR16 *str) {
    struct forth_word *word;
    if (wstrlen(str) == 0) { return; }
    word = (struct forth_word *)forth_cpop();
    forth_cpush((uintptr_t)word);
    if (word->blen != 0) {
        debug_putws(L"WORD ");
        debug_putws(word->name);
        debug_putws(L" NO SPACE FOR DESC");
        return;
    }
    word->desc = FORTH_DP;
    FORTH_DP += wstrbytes(str) + sizeof(CHAR16);
    if (word->body == (uintptr_t)word->desc) {
        word->body = FORTH_DP;
    }
    GST->BootServices->CopyMem(word->desc, str, wstrbytes(str) + sizeof(CHAR16));
}

void forth_semicolon(void)
{
	struct forth_word *word;
	word = forth_cpop();
	word->blen = FORTH_DP - word->body;
	FORTH_LATEST = word;
}

void forth_comma(uintptr_t val)
{
	*(uintptr_t *)FORTH_DP = val;
	FORTH_DP += sizeof(uintptr_t);
}

void forth_lit(uintptr_t val)
{
    forth_find_comma(L"LIT");
    forth_comma(val);
}

void forth_words(void)
{
	struct forth_word *word;
	word = FORTH_LATEST;
	do {
		debug_putws(word->name);
		debug_putwc(L' ');
		word = word->prev;
	} while (word != NULL);
	debug_cr();
}

struct forth_word *forth_find(CHAR16 *name)
{
	struct forth_word *word;
	word = FORTH_LATEST;
	do {
		if (wstrlen(name) == wstrlen(word->name)) {
			if (wstrsame(name, word->name) == true) {
				return word;
			}
		}
		word = word->prev;
	} while (word != NULL);
	return word;
}

void forth_find_comma(CHAR16 *name)
{
	struct forth_word *word;
	word = forth_find(name);
	if (word == NULL) {
		debug_putws(L"\r\n!!! WORD ");
		debug_putws(name);
		debug_putws(L" NOT FOUND !!!\r\n");
	}
	forth_comma(word);
}

void forth_find_comma_multi(CHAR16 *name0, ...)
{
	va_list ap;
	CHAR16 *name;
	va_start(ap, name0);
	forth_find_comma(name0);
	name = va_arg(ap, CHAR16 *);
	while (name != NULL) {
		forth_find_comma(name);
		name = va_arg(ap, CHAR16 *);
	}
	va_end(ap);
}

void forth_constant(CHAR16 *name, uintptr_t val)
{
    forth_coloncode(name, O_DOCONST);
    forth_comma(val);
}

void forth_if(void) {
    forth_find_comma(L"ZBRANCH");
    forth_cpush(FORTH_DP);
    forth_comma(0);
}

void forth_then(void) {
    uintptr_t a;
    a = forth_cpop();
    *(uintptr_t *)a = FORTH_DP;
}

void forth_begin(void) {
    forth_cpush(FORTH_DP);
}

void forth_again(void) {
    forth_find_comma(L"BRANCH");
    uintptr_t a;
    a = forth_cpop();
    forth_comma(a);
}

void forth_until(void) {
    forth_find_comma(L"ZBRANCH");
    uintptr_t a;
    a = forth_cpop();
    forth_comma(a);
}

void forth_immediate(void) {
    FORTH_LATEST->flags |= FORTH_WFLG_IMMED;
}

struct forth_context *forth_task_new(struct forth_context *parent, uintptr_t ip)
{
	struct forth_context *fctx = NULL;
	EFI_STATUS Status;
	UINTN alloc_size = sizeof(struct forth_context);
	alloc_size += FORTH_STACK_DEPTH * sizeof(uintptr_t); // PARAM STACK
	alloc_size += FORTH_STACK_DEPTH * sizeof(uintptr_t); // RETURN STACK
	uint8_t *p = NULL;
	Status = GST->BootServices->AllocatePool(EfiRuntimeServicesData,
						 alloc_size, (void *)&p);
	if (EFI_ERROR(Status)) {
		return NULL;
	}
	GST->BootServices->SetMem(p, alloc_size, 0);
	fctx = (struct forth_context *)p;
	p += sizeof(struct forth_context);
	fctx->xt_wemit = forth_find(L"EARLY-WEMIT");
	fctx->xt_wkey = forth_find(L"EARLY-WKEY");
	fctx->xt_dot = forth_find(L"XHEX.");
	if (parent != NULL) {
		GST->BootServices->CopyMem(fctx, parent,
					   sizeof(struct forth_context));
	}
	fctx->ip = ip;
	fctx->tos = FORTH_TOS_INIT;
	fctx->sta = 0;
	//fctx->sta |= FORTH_STA_DUMP;
	fctx->wait_state = FORTH_WAIT_NOOP;
	fctx->save = NULL;
	p += FORTH_STACK_DEPTH * sizeof(uintptr_t); // PARAM STACK TOP
	fctx->ps0 = p;
	fctx->psp = fctx->ps0;
	p += FORTH_STACK_DEPTH * sizeof(uintptr_t); // RETURN STACK TOP
	fctx->rs0 = p;
	fctx->rsp = fctx->rs0;
	fctx->win = 0;
	fctx->next = fctx;
	if (parent != NULL) {
		parent->next = fctx;
		fctx->next = parent->next;
	}
	forth_task_nums += 1;
	return fctx;
}

void forth_init_task(void)
{
	forth_task_root = forth_task_new(NULL, FORTH_SELFTEST);
	if (forth_task_root == NULL) {
		debug_putws(L"FORTH TASK ROOT INIT FAIL\r\n");
		GST->BootServices->Exit(GIH, EFI_OUT_OF_RESOURCES, NULL, 0);
	}
	forth_task_root->wib = FORTH_DP;
	FORTH_DP += FORTH_NAME_MAXLEN + 1;
}

void forth_init(void)
{
	EFI_STATUS Status;
	Status = GST->BootServices->AllocatePool(
		EfiRuntimeServicesData, FORTH_RAM_SIZE, (void *)&FORTH_RAM);
	if (EFI_ERROR(Status)) {
		debug_putws(L"FORTH RAM ALLOT FAIL\r\n");
		GST->BootServices->Exit(GIH, Status, NULL, 0);
	}
	FORTH_DP = FORTH_RAM;
	forth_init_core();
	forth_init_io();
	forth_init_interpret();
	forth_init_test();
	//forth_words();
	forth_init_task();
}

void forth_dump_ctx(struct forth_context *fctx)
{
	debug_putws(L"FORTH DUMP:");
	debug_putws(L" IP ");
	debug_puthex(fctx->ip);
	debug_putws(L" W ");
	debug_puthex(fctx->w);
	debug_putws(L" TOS ");
	debug_puthex(fctx->tos);
	debug_putws(L" PSP ");
	debug_puthex(fctx->psp);
	debug_putws(L" RSP ");
	debug_puthex(fctx->rsp);
	debug_putws(L" STA ");
	debug_puthex(fctx->sta);
	debug_cr();
}

void forth_dump_word(struct forth_word *word)
{
	debug_putws(L"FORTH WORD:");
	debug_putws(L" NAME: ");
	debug_putws(word->name);
	debug_putws(L" OPCODE: ");
	debug_puthex(word->opcode);
	if (word->desc != NULL) {
    	debug_putws(L" DESC: ");
        debug_putws(word->desc);
	}
	debug_cr();
}

void forth_o_find(struct forth_context *fctx) {
    fctx->w = fctx->tos;
    fctx->x = forth_ppop(fctx);
    if (fctx->w > FORTH_NAME_MAXLEN) {
        fctx->tos = 0;
        return;
    }
    CHAR16 name[FORTH_NAME_MAXLEN + 1];
    GST->BootServices->SetMem(name, ((FORTH_NAME_MAXLEN + 1) * sizeof(CHAR16)), 0);
    GST->BootServices->CopyMem(name, fctx->x, (fctx->w * sizeof(CHAR16)));
    fctx->tos = forth_find(name);
}

void  forth_o_isimmediate(struct forth_context *fctx) {
    struct forth_word *word;
    word = (struct forth_word *)fctx->tos;
    fctx->tos = FORTH_FALSE;
    if ((word->flags & FORTH_WFLG_IMMED) != 0) {
        fctx->tos = FORTH_TRUE;
    }
}

void forth_o_word_new(struct forth_context *fctx) {
    fctx->w = fctx->tos; // count
    fctx->x = forth_ppop(fctx); // waddr
    if (fctx->w > FORTH_NAME_MAXLEN) {
        fctx->tos = 0;
        return;
    }
    CHAR16 name[FORTH_NAME_MAXLEN + 1];
    GST->BootServices->SetMem(name, ((FORTH_NAME_MAXLEN + 1) * sizeof(CHAR16)), 0);
    GST->BootServices->CopyMem(name, fctx->x, (fctx->w * sizeof(CHAR16)));
    fctx->tos = forth_word_new(name, O_CALL);
}

bool wisxdigit(CHAR16 c) {
    if (c < '0') { return false; }
    if (c > 'F') { return false; }
    if (c <= '9') { return true; }
    if (c >= 'A') { return true; }
    return false;
}

void forth_o_isnumber(struct forth_context *fctx) {
    fctx->w = fctx->tos; // count
    fctx->x = forth_ppop(fctx); // waddr
    fctx->tos = FORTH_FALSE;
    if (fctx->w < 2) {
        return;
    }
    CHAR16 *wp;
    wp = fctx->x;
    switch (wp[0]) {
    case L'$':
        break;
    default:
        return;
    }
    wp += 1; fctx->w -= 1;
    while(fctx->w > 0) {
        if (wisxdigit(wp[0]) == false) {
            return;
        }
        wp += 1;
        fctx->w -= 1;
    }
    fctx->tos = FORTH_TRUE;
}

int whex2num(CHAR16 wc) {
    if (wc <= '9') { return wc - '0'; }
    wc -= 'A';
    wc += 0xA;
    return wc;
}

void forth_o_tonumber(struct forth_context *fctx) {
    fctx->w = fctx->tos; // count
    fctx->x = forth_ppop(fctx); // waddr
    fctx->tos = 0;
    if (fctx->w < 2) {
        return;
    }
    CHAR16 *wp;
    wp = fctx->x;
    switch (wp[0]) {
    case L'$':
        break;
    default:
        return;
    }
    wp += 1; fctx->w -= 1;
    while(fctx->w > 0) {
        fctx->tos |= whex2num(*wp) << ((fctx->w - 1) * 4);
        wp += 1;
        fctx->w -= 1;
    }
}

void forth_o_xt2wname(struct forth_context *fctx) {
    struct forth_word *word;
    word = (struct forth_word *)fctx->tos;
    fctx->tos = (uintptr_t)word->name;
}

void forth_o_xt2wnlen(struct forth_context *fctx) {
    struct forth_word *word;
    word = (struct forth_word *)fctx->tos;
    fctx->tos = (uintptr_t)wstrlen(word->name);
}

void forth_o_xt2prev(struct forth_context *fctx) {
    struct forth_word *word;
    word = (struct forth_word *)fctx->tos;
    fctx->tos = (uintptr_t)word->prev;
}

void forth_run(struct forth_context *fctx)
{
	if (fctx == NULL) {
		return;
	}
	if ((fctx->sta & FORTH_STA_HALT) != 0) {
		return;
	}
	if (fctx->save != NULL) {
	    goto * fctx->save;
	}
forth_next:
	fctx->w = *(uintptr_t *)fctx->ip;
	fctx->ip += sizeof(uintptr_t);
forth_exec:
	struct forth_word *word;
	word = fctx->w;
	if ((fctx->sta & FORTH_STA_DUMP) != 0) {
		forth_dump_ctx(fctx);
		forth_dump_word(word);
	}
	switch (word->opcode) {
	case O_NOOP:
		break;
	case O_HALT:
		fctx->sta |= FORTH_STA_HALT;
		forth_dump_ctx(fctx);
        forth_dump_word(word);
		debug_putws(L"FORTH: HALT\r\n");
		return;
	case O_BRANCH:
		fctx->ip = *(uintptr_t *)fctx->ip;
		break;
	case O_ZBRANCH:
		fctx->w = fctx->tos;
		fctx->tos = forth_ppop(fctx);
		fctx->ip += sizeof(uintptr_t);
		if (fctx->w == 0) {
			fctx->ip = *(uintptr_t *)(fctx->ip - sizeof(uintptr_t));
		}
		break;
	case O_EXIT:
	    fctx->ip = forth_rpop(fctx);
	    break;
    case O_CALL:
    	forth_rpush(fctx, fctx->ip);
        fctx->ip = word->body;
		break;
	case O_LIT:
    	forth_ppush(fctx, fctx->tos);
	    fctx->tos = *(uintptr_t *)fctx->ip;
		fctx->ip += sizeof(uintptr_t);
	    break;
	case O_DOCONST:
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = *(uintptr_t *)word->body;
	    break;
	case O_DUP:
	    forth_ppush(fctx, fctx->tos);
	    break;
	case O_DROP:
	    fctx->tos = forth_ppop(fctx);
	    break;
	case O_PZCHK:
	    if ((fctx->psp != fctx->ps0) ||
    		(fctx->tos != FORTH_TOS_INIT)) {
    		fctx->sta |= FORTH_STA_HALT;
            forth_dump_ctx(fctx);
            forth_dump_word(word);
    		debug_putws(L"PZCHK FAIL\r\n");
    		return;
    	}
	    break;
	case O_EXECUTE:
	    fctx->w = fctx->tos;
		fctx->tos = forth_ppop(fctx);
	    goto forth_exec;
	case O_EQCHK:
	    fctx->w = forth_ppop(fctx);
		if (fctx->tos != fctx->w) {
    		fctx->sta |= FORTH_STA_HALT;
            forth_dump_ctx(fctx);
            forth_dump_word(word);
		    debug_putws(L"=CHK FAIL ");
			debug_puthex(fctx->w);
			debug_putws(L" ");
			debug_puthex(fctx->tos);
			debug_cr();
			return;
		}
		fctx->tos = forth_ppop(fctx);
	    break;
	case O_DBGOFF:
	    fctx->sta &= ~(FORTH_STA_DUMP);
	    break;
	case O_DBGON:
        fctx->sta |= FORTH_STA_DUMP;
	    break;
	case O_SWAP:
	    fctx->w = forth_ppop(fctx);
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = fctx->w;
	    break;
	case O_NIP:
	    fctx->w = forth_ppop(fctx);
	    break;
	case O_OVER:
	    fctx->w = forth_ppop(fctx);
	    forth_ppush(fctx, fctx->w);
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = fctx->w;
	    break;
	case O_TOR:
	    forth_rpush(fctx, fctx->tos);
		fctx->tos = forth_ppop(fctx);
	    break;
	case O_FROMR:
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = forth_rpop(fctx);
	    break;
	case O_PICK:
	    if (((intptr_t)fctx->tos) < 0) {
			debug_putws(L"FORTH STACK UNDERFLOW\r\n");
			fctx->sta |= FORTH_STA_PSER;
			fctx->tos = FORTH_BAD_PATTERN;
			break;
		}
	    if (((intptr_t)fctx->tos) > FORTH_STACK_DEPTH) {
			debug_putws(L"FORTH STACK OVERFLOW\r\n");
			fctx->sta |= FORTH_STA_PSER;
			fctx->tos = FORTH_BAD_PATTERN;
			break;
		}
		fctx->tos = *(uintptr_t *)(fctx->psp + (fctx->tos * sizeof(uintptr_t)));
	    break;
	case O_DEPTH:
	    fctx->w = (fctx->ps0 - fctx->psp) / sizeof(uintptr_t);
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = fctx->w;
	    break;
	case O_PLUS:
	    fctx->w = forth_ppop(fctx);
		fctx->tos = fctx->w + fctx->tos;
	    break;
	case O_1PLUS:
    	fctx->tos += 1;
        break;
    case O_MINUS:
	    fctx->w = forth_ppop(fctx);
		fctx->tos = fctx->w - fctx->tos;
	    break;
    case O_1MINUS:
	    fctx->tos -= 1;
		break;
	case O_MULTI:
        fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w * fctx->tos;
	    break;
	case O_2MULTI:
	    fctx->tos *= 2;
		break;
	case O_DIVID:
        fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w / fctx->tos;
	    break;
	case O_2DIVID:
	    fctx->tos /= 2;
		break;
	case O_LSHIFT:
    	fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w << fctx->tos;
        break;
    case O_RSHIFT:
       	fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w >> fctx->tos;
        break;
    case O_AND:
       	fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w & fctx->tos;
        break;
    case O_OR:
       	fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w | fctx->tos;
        break;
    case O_XOR:
       	fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w ^ fctx->tos;
        break;
	case O_INVERT:
	    fctx->tos ^= -1;
	    break;
	case O_NEGATE:
	    fctx->tos = - (fctx->tos);
	    break;
	case O_MOD:
    	fctx->w = forth_ppop(fctx);
        fctx->tos = fctx->w % fctx->tos;
	    break;
	case O_EQ:
	    fctx->w = forth_ppop(fctx);
		fctx->x = fctx->tos;
		fctx->tos = FORTH_FALSE;
		if (fctx->w == fctx->x) {
		    fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_NE:
	    fctx->w = forth_ppop(fctx);
		fctx->x = fctx->tos;
		fctx->tos = FORTH_FALSE;
		if (fctx->w != fctx->x) {
		    fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_EQZ:
	    fctx->w = fctx->tos;
		fctx->tos = FORTH_FALSE;
	    if (fctx->w == 0) {
			fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_NEZ:
	    fctx->w = fctx->tos;
		fctx->tos = FORTH_FALSE;
	    if (fctx->w != 0) {
			fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_LT:
	    fctx->w = forth_ppop(fctx);
		fctx->x = fctx->tos;
		fctx->tos = FORTH_FALSE;
		if (((intptr_t)fctx->w) < ((intptr_t)fctx->x)) {
		    fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_GT:
	    fctx->w = forth_ppop(fctx);
		fctx->x = fctx->tos;
		fctx->tos = FORTH_FALSE;
		if (((intptr_t)fctx->w) > ((intptr_t)fctx->x)) {
		    fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_ULT:
	    fctx->w = forth_ppop(fctx);
		fctx->x = fctx->tos;
		fctx->tos = FORTH_FALSE;
		if (((uintptr_t)fctx->w) < ((uintptr_t)fctx->x)) {
		    fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_UGT:
	    fctx->w = forth_ppop(fctx);
		fctx->x = fctx->tos;
		fctx->tos = FORTH_FALSE;
		if (((uintptr_t)fctx->w) > ((uintptr_t)fctx->x)) {
		    fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_DP:
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = (uintptr_t)&FORTH_DP;
	    break;
	case O_UPLOAD:
        forth_ppush(fctx, fctx->tos);
        fctx->tos = (uintptr_t)fctx;
	    break;
	case O_STALOAD:
    	forth_ppush(fctx, fctx->tos);
        fctx->tos = fctx->sta;
	    break;
	case O_CLOAD:
	    fctx->w = *(uint8_t *)fctx->tos;
		fctx->tos = fctx->w;
	    break;
	case O_WLOAD:
	    fctx->w = *(uint16_t *)fctx->tos;
		fctx->tos = fctx->w;
	    break;
	case O_LLOAD:
	    fctx->w = *(uint32_t *)fctx->tos;
		fctx->tos = fctx->w;
	    break;
	case O_XLOAD:
	    fctx->w = *(uint64_t *)fctx->tos;
		fctx->tos = fctx->w;
	    break;
	case O_CSTORE:
        fctx->w = forth_ppop(fctx);
        *(uint8_t *)fctx->tos = fctx->w;
        fctx->tos = forth_ppop(fctx);
	    break;
	case O_WSTORE:
        fctx->w = forth_ppop(fctx);
        *(uint16_t *)fctx->tos = fctx->w;
        fctx->tos = forth_ppop(fctx);
	    break;
	case O_LSTORE:
        fctx->w = forth_ppop(fctx);
        *(uint32_t *)fctx->tos = fctx->w;
        fctx->tos = forth_ppop(fctx);
	    break;
	case O_XSTORE:
        fctx->w = forth_ppop(fctx);
        *(uint64_t *)fctx->tos = fctx->w;
        fctx->tos = forth_ppop(fctx);
	    break;
	case O_PSP_RST:
	    fctx->psp = fctx->ps0;
		fctx->tos = FORTH_TOS_INIT;
		fctx->sta &= ~(FORTH_STA_PSER);
	    break;
	case O_EARLY_WEMIT:
    	fctx->w = fctx->tos;
	    fctx->tos = forth_ppop(fctx);
		debug_putwc(fctx->w);
	    break;
	case O_EARLY_WKEY:
forth_wait_early_wkey:
	    if (fifo16_is_empty(early_wkey_fifo) == true) {
			fctx->wait_state = FORTH_WAIT_EARLY_WKEY;
			fctx->save = &&forth_wait_early_wkey;
			return;
		}
		fctx->wait_state = FORTH_WAIT_NOOP;
		fctx->save = NULL;
		forth_ppush(fctx, fctx->tos);
		fctx->tos = 0;
		fifo16_pop(early_wkey_fifo, (uint16_t *)&fctx->tos);
	    break;
	case O_USER_WEMIT:
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = &fctx->xt_wemit;
	    break;
	case O_USER_WKEY:
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = &fctx->xt_wkey;
	    break;
	case O_USER_DOT:
        forth_ppush(fctx, fctx->tos);
        fctx->tos = &fctx->xt_dot;
	    break;
	case O_FIND:
	    forth_o_find(fctx);
	    break;
	case O_COMPON:
	    fctx->sta |= FORTH_STA_COMP;
	    break;
	case O_COMPOFF:
	    fctx->sta &= ~(FORTH_STA_COMP);
	    break;
	case O_COMPSTA:
	    forth_ppush(fctx, fctx->tos);
	    fctx->tos = FORTH_FALSE;
		if ((fctx->sta & FORTH_STA_COMP) != 0) {
		    fctx->tos = FORTH_TRUE;
		}
	    break;
	case O_ISIMMEDIATE:
	    forth_o_isimmediate(fctx);
	    break;
	case O_WIB:
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = fctx->wib;
		break;
	case O_WIN:
	    forth_ppush(fctx, fctx->tos);
		fctx->tos = (uintptr_t)&fctx->win;
		break;
	case O_ISNUMBER:
	    forth_o_isnumber(fctx);
	    break;
	case O_TONUMBER:
        forth_o_tonumber(fctx);
        break;
	case O_WORD_NEW:
	    forth_o_word_new(fctx);
	    break;
	case O_XT2WNAME:
	    forth_o_xt2wname(fctx);
	    break;
	case O_XT2WNLEN:
	    forth_o_xt2wnlen(fctx);
        break;
    case O_XT2PREV:
        forth_o_xt2prev(fctx);
        break;
    case O_EARLY_ECHO_OFF:
        early_echo = false;
        break;
    case O_EARLY_ECHO_ON:
        early_echo = true;
        break;
	default:
		fctx->sta |= FORTH_STA_HALT;
		forth_dump_ctx(fctx);
        forth_dump_word(word);
		debug_putws(L"FORTH: INVALID OPCODE\r\n");
		return;
	}
	goto forth_next;
}

void forth_run_all(void)
{
	if (forth_task_root == NULL) {
		return;
	}
	uint64_t idx = 0;
	struct forth_context *fctx;
	fctx = forth_task_root;
	while (idx < forth_task_nums) {
		forth_run(fctx);
		fctx = fctx->next;
		idx += 1;
	}
}
