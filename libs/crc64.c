#include "WinHash.h"

#if !defined(CRC64_NO_SIMD)
#if defined(_M_ARM64) || defined(_M_ARM64EC)
#include <arm64_neon.h>
#define CRC64_SIMD_ARM64
#elif defined(_M_IX86) || defined(_M_AMD64) || defined(_M_X64)
#include <intrin.h>
#include <wmmintrin.h>
#define CRC64_SIMD_X86
#endif
#endif

static UINT64 crc64_tab[256];
static INIT_ONCE crc64_once = INIT_ONCE_STATIC_INIT;
#if defined(CRC64_SIMD_X86) || defined(CRC64_SIMD_ARM64)
static BOOL crc64_has_simd;
#endif

static BOOL CALLBACK crc64_init(PINIT_ONCE once, PVOID parameter, PVOID* context)
{
	const UINT64 poly = 0xC96C5795D7870F42ULL;
	UINT i;
	(void)once;
	(void)parameter;
	(void)context;

	for (i = 0; i < 256; ++i)
	{
		UINT64 crc = (UINT64)i;
		UINT j;
		for (j = 0; j < 8; ++j)
			crc = (crc & 1) ? (crc >> 1) ^ poly : (crc >> 1);
		crc64_tab[i] = crc;
	}

#if defined(CRC64_SIMD_X86)
	{
		int cpu[4];
		__cpuid(cpu, 0);
		if (cpu[0] >= 1)
		{
			__cpuid(cpu, 1);
			// Only SSE2 and PCLMULQDQ are used; AVX/XSAVE are not required.
			crc64_has_simd = (cpu[2] & (1 << 1)) != 0 &&
				(cpu[3] & (1 << 26)) != 0 &&
				IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
		}
	}
#elif defined(CRC64_SIMD_ARM64)
	// Windows reports the ARMv8 crypto extension, including PMULL.
	crc64_has_simd = IsProcessorFeaturePresent(PF_ARM_V8_CRYPTO_INSTRUCTIONS_AVAILABLE);
#endif

	return TRUE;
}

// Operates on the uncomplemented running state, also used to reduce SIMD folds.
static UINT64 crc64_scalar(UINT64 crc, PCBYTE buf, UINT size)
{
	while (size--)
		crc = crc64_tab[(crc ^ *buf++) & 0xFF] ^ (crc >> 8);
	return crc;
}

#if defined(CRC64_SIMD_X86) || defined(CRC64_SIMD_ARM64)

#if defined(CRC64_SIMD_X86)
typedef __m128i CRC64_VECTOR;

static __forceinline CRC64_VECTOR crc64_load(PCBYTE buf)
{
	return _mm_loadu_si128((const __m128i*)buf);
}

static __forceinline CRC64_VECTOR crc64_xor(CRC64_VECTOR a, CRC64_VECTOR b)
{
	return _mm_xor_si128(a, b);
}

static __forceinline CRC64_VECTOR crc64_fold(CRC64_VECTOR value, CRC64_VECTOR factors)
{
	return _mm_xor_si128(_mm_clmulepi64_si128(value, factors, 0x00),
		_mm_clmulepi64_si128(value, factors, 0x11));
}

static __forceinline void crc64_store(UINT64* words, CRC64_VECTOR value)
{
	_mm_storeu_si128((__m128i*)words, value);
}
#else
typedef uint64x2_t CRC64_VECTOR;

static __forceinline CRC64_VECTOR crc64_load(PCBYTE buf)
{
	return vreinterpretq_u64_u8(vld1q_u8(buf));
}

static __forceinline CRC64_VECTOR crc64_xor(CRC64_VECTOR a, CRC64_VECTOR b)
{
	return veorq_u64(a, b);
}

static __forceinline CRC64_VECTOR crc64_fold(CRC64_VECTOR value, CRC64_VECTOR factors)
{
	poly64x2_t a = vreinterpretq_p64_u64(value);
	poly64x2_t b = vreinterpretq_p64_u64(factors);
	// MSVC's vmull_p64 takes poly64x1_t operands, unlike scalar ACLE variants.
	return veorq_u64(vreinterpretq_u64_p128(vmull_p64(vget_low_p64(a), vget_low_p64(b))),
		vreinterpretq_u64_p128(vmull_high_p64(a, b)));
}

static __forceinline void crc64_store(UINT64* words, CRC64_VECTOR value)
{
	vst1q_u64(words, value);
}
#endif

// Reflected remainders of x^191/x^127 and x^575/x^511 modulo
// x^64 + 0x42F0E1EBA9EA3693; the -1 accounts for reflected CLMUL products.
static const UINT64 crc64_fold16[2] = { 0xE05DD497CA393AE4ULL, 0xDABE95AFC7875F40ULL };
static const UINT64 crc64_fold64[2] = { 0x6AE3EFBB9DD441F3ULL, 0x081F6054A7842DF4ULL };

// Keep optional instructions out of the dispatcher, including with MSVC /GL.
// Requires size >= 256; unaligned loads only read complete 16-byte blocks.
static __declspec(noinline) UINT64 crc64_simd(UINT64 crc, PCBYTE buf, UINT size)
{
	const UINT64 initial[2] = { crc, 0 };
	CRC64_VECTOR k16 = crc64_load((PCBYTE)crc64_fold16);
	CRC64_VECTOR k64 = crc64_load((PCBYTE)crc64_fold64);
	CRC64_VECTOR a = crc64_xor(crc64_load(buf), crc64_load((PCBYTE)initial));
	CRC64_VECTOR b = crc64_load(buf + 16);
	CRC64_VECTOR c = crc64_load(buf + 32);
	CRC64_VECTOR d = crc64_load(buf + 48);
	UINT64 words[2];
	buf += 64;
	size -= 64;

	// Four independent chains hide carry-less multiplication latency.
	while (size >= 64)
	{
		a = crc64_xor(crc64_fold(a, k64), crc64_load(buf));
		b = crc64_xor(crc64_fold(b, k64), crc64_load(buf + 16));
		c = crc64_xor(crc64_fold(c, k64), crc64_load(buf + 32));
		d = crc64_xor(crc64_fold(d, k64), crc64_load(buf + 48));
		buf += 64;
		size -= 64;
	}

	a = crc64_xor(crc64_fold(a, k16), b);
	a = crc64_xor(crc64_fold(a, k16), c);
	a = crc64_xor(crc64_fold(a, k16), d);

	while (size >= 16)
	{
		a = crc64_xor(crc64_fold(a, k16), crc64_load(buf));
		buf += 16;
		size -= 16;
	}

	crc64_store(words, a);
	crc = crc64_scalar(0, (PCBYTE)words, sizeof(words));
	return crc64_scalar(crc, buf, size);
}
#endif

UINT64 crc64(UINT64 crc, PCBYTE buf, UINT size)
{
	if (size == 0)
		return crc;

	// Publishes both the lookup table and CPU dispatch choice to all workers.
	InitOnceExecuteOnce(&crc64_once, crc64_init, NULL, NULL);
	crc = ~crc;
#if defined(CRC64_SIMD_X86) || defined(CRC64_SIMD_ARM64)
	if (size >= 256 && crc64_has_simd)
		return ~crc64_simd(crc, buf, size);
#endif
	return ~crc64_scalar(crc, buf, size);
}
