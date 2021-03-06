/*
 * strchrnul test.
 *
 * Copyright (c) 2019, Arm Limited.
 * SPDX-License-Identifier: MIT
 */

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "stringlib.h"
#include "stringtest.h"

static const struct fun
{
	const char *name;
	char *(*fun)(const char *s, int c);
} funtab[] = {
#define F(x) {#x, x},
F(strchrnul)
#if __aarch64__
F(__strchrnul_aarch64)
F(__strchrnul_aarch64_mte)
# if __ARM_FEATURE_SVE
F(__strchrnul_aarch64_sve)
# endif
#endif
#undef F
	{0, 0}
};

#define A 32
#define LEN 512
static char sbuf[LEN+3*A];

static void *alignup(void *p)
{
	return (void*)(((uintptr_t)p + A-1) & -A);
}

static void test(const struct fun *fun, int align, int seekpos, int len)
{
	char *src = alignup(sbuf);
	char *s = src + align;
	char *f = seekpos != -1 ? s + seekpos : s + len;
	int seekchar = 0x1;
	void *p;

	if (err_count >= ERR_LIMIT)
		return;
	if (len > LEN || seekpos >= len || align >= A)
		abort();

	for (int i = 0; src + i < s; i++)
		src[i] = i & 1 ? seekchar : 0;
	for (int i = 1; i < A; i++)
		s[len+i] = i & 1 ? seekchar : 0;
	for (int i = 0; i < len; i++)
		s[i] = 'a' + i%32;
	if (seekpos != -1)
		s[seekpos] = s[seekpos+2] = seekchar;
	s[len] = '\0';

	p = fun->fun(s, seekchar);
	if (p != f) {
		ERR("%s(%p,0x%02x) len %d returned %p, expected %p pos %d\n",
			fun->name, s, seekchar, len, p, f, seekpos);
		quote("input", s, len);
	}

	p = fun->fun(s, 0);
	if (p != s + len) {
		ERR("%s(%p,0x%02x) len %d returned %p, expected %p pos %d\n",
			fun->name, s, seekchar, len, p, s + len, len);
		quote("input", s, len);
	}
}

int main()
{
	int r = 0;
	for (int i=0; funtab[i].name; i++) {
		err_count = 0;
		for (int a = 0; a < A; a++) {
			int n;
			for (n = 1; n < LEN; n++) {
				for (int sp = 0; sp < n; sp++)
					test(funtab+i, a, sp, n);
				test(funtab+i, a, -1, n);
			}
		}
		printf("%s %s\n", err_count ? "FAIL" : "PASS", funtab[i].name);
		if (err_count)
			r = -1;
	}
	return r;
}
