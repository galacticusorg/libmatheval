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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <assert.h>
#include <stdarg.h>
#include "common.h"
#include "node.h"

Node *
node_create(int type, ...)
{
	Node *node;	/* New node.  */
	va_list ap;	/* Variable argument list.  */

	/* Allocate memory for node and initialize its type.  */
	node = XMALLOC(Node, 1);
	node->type = type;

	/* According to node type, initialize rest of the node from
	   variable argument list.  */
	va_start(ap, type);
	switch (node->type) {
	case 'n':
		/* Initialize number value.  */
		node->data.number = va_arg(ap, double);
		break;

	case 'c':
		/* Remember pointer to symbol table record describing
		   constant.  */
		node->data.constant = va_arg(ap, Record *);
		break;

	case 'v':
		/* Remember pointer to symbol table record describing
		   variable.  */
		node->data.variable = va_arg(ap, Record *);
		break;

	case 'f':
		/* Remember pointer to symbol table record describing
		   function and initialize function argument.  */
		node->data.function.record = va_arg(ap, Record *);
		node->data.function.child = va_arg(ap, Node *);
		break;

	case 'g':
		/* Remember pointer to symbol table record describing
		   function and initialize function arguments.  */
		node->data.function2.record = va_arg(ap, Record *);
		node->data.function2.left = va_arg(ap, Node *);
		node->data.function2.right = va_arg(ap, Node *);
		break;

	case 's':
		/* Remember pointer to symbol table record describing
		   selection and initialize condition and arguments.  */
		node->data.selection.record = va_arg(ap, Record *);
		node->data.selection.condition = va_arg(ap, Node *);
		node->data.selection.left = va_arg(ap, Node *);
		node->data.selection.right = va_arg(ap, Node *);
		break;

	case 'u':
		/* Initialize operation type and operand.  */
		node->data.un_op.operation = (char) va_arg(ap, int);

		node->data.un_op.child = va_arg(ap, Node *);
		break;

	case 'b':
		/* Initialize operation type and operands.  */
		node->data.un_op.operation = (char) va_arg(ap, int);

		node->data.bin_op.left = va_arg(ap, Node *);
		node->data.bin_op.right = va_arg(ap, Node *);
		break;

	default:
		assert(0);
	}
	va_end(ap);

	return node;
}

void
node_destroy(Node *node)
{
	/* Skip if node already null (this may occur during
	   simplification).  */
	if (!node)
		return;

	/* If necessary, destroy subtree rooted at node.  */
	switch (node->type) {
	case 'n':
	case 'c':
	case 'v':
		break;

	case 'f':
		node_destroy(node->data.function.child);
		break;

	case 'g':
		node_destroy(node->data.function2.left);
		node_destroy(node->data.function2.right);
		break;

	case 's':
		node_destroy(node->data.selection.condition);
		node_destroy(node->data.selection.left);
		node_destroy(node->data.selection.right);
		break;

	case 'u':
		node_destroy(node->data.un_op.child);
		break;

	case 'b':
		node_destroy(node->data.bin_op.left);
		node_destroy(node->data.bin_op.right);
		break;
	}

	/* Deallocate memory used by node.  */
	XFREE(node);
}

Node *
node_copy(Node *node)
{
	Node *n0, *n1, *n2;	/* Copies of children.  */

	/* According to node type, create (deep) copy of subtree rooted at
	   node.  */
	switch (node->type) {
	case 'n':
		return node_create('n', node->data.number);

	case 'c':
		return node_create('c', node->data.constant);

	case 'v':
		return node_create('v', node->data.variable);

	case 'f':
		n0 = node_copy(node->data.function.child);
		return node_create('f', node->data.function.record, n0);

	case 'g':
		n0 = node_copy(node->data.function2.left);
		n1 = node_copy(node->data.function2.right);
		return node_create('g', node->data.function2.record, n0,
				   n1);

	case 's':
		n0 = node_copy(node->data.selection.condition);
		n1 = node_copy(node->data.selection.left);
		n2 = node_copy(node->data.selection.right);
		return node_create('s', node->data.selection.record, n0,
				   n1, n2);

	case 'u':
		n0 = node_copy(node->data.un_op.child);
		return node_create('u', node->data.un_op.operation, n0);

	case 'b':
		n0 = node_copy(node->data.bin_op.left);
		n1 = node_copy(node->data.bin_op.right);
		return node_create('b', node->data.bin_op.operation, n0,
				   n1);
	}

	return NULL;
}

