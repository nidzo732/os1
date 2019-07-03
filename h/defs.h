#ifndef HEADERS_DEFS_H_
#define HEADERS_DEFS_H_

#define safe(K){\
	asm pushf;\
	asm cli;\
	K;\
	asm popf;\
}
#endif /* HEADERS_DEFS_H_ */
