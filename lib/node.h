/* Copyright (C) 1999, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011,
   2012, 2013, 2026 Free Software Foundation, Inc.

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

#ifndef NODE_H
#define NODE_H 1

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include "symbol_table.h"

/* Data structure representing function tree node.  */
typedef struct _Node {
	/* Node type ('n' for number, 'c' for constant, 'v' for variable,
	   'f' for function, 'g' for function of two arguments, 's' for
	   selection, 'u' for unary operation, 'b' for binary
	   operation).  */
	char type;
	union {
		double number;	/* Number value.  */
		/* Symbol table record for constant.  */
		Record *constant;
		/* Symbol table record for variable.  */
		Record *variable;
		struct {
			/* Symbol table record for function.  */
			Record *record;
			/* Function argument node.  */
			struct _Node *child;
		} function;	/* Structure representing function.  */
		/* Structure representing function of two arguments.  */
		struct {
			/* Symbol table record for function.  */
			Record *record;
			/* Function argument nodes.  */
			struct _Node *left, *right;
		} function2;
		/* Structure representing selection.  */
		struct {
			/* Symbol table record for selection.  */
			Record *record;
			/* Condition and argument nodes.  */
			struct _Node *condition, *left, *right;
		} selection;
		/* Structure representing unary operation.  */
		struct {
			/* Operation type ('-' for unary minus).  */
			char operation;
			struct _Node *child;	/* Operand node.  */
		} un_op;
		/* Structure representing binary operation.  */
		struct {
			/* Operation type ('+' for adition, '-' for
			   subtraction, '*' for multiplication, '/' for
			   division and '^' for exponentiation).  */
			char operation;
			/* Operands nodes.  */
			struct _Node *left, *right;
		} bin_op;
	} data;
} Node;

/* Create node of given type and initialize it from optional arguments.
   Function returns pointer to node object that should be passed as first
   argument to all other node functions.  */
Node *node_create(int type, ...);

/* Destroy subtree rooted at specified node.  */
void node_destroy(Node *node);

/* Make a copy of subtree rooted at given node.  Deep copy operation is
   employed.  */
Node *node_copy(Node *node);

/* Simplify subtree rooted at given node.  Function returns root of
   simplified subtree (that may or may not be original node).  */
Node *node_simplify(Node *node);

/* Evaluate subtree rooted at given node.  For variables, values from
   symbol table are used.  */
double node_evaluate(Node *node);

/* Create derivative tree for subtree rooted at given node.  Second
   argument is derivation variable, third argument is symbol table (needed
   for functions derivatives).  Function returns root of corresponding
   derivation tree.  */
Node *node_derivative(Node *node, char *name, SymbolTable *symbol_table);

/* Flag each variable in symbol table that is used from subtree rooted at
   specified node.  */
void node_flag_variables(Node *node);

/* Calculate length of the string representing subtree rooted at specified
   node.  */
size_t node_get_length(Node *node);

/* Write subtree rooted at specified node to given string variable.  No
   checking of string overflow is done by this procedure; it is expected
   that string of appropriate length is passed as argument.  */
void node_write(Node *node, char *string);

#endif