Node *
node_simplify(Node *node)
{
	/* According to node type, apply further simplifications.
	   Constants are not simplified, in order to eventually appear
	   unchanged in derivatives.  */
	switch (node->type) {
	case 'n':
	case 'c':
	case 'v':
		return node;

	case 'f':
		/* Simplify function argument and if number evaluate
		   function and replace function node with number node.  */
		node->data.function.child =
		    node_simplify(node->data.function.child);
		if (node->data.function.child->type == 'n') {
			double value = node_evaluate(node);

			node_destroy(node);
			return node_create('n', value);
		} else
			return node;

	case 'g':
		/* Simplify function arguments and if numbers evaluate
		   function and replace function node with number node.  */
		node->data.function2.left =
		    node_simplify(node->data.function2.left);
		node->data.function2.right =
		    node_simplify(node->data.function2.right);
		if (node->data.function2.left->type == 'n'
		    && node->data.function2.right->type == 'n') {
			double value = node_evaluate(node);

			node_destroy(node);
			return node_create('n', value);
		} else
			return node;

	case 's':
		/* Simplify condition, and if number or constant replace
		   selection node with selected argument node, other
		   argument being not simplified (thus not evaluated).  */
		node->data.selection.condition =
		    node_simplify(node->data.selection.condition);
		if (node->data.selection.condition->type == 'n'
		    || node->data.selection.condition->type == 'c') {
			double value =
			    node_evaluate(node->data.selection.condition);
			Node *selected;

			if (value != 0 && !isnan(value)) {
				selected = node->data.selection.left;
				node->data.selection.left = NULL;
			} else {
				selected = node->data.selection.right;
				node->data.selection.right = NULL;
			}
			node_destroy(node);
			return node_simplify(selected);
		}

		/* Otherwise simplify arguments, and if same number,
		   condition does not matter.  */
		node->data.selection.left =
		    node_simplify(node->data.selection.left);
		node->data.selection.right =
		    node_simplify(node->data.selection.right);
		if (node->data.selection.left->type == 'n'
		    && node->data.selection.right->type == 'n'
		    && node->data.selection.left->data.number
		    == node->data.selection.right->data.number) {
			double value =
			    node->data.selection.left->data.number;

			node_destroy(node);
			return node_create('n', value);
		} else
			return node;

	case 'u':
		/* Simplify unary operation operand and if number apply
		   operation and replace operation node with number
		   node.  */
		node->data.un_op.child =
		    node_simplify(node->data.un_op.child);
		if (node->data.un_op.operation == '-'
		    && node->data.un_op.child->type == 'n') {
			double value = node_evaluate(node);

			node_destroy(node);
			return node_create('n', value);
		} else
			return node;

	case 'b':
		/* Simplify binary operation operands.  */
		node->data.bin_op.left =
		    node_simplify(node->data.bin_op.left);
		node->data.bin_op.right =
		    node_simplify(node->data.bin_op.right);

		/* If operands numbers apply operation and replace
		   operation node with number node.  */
		if (node->data.bin_op.left->type == 'n'
		    && node->data.bin_op.right->type == 'n') {
			double value = node_evaluate(node);

			node_destroy(node);
			return node_create('n', value);
		}
		/* Eliminate 0 as neutral addition operand.  */
		else if (node->data.bin_op.operation == '+')
			if (node->data.bin_op.left->type == 'n'
			    && node->data.bin_op.left->data.number == 0) {
				Node *right;

				right = node->data.bin_op.right;
				node->data.bin_op.right = NULL;
				node_destroy(node);
				return right;
			} else if (node->data.bin_op.right->type == 'n'
				   && node->data.bin_op.right->data.number
				   == 0) {
				Node *left;

				left = node->data.bin_op.left;
				node->data.bin_op.left = NULL;
				node_destroy(node);
				return left;
			} else
				return node;
		/* Eliminate 0 as neutral subtraction right operand.  */
		else if (node->data.bin_op.operation == '-')
			if (node->data.bin_op.right->type == 'n'
			    && node->data.bin_op.right->data.number == 0) {
				Node *left;

				left = node->data.bin_op.left;
				node->data.bin_op.left = NULL;
				node_destroy(node);
				return left;
			} else
				return node;
		/* Eliminate 1 as neutral multiplication operand.  */
		else if (node->data.bin_op.operation == '*')
			if (node->data.bin_op.left->type == 'n'
			    && node->data.bin_op.left->data.number == 1) {
				Node *right;

				right = node->data.bin_op.right;
				node->data.bin_op.right = NULL;
				node_destroy(node);
				return right;
			} else if (node->data.bin_op.right->type == 'n'
				   && node->data.bin_op.right->data.number
				   == 1) {
				Node *left;

				left = node->data.bin_op.left;
				node->data.bin_op.left = NULL;
				node_destroy(node);
				return left;
			} else
				return node;
		/* Eliminate 1 as neutral division right operand.  */
		else if (node->data.bin_op.operation == '/')
			if (node->data.bin_op.right->type == 'n'
			    && node->data.bin_op.right->data.number == 1) {
				Node *left;

				left = node->data.bin_op.left;
				node->data.bin_op.left = NULL;
				node_destroy(node);
				return left;
			} else
				return node;
		/* Eliminate 1 as left exponentiation operand, and 0 and 1
		   as right exponentiation operand (0 as left operand is
		   not eliminated, as 0^x is not 0 for x not positive).  */
		else if (node->data.bin_op.operation == '^')
			if (node->data.bin_op.left->type == 'n'
			    && node->data.bin_op.left->data.number == 1) {
				node_destroy(node);
				return node_create('n', 1.0);
			} else if (node->data.bin_op.right->type == 'n'
				   && node->data.bin_op.right->data.number
				   == 0) {
				node_destroy(node);
				return node_create('n', 1.0);
			} else if (node->data.bin_op.right->type == 'n'
				   && node->data.bin_op.right->data.number
				   == 1) {
				Node *left;

				left = node->data.bin_op.left;
				node->data.bin_op.left = NULL;
				node_destroy(node);
				return left;
			} else
				return node;
		else
			return node;
	}

	return node;
}

