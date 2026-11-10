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


/* Test that symbols internal to library do not clash with symbols of
   same names in program.  */

#include <stdio.h>
#include <stdlib.h>
#include <matheval.h>

/* Names of variables and functions internal to library.  */
int root, ok, symbol_table;

int
yyparse(void)
{
	abort();
}

int
yylex(void)
{
	abort();
}

void *
xmalloc(size_t size)
{
	(void) size;
	abort();
}

int
main(void)
{
	void *evaluator;	/* Evaluator for function.  */
	double value;	/* Function value.  */

	evaluator = evaluator_create("x+1");
	if (!evaluator)
		return 1;
	value = evaluator_evaluate_x(evaluator, 1);
	evaluator_destroy(evaluator);
	return value == 2 ? 0 : 1;
}
