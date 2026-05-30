#include "wstring.h"

int wstrlen(CHAR16 *ws)
{
	int len = 0;
	while (ws[0] != '\0') {
		len += 1;
		ws += 1;
	}
	return len;
}
int wstrbytes(CHAR16 *ws)
{
	return wstrlen(ws) * sizeof(CHAR16);
}

bool wstrsame(CHAR16 *aws, CHAR16 *bws)
{
	int lena, lenb;
	lena = wstrlen(aws);
	lenb = wstrlen(bws);
	if (lena != lenb) {
        return false;
	}
	while (lena > 0) {
		if (aws[0] != bws[0]) {
			return false;
		}
		aws += 1;
		bws += 1;
		lena -= 1;
	}
	return true;
}