double
node_evaluate(Node *node)
{
	Record *record;	/* Symbol table record.  */
	Node *child;	/* Function argument or operand.  */
	Node *left, *right;	/* Arguments or operands.  */
	Node *condition;	/* Selection condition.  */
	double value;	/* Selection condition value.  */

	/* According to node type, evaluate subtree rooted at node.  */
	switch (node->type) {
	case 'n':
		return node->data.number;

	case 'c':
		/* Constant values are used from symbol table.  */
		return node->data.constant->data.value;

	case 'v':
		/* Variable values are used from symbol table.  */
		return node->data.variable->data.value;

	case 'f':
		record = node->data.function.record;
		child = node->data.function.child;
		/* Functions are evaluated through symbol table.  */
		return (*record->data.function) (node_evaluate(child));

	case 'g':
		record = node->data.function2.record;
		left = node->data.function2.left;
		right = node->data.function2.right;
		/* Functions are evaluated through symbol table.  */
		return (*record->data.function2) (node_evaluate(left),
						  node_evaluate(right));

	case 's':
		condition = node->data.selection.condition;
		left = node->data.selection.left;
		right = node->data.selection.right;
		/* Only selected argument is evaluated; condition not a
		   number selects second argument.  */
		value = node_evaluate(condition);
		if (value != 0 && !isnan(value))
			return node_evaluate(left);
		return node_evaluate(right);

	case 'u':
		child = node->data.un_op.child;
		/* Unary operation node is evaluated according to
		   operation type.  */
		switch (node->data.un_op.operation) {
		case '-':
			return -node_evaluate(child);
		}
		break;

	case 'b':
		left = node->data.bin_op.left;
		right = node->data.bin_op.right;
		/* Binary operation node is evaluated according to
		   operation type.  */
		switch (node->data.bin_op.operation) {
		case '+':
			return node_evaluate(left) + node_evaluate(right);

		case '-':
			return node_evaluate(left) - node_evaluate(right);

		case '*':
			return node_evaluate(left) * node_evaluate(right);

		case '/':
			return node_evaluate(left) / node_evaluate(right);

		case '^':
			return pow(node_evaluate(left),
				   node_evaluate(right));
		}
	}

	return 0;
}

