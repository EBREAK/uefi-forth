#pragma once

#include <efi.h>
#include <stdbool.h>

extern int wstrlen(CHAR16 *ws);
extern int wstrbytes(CHAR16 *ws);
extern bool wstrsame(CHAR16 *aws, CHAR16 *bws);
