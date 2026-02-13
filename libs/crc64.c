#include "WinHash.h"

static UINT64 crc64_tab[256];
static int crc64_tab_initialized = 0;

static void crc64_init_table(void)
{
	UINT64 poly = 0xC96C5795D7870F42ULL;
	UINT i;
	for (i = 0; i < 256; ++i)
	{
		UINT64 crc = (UINT64)i;
		UINT j;
		for (j = 0; j < 8; ++j)
			crc = (crc & 1) ? (crc >> 1) ^ poly : (crc >> 1);
		crc64_tab[i] = crc;
	}
	crc64_tab_initialized = 1;
}

UINT64 crc64(UINT64 crc, PCBYTE buf, UINT size)
{
	const BYTE* p = buf;
	if (!crc64_tab_initialized)
		crc64_init_table();

	crc = crc ^ ~0ULL;
	while (size--)
		crc = crc64_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	return crc ^ ~0ULL;
}