Node *
node_derivative(Node *node, char *name, SymbolTable *symbol_table)
{
	Record *record;	/* Symbol table record.  */
	Node *child;	/* Function argument or operand.  */
	Node *left, *right;	/* Arguments or operands.  */
	Node *condition;	/* Selection condition.  */
	/* Nodes of derivative tree.  */
	Node *n0, *n1, *n2, *n3, *n4, *n5, *n6, *n7, *n8, *n9, *n10, *n11,
	    *n12, *n13, *n14, *n15, *n16;
	Record *r0, *r1, *r2;	/* Records of functions used.  */

	/* According to node type, derivative tree for subtree rooted at
	   node is created.  */
	switch (node->type) {
	case 'n':
		/* Derivative of number equals 0.  */
		return node_create('n', 0.0);

	case 'c':
		/* Derivative of constant equals 0.  */
		return node_create('n', 0.0);

	case 'v':
		record = node->data.variable;
		/* Derivative of variable equals 1 if variable is
		   derivative variable, 0 otherwise.  */
		return node_create('n',
				   (!strcmp(name, record->name)) ? 1.0 :
				   0.0);

	case 'f':
		record = node->data.function.record;
		child = node->data.function.child;
		/* Apply rule of exponential function derivative.  */
		if (!strcmp(record->name, "exp")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_copy(node);
			return node_create('b', '*', n0, n1);
		}
		/* Apply rule of logarithmic function derivative.  */
		if (!strcmp(record->name, "log")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_copy(child);
			return node_create('b', '/', n0, n1);
		}
		/* Apply rule of square root function derivative.  */
		if (!strcmp(record->name, "sqrt")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 2.0);
			n2 = node_copy(node);
			n3 = node_create('b', '*', n1, n2);
			return node_create('b', '/', n0, n3);
		}
		/* Apply rule of sine function derivative.  */
		if (!strcmp(record->name, "sin")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "cos");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			return node_create('b', '*', n0, n2);
		}
		/* Apply rule of cosine function derivative.  */
		if (!strcmp(record->name, "cos")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sin");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			n3 = node_create('b', '*', n0, n2);
			return node_create('u', '-', n3);
		}
		/* Apply rule of tangent function derivative.  */
		if (!strcmp(record->name, "tan")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "cos");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			return node_create('b', '/', n0, n4);
		}
		/* Apply rule of cotangent function derivative.  */
		if (!strcmp(record->name, "cot")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sin");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '/', n0, n4);
			return node_create('u', '-', n5);
		}
		/* Apply rule of secant function derivative.  */
		if (!strcmp(record->name, "sec")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sec");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			r1 = symbol_table_lookup(symbol_table, "tan");
			n3 = node_copy(child);
			n4 = node_create('f', r1, n3);
			n5 = node_create('b', '*', n2, n4);
			return node_create('b', '*', n0, n5);
		}
		/* Apply rule of cosecant function derivative.  */
		if (!strcmp(record->name, "csc")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "cot");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			r1 = symbol_table_lookup(symbol_table, "csc");
			n3 = node_copy(child);
			n4 = node_create('f', r1, n3);
			n5 = node_create('b', '*', n2, n4);
			n6 = node_create('u', '-', n5);
			return node_create('b', '*', n0, n6);
		}
		/* Apply rule of inverse sine function derivative.  */
		if (!strcmp(record->name, "asin")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '-', n1, n4);
			n6 = node_create('f', r0, n5);
			return node_create('b', '/', n0, n6);
		}
		/* Apply rule of inverse cosine function derivative.  */
		if (!strcmp(record->name, "acos")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '-', n1, n4);
			n6 = node_create('f', r0, n5);
			n7 = node_create('b', '/', n0, n6);
			return node_create('u', '-', n7);
		}
		/* Apply rule of inverse tangent function derivative.  */
		if (!strcmp(record->name, "atan")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '+', n1, n4);
			return node_create('b', '/', n0, n5);
		}
		/* Apply rule of inverse cotanget function derivative.  */
		if (!strcmp(record->name, "acot")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '+', n1, n4);
			n6 = node_create('b', '/', n0, n5);
			return node_create('u', '-', n6);
		}
		/* Apply rule of inverse secant function derivative.  */
		if (!strcmp(record->name, "asec")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n5 = node_create('n', 1.0);
			n6 = node_create('n', 1.0);
			n7 = node_copy(child);
			n8 = node_create('n', 2.0);
			n9 = node_create('b', '^', n7, n8);
			n10 = node_create('b', '/', n6, n9);
			n11 = node_create('b', '-', n5, n10);
			n12 = node_create('f', r0, n11);
			n13 = node_create('b', '*', n4, n12);
			n14 = node_create('b', '/', n1, n13);
			return node_create('b', '*', n0, n14);
		}
		/* Apply rule of inverse cosecant function derivative.  */
		if (!strcmp(record->name, "acsc")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n5 = node_create('n', 1.0);
			n6 = node_create('n', 1.0);
			n7 = node_copy(child);
			n8 = node_create('n', 2.0);
			n9 = node_create('b', '^', n7, n8);
			n10 = node_create('b', '/', n6, n9);
			n11 = node_create('b', '-', n5, n10);
			n12 = node_create('f', r0, n11);
			n13 = node_create('b', '*', n4, n12);
			n14 = node_create('b', '/', n1, n13);
			n15 = node_create('u', '-', n14);
			return node_create('b', '*', n0, n15);
		}
		/* Apply rule of hyperbolic sine function derivative.  */
		if (!strcmp(record->name, "sinh")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "cosh");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			return node_create('b', '*', n0, n2);
		}
		/* Apply rule of hyperbolic cosine function derivative.  */
		if (!strcmp(record->name, "cosh")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sinh");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			return node_create('b', '*', n0, n2);
		}
		/* Apply rule of hyperbolic tangent function
		   derivative.  */
		if (!strcmp(record->name, "tanh")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "cosh");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			return node_create('b', '/', n0, n4);
		}
		/* Apply rule of hyperbolic cotangent function
		   derivative.  */
		if (!strcmp(record->name, "coth")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sinh");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '/', n0, n4);
			return node_create('u', '-', n5);
		}
		/* Apply rule of hyperbolic secant function derivative.  */
		if (!strcmp(record->name, "sech")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sech");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			r1 = symbol_table_lookup(symbol_table, "tanh");
			n3 = node_copy(child);
			n4 = node_create('f', r1, n3);
			n5 = node_create('b', '*', n2, n4);
			n6 = node_create('u', '-', n5);
			return node_create('b', '*', n0, n6);
		}
		/* Apply rule of hyperbolic cosecant function
		   derivative.  */
		if (!strcmp(record->name, "csch")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "coth");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			r1 = symbol_table_lookup(symbol_table, "csch");
			n3 = node_copy(child);
			n4 = node_create('f', r1, n3);
			n5 = node_create('b', '*', n2, n4);
			n6 = node_create('u', '-', n5);
			return node_create('b', '*', n0, n6);
		}
		/* Apply rule of inverse hyperbolic sine function
		   derivative.  */
		if (!strcmp(record->name, "asinh")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '+', n1, n4);
			n6 = node_create('f', r0, n5);
			return node_create('b', '/', n0, n6);
		}
		/* Apply rule of inverse hyperbolic cosine function
		   derivative.  */
		if (!strcmp(record->name, "acosh")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n1 = node_copy(child);
			n2 = node_create('n', 2.0);
			n3 = node_create('b', '^', n1, n2);
			n4 = node_create('n', 1.0);
			n5 = node_create('b', '-', n3, n4);
			n6 = node_create('f', r0, n5);
			return node_create('b', '/', n0, n6);
		}
		/* Apply rule of inverse hyperbolic tangent function
		   derivative.  */
		if (!strcmp(record->name, "atanh")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '-', n1, n4);
			return node_create('b', '/', n0, n5);
		}
		/* Apply rule of inverse hyperbolic cotangent function
		   derivative.  */
		if (!strcmp(record->name, "acoth")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('b', '-', n1, n4);
			return node_create('b', '/', n0, n5);
		}
		/* Apply rule of inverse hyperbolic secant function
		   derivative.  */
		if (!strcmp(record->name, "asech")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n3 = node_create('n', 1.0);
			n4 = node_copy(child);
			n5 = node_create('b', '-', n3, n4);
			n6 = node_create('f', r0, n5);
			n7 = node_create('b', '*', n2, n6);
			n8 = node_create('b', '/', n1, n7);
			r1 = symbol_table_lookup(symbol_table, "sqrt");
			n9 = node_create('n', 1.0);
			n10 = node_create('n', 1.0);
			n11 = node_copy(child);
			n12 = node_create('b', '+', n10, n11);
			n13 = node_create('b', '/', n9, n12);
			n14 = node_create('f', r1, n13);
			n15 = node_create('b', '*', n8, n14);
			n16 = node_create('u', '-', n15);
			return node_create('b', '*', n0, n16);
		}
		/* Apply rule of inverse hyperbolic cosecant function
		   derivative.  */
		if (!strcmp(record->name, "acsch")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 1.0);
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			r0 = symbol_table_lookup(symbol_table, "sqrt");
			n5 = node_create('n', 1.0);
			n6 = node_create('n', 1.0);
			n7 = node_copy(child);
			n8 = node_create('n', 2.0);
			n9 = node_create('b', '^', n7, n8);
			n10 = node_create('b', '/', n6, n9);
			n11 = node_create('b', '+', n5, n10);
			n12 = node_create('f', r0, n11);
			n13 = node_create('b', '*', n4, n12);
			n14 = node_create('b', '/', n1, n13);
			n15 = node_create('u', '-', n14);
			return node_create('b', '*', n0, n15);
		}
		/* Apply rule of absolute value function derivative.  */
		if (!strcmp(record->name, "abs")) {
			n0 = node_derivative(child, name, symbol_table);
			n1 = node_create('n', 2.0);
			r0 = symbol_table_lookup(symbol_table, "step");
			n2 = node_copy(child);
			n3 = node_create('f', r0, n2);
			n4 = node_create('b', '*', n1, n3);
			n5 = node_create('n', 1.0);
			n6 = node_create('b', '-', n4, n5);
			return node_create('b', '*', n0, n6);
		}
		/* Apply rule of step function derivative.  */
		if (!strcmp(record->name, "step")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "delta");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			return node_create('b', '*', n0, n2);
		}
		/* Apply rule of delta function derivative.  */
		if (!strcmp(record->name, "delta")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "nandelta");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			return node_create('b', '*', n0, n2);
		}
		/* Apply rule of nandelta function derivative.  */
		if (!strcmp(record->name, "nandelta")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "nandelta");
			n1 = node_copy(child);
			n2 = node_create('f', r0, n1);
			return node_create('b', '*', n0, n2);
		}
		/* Apply rule of erf function derivative.  */
		if (!strcmp(record->name, "erf")) {
			n0 = node_derivative(child, name, symbol_table);
			r0 = symbol_table_lookup(symbol_table, "2_sqrtpi");
			n1 = node_create('c', r0);
			r1 = symbol_table_lookup(symbol_table, "exp");
			n2 = node_copy(child);
			n3 = node_create('n', 2.0);
			n4 = node_create('b', '^', n2, n3);
			n5 = node_create('u', '-', n4);
			n6 = node_create('f', r1, n5);
			n7 = node_create('b', '*', n1, n6);
			return node_create('b', '*', n0, n7);
		}

		break;

	case 'g':
		record = node->data.function2.record;
		left = node->data.function2.left;
		right = node->data.function2.right;
		/* Apply rules of min and max functions derivatives: take
		   derivative of argument returned by C library fmin() or
		   fmax(), of f also where f and g are equal:
		   max(f,g)'=select(c,f',g'), with
		   c=select(step(f-g),1,select(step(g-f),0,step(abs(f)))),
		   and min alike with differences swapped.  Inner
		   selections decide only if a difference is not a number:
		   equal infinite arguments, or argument not a number
		   (ignored by C library unless both are).  Derivative of
		   argument not selected is never evaluated.  */
		n0 = node_copy(left);
		n1 = node_copy(right);
		n2 = node_copy(left);
		n3 = node_copy(right);
		if (!strcmp(record->name, "min")) {
			n4 = node_create('b', '-', n1, n0);
			n5 = node_create('b', '-', n2, n3);
		} else {
			n4 = node_create('b', '-', n0, n1);
			n5 = node_create('b', '-', n3, n2);
		}
		r0 = symbol_table_lookup(symbol_table, "step");
		n6 = node_create('f', r0, n4);
		n7 = node_create('f', r0, n5);
		n8 = node_copy(left);
		r1 = symbol_table_lookup(symbol_table, "abs");
		n9 = node_create('f', r1, n8);
		n10 = node_create('f', r0, n9);
		r2 = symbol_table_lookup(symbol_table, "select");
		n11 = node_create('n', 0.0);
		n12 = node_create('s', r2, n7, n11, n10);
		n13 = node_create('n', 1.0);
		n14 = node_create('s', r2, n6, n13, n12);
		n15 = node_derivative(left, name, symbol_table);
		n16 = node_derivative(right, name, symbol_table);
		return node_create('s', r2, n14, n15, n16);

	case 's':
		record = node->data.selection.record;
		condition = node->data.selection.condition;
		left = node->data.selection.left;
		right = node->data.selection.right;
		/* Apply select(c,f,g)'=select(c,f',g') derivative rule:
		   derivative of selected argument, also where condition
		   changes (as SymPy does for Piecewise).  */
		n0 = node_copy(condition);
		n1 = node_derivative(left, name, symbol_table);
		n2 = node_derivative(right, name, symbol_table);
		return node_create('s', record, n0, n1, n2);

	case 'u':
		child = node->data.un_op.child;
		switch (node->data.un_op.operation) {
		case '-':
			/* Apply (-f)'=-f' derivative rule.  */
			n0 = node_derivative(child, name, symbol_table);
			return node_create('u', '-', n0);
		}

		break;

	case 'b':
		left = node->data.bin_op.left;
		right = node->data.bin_op.right;
		switch (node->data.bin_op.operation) {
		case '+':
			/* Apply (f+g)'=f'+g' derivative rule.  */
			n0 = node_derivative(left, name, symbol_table);
			n1 = node_derivative(right, name, symbol_table);
			return node_create('b', '+', n0, n1);

		case '-':
			/* Apply (f-g)'=f'-g' derivative rule.  */
			n0 = node_derivative(left, name, symbol_table);
			n1 = node_derivative(right, name, symbol_table);
			return node_create('b', '-', n0, n1);

		case '*':
			/* Apply (f*g)'=f'*g+f*g' derivative rule.  */
			n0 = node_derivative(left, name, symbol_table);
			n1 = node_copy(right);
			n2 = node_create('b', '*', n0, n1);
			n3 = node_copy(left);
			n4 = node_derivative(right, name, symbol_table);
			n5 = node_create('b', '*', n3, n4);
			return node_create('b', '+', n2, n5);

		case '/':
			/* Apply (f/g)'=(f'*g-f*g')/g^2 derivative
			   rule.  */
			n0 = node_derivative(left, name, symbol_table);
			n1 = node_copy(right);
			n2 = node_create('b', '*', n0, n1);
			n3 = node_copy(left);
			n4 = node_derivative(right, name, symbol_table);
			n5 = node_create('b', '*', n3, n4);
			n6 = node_create('b', '-', n2, n5);
			n7 = node_copy(right);
			n8 = node_create('n', 2.0);
			n9 = node_create('b', '^', n7, n8);
			return node_create('b', '/', n6, n9);

		case '^':
			/* If right operand of exponentiation number apply
			   (f^n)'=n*f^(n-1)*f' derivative rule.  */
			if (right->type == 'n') {
				n0 = node_copy(right);
				n1 = node_derivative(left, name,
						     symbol_table);
				n2 = node_create('b', '*', n0, n1);
				n3 = node_copy(left);
				n4 = node_copy(right);
				n5 = node_create('n', 1.0);
				n6 = node_create('b', '-', n4, n5);
				n7 = node_create('b', '^', n3, n6);
				return node_create('b', '*', n2, n7);
			}
			/* If left operand of exponentiation zero, apply
			   (0^g)'=0 derivative rule.  */
			if (left->type == 'n' && left->data.number == 0)
				return node_create('n', 0.0);
			/* Otherwise, apply logarithmic derivative rule:
			   (log(f^g))'=(f^g)'/f^g =>
			   (f^g)'=f^g*(log(f^g))'=f^g*(g*log(f))' */
			n0 = node_copy(right);
			n1 = node_copy(left);
			r0 = symbol_table_lookup(symbol_table, "log");
			n2 = node_create('f', r0, n1);
			n3 = node_create('b', '*', n0, n2);
			n4 = node_copy(node);
			n5 = node_derivative(n3, name, symbol_table);
			node_destroy(n3);
			return node_create('b', '*', n4, n5);
		}
	}

	return NULL;
}

