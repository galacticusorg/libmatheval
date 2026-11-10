/* Copyright (C) 2026 Free Software Foundation, Inc.

   This file is part of GNU libmatheval

   GNU libmatheval is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   GNU libmatheval is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU libmatheval.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Test of Fortran 77 interface, called through its C calling
   conventions, and of handling of long strings.  */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <matheval.h>

extern int64_t evaluator_create__(char *string, int length);
extern void evaluator_destroy__(int64_t *evaluator);
extern double evaluator_evaluate__(int64_t *evaluator, int *count,
				   char *names, double *values,
				   int length);
extern void evaluator_get_string_chars__(int64_t *evaluator, char *string,
					 int length);
extern void evaluator_get_variables_chars__(int64_t *evaluator,
					    char *string, int length);
extern int64_t evaluator_derivative__(int64_t *evaluator, char *name,
				      int length);

#define CHECK(condition) \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, \
			__LINE__, #condition); \
		return EXIT_FAILURE; \
	}

int
main(void)
{
	char buffer[3000];
	char *string;
	double values[] = { 1, 2, 3 };
	int64_t f, g;
	int count, i;
	void *e;

	/* Output strings are padded with blanks, and never read beyond
	   the evaluator string, whatever length is given.  */
	f = evaluator_create__("x+y", 3);
	CHECK(f != 0);
	evaluator_get_string_chars__(&f, buffer, sizeof(buffer));
	CHECK(!memcmp(buffer, "(x+y)", 5));
	for (i = 5; i < (int) sizeof(buffer); i++)
		CHECK(buffer[i] == ' ');
	memset(buffer, '*', sizeof(buffer));
	evaluator_get_string_chars__(&f, buffer, 3);
	CHECK(!memcmp(buffer, "(x+*", 4));
	evaluator_get_string_chars__(&f, buffer, -1);
	CHECK(buffer[0] == '(');
	evaluator_get_variables_chars__(&f, buffer, -1);
	evaluator_get_variables_chars__(&f, buffer, 10);
	CHECK(!memcmp(buffer, "x y       ", 10));

	/* Names of variables are not read beyond given length, and only
	   names actually given are used.  */
	count = 3;
	CHECK(evaluator_evaluate__(&f, &count, "x y   ", values, 6) == 3);
	CHECK(evaluator_evaluate__(&f, &count, "      ", values, 6) == 3);
	count = 1;
	CHECK(evaluator_evaluate__(&f, &count, "y", values + 2, 1) == 4);

	/* Negative lengths are treated as zero.  */
	CHECK(evaluator_create__("x", -1) == 0);
	g = evaluator_derivative__(&f, "x", -1);
	CHECK(g != 0);
	evaluator_destroy__(&g);
	evaluator_destroy__(&f);

	/* Numbers are rendered completely.  */
	e = evaluator_create("12345.6789");
	CHECK(!strcmp(evaluator_get_string(e), "12345.7"));
	evaluator_destroy(e);
	e = evaluator_create("-x*12345.6789");
	CHECK(!strcmp(evaluator_get_string(e), "((-x)*12345.7)"));
	evaluator_destroy(e);

	/* Long string is parsed completely.  */
	count = 20000;
	string = malloc(2 * count);
	for (i = 0; i < count - 1; i++) {
		string[2 * i] = 'x';
		string[2 * i + 1] = '+';
	}
	strcpy(string + 2 * i, "x");
	e = evaluator_create(string);
	CHECK(e != NULL && evaluator_evaluate_x(e, 1) == count);
	evaluator_destroy(e);
	free(string);

	return EXIT_SUCCESS;
}
