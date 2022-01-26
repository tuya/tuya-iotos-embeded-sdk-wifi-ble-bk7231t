#include "new_common.h"


// returns amount of space left in buffer (0=overflow happened)
int strcat_safe(char *tg, const char *src, int tgMaxLen) {
	// keep space for 1 more char
	int curOfs = 1;

	// skip
	while(*tg != 0) {
		tg++;
		curOfs++;
		if(curOfs >= tgMaxLen) {
			*tg = 0;
			return 0;
		}
	}
	// copy
	while(*src != 0) {
		*tg = *src;
		src++;
		tg++;
		curOfs++;
		if(curOfs >= tgMaxLen) {
			*tg = 0;
			return 0;
		}
	}
	*tg = 0;
	return tgMaxLen-curOfs;
}