void
node_flag_variables(Node *node)
{
	/* According to node type, flag variable in symbol table or
	   proceed with calling function recursively on node children.  */
	switch (node->type) {
	case 'v':
		node->data.variable->flag = TRUE;
		break;

	case 'f':
		node_flag_variables(node->data.function.child);
		break;

	case 'g':
		node_flag_variables(node->data.function2.left);
		node_flag_variables(node->data.function2.right);
		break;

	case 's':
		node_flag_variables(node->data.selection.condition);
		node_flag_variables(node->data.selection.left);
		node_flag_variables(node->data.selection.right);
		break;

	case 'u':
		node_flag_variables(node->data.un_op.child);
		break;

	case 'b':
		node_flag_variables(node->data.bin_op.left);
		node_flag_variables(node->data.bin_op.right);
		break;
	}
}

size_t
node_get_length(Node *node)
{
	size_t length;	/* Length of above string.  */
	Record *record;	/* Symbol table record.  */
	Node *child;	/* Function argument or operand.  */
	Node *left, *right;	/* Arguments or operands.  */
	Node *condition;	/* Selection condition.  */

	/* According to node type, calculate length of string representing
	   subtree rooted at node.  */
	switch (node->type) {
	case 'n':
		length = snprintf(NULL, 0, "%g", node->data.number);
		if (node->data.number < 0)
			length += 2;
		return length;

	case 'c':
		return strlen(node->data.constant->name);

	case 'v':
		return strlen(node->data.variable->name);

	case 'f':
		record = node->data.function.record;
		child = node->data.function.child;
		return strlen(record->name) + 1 + node_get_length(child) +
		    1;
		break;

	case 'g':
		record = node->data.function2.record;
		left = node->data.function2.left;
		right = node->data.function2.right;
		return strlen(record->name) + 1 + node_get_length(left) +
		    1 + node_get_length(right) + 1;

	case 's':
		record = node->data.selection.record;
		condition = node->data.selection.condition;
		left = node->data.selection.left;
		right = node->data.selection.right;
		return strlen(record->name) + 1 +
		    node_get_length(condition)
		    + 1 + node_get_length(left) + 1 +
		    node_get_length(right) + 1;

	case 'u':
		child = node->data.un_op.child;
		return 1 + 1 + node_get_length(child) + 1;

	case 'b':
		left = node->data.bin_op.left;
		right = node->data.bin_op.right;
		return 1 + node_get_length(left) + 1 +
		    node_get_length(right) + 1;
	}

	return 0;
}

void
node_write(Node *node, char *string)
{
	Record *record;	/* Symbol table record.  */
	Node *child;	/* Function argument or operand.  */
	Node *left, *right;	/* Arguments or operands.  */
	Node *condition;	/* Selection condition.  */

	/* According to node type, write subtree rooted at node to node
	   string variable.  Always use parenthesis to resolve operation
	   precedence.  */
	switch (node->type) {
	case 'n':
		if (node->data.number < 0) {
			sprintf(string, "%c", '(');
			string += strlen(string);
		}
		sprintf(string, "%g", node->data.number);
		string += strlen(string);
		if (node->data.number < 0)
			sprintf(string, "%c", ')');
		break;

	case 'c':
		sprintf(string, "%s", node->data.constant->name);
		break;

	case 'v':
		sprintf(string, "%s", node->data.variable->name);
		break;

	case 'f':
		record = node->data.function.record;
		child = node->data.function.child;
		sprintf(string, "%s%c", record->name, '(');
		string += strlen(string);
		node_write(child, string);
		string += strlen(string);
		sprintf(string, "%c", ')');
		break;

	case 'g':
		record = node->data.function2.record;
		left = node->data.function2.left;
		right = node->data.function2.right;
		sprintf(string, "%s%c", record->name, '(');
		string += strlen(string);
		node_write(left, string);
		string += strlen(string);
		sprintf(string, "%c", ',');
		string += strlen(string);
		node_write(right, string);
		string += strlen(string);
		sprintf(string, "%c", ')');
		break;

	case 's':
		record = node->data.selection.record;
		condition = node->data.selection.condition;
		left = node->data.selection.left;
		right = node->data.selection.right;
		sprintf(string, "%s%c", record->name, '(');
		string += strlen(string);
		node_write(condition, string);
		string += strlen(string);
		sprintf(string, "%c", ',');
		string += strlen(string);
		node_write(left, string);
		string += strlen(string);
		sprintf(string, "%c", ',');
		string += strlen(string);
		node_write(right, string);
		string += strlen(string);
		sprintf(string, "%c", ')');
		break;

	case 'u':
		child = node->data.un_op.child;
		sprintf(string, "%c", '(');
		string += strlen(string);
		sprintf(string, "%c", node->data.un_op.operation);
		string += strlen(string);
		node_write(child, string);
		string += strlen(string);
		sprintf(string, "%c", ')');
		break;

	case 'b':
		left = node->data.bin_op.left;
		right = node->data.bin_op.right;
		sprintf(string, "%c", '(');
		string += strlen(string);
		node_write(left, string);
		string += strlen(string);
		sprintf(string, "%c", node->data.bin_op.operation);
		string += strlen(string);
		node_write(right, string);
		string += strlen(string);
		sprintf(string, "%c", ')');
		break;
	}
}
