/*
 * An implementation of a simplified push down automaton (PDA)
 *
 * Copyright (c) Ralf Ramsauer, 2017
 *
 * Authors:
 *   Ralf Ramsauer <ralf.ramsauer@oth-regensburg.de>
 *
 * This work is licensed under the terms of the GNU GPL, version 2. See the
 * COPYING file in the top-level directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/* We have 26 possible nonterminal symbols */
#define NUM_NONTERMS ('Z' - 'A' + 1)

/* We substitute char*** as grammar.  This is easier to understand (and to use)
 * as pointer to a pointer to a pointer of chars.
 */
typedef const char ** rule;
typedef const rule * grammar;

#define DEFINE_GRAMMAR(name) \
	rule name[NUM_NONTERMS]

/* This definition avoids redundant copy-pasting. A NULL marks the end of the
 * rule array
 */
#define RULE(NTERM, ...) \
	[ NTERM - 'A' ] = (const char *[]){__VA_ARGS__, NULL}

/* Define a nice for_each loop
 * It basicaly checks wether we have a actually defined nonterminal NTERM
 * (RULE) and that we do actually have a rule in there, and not only our end
 * marker NULL (*RULE)
 */
#define for_each_production(GRAMMAR, NTERM, RULE) \
	for (RULE = GRAMMAR[NTERM - 'A']; RULE && *RULE; RULE++)

/* struct stack can currently hold up to 1024 elements.
 * Should be enough for a simple grammar.  Note that we currenty do _not_ have
 * boundary checks.
 */
struct stack {
	char content[1024];
	unsigned int top;
};

const static DEFINE_GRAMMAR(wtf) = {
	RULE('S', "AB"),
	RULE('A', "aA", "a"),
	RULE('B', "bBc", "bc"),
};

static void dump_grammar(grammar g)
{
	unsigned int i;
	rule right;

	/* We iterate over every possible nonterminal symol */
	for (i = 0; i < NUM_NONTERMS; i++) {
		/* We assigning the array of rules for the non-terminal i to
		 * the rule "right". The if-condition then checks wether this
		 * rule is actually present. If we didn't define any rule, g[i]
		 * would be NULL, and we simply skip it. */
		if (right = g[i]) {
			/* right points to an array holding all rules of the
			 * nonterminal symbol.  If this *right is NULL, we have
			 * reached the end of the list of rules.
			 * (cf. #define RULE(...) { __VA_ARGS__, NULL })
			 *					 ^^^^
			 */
			while (*right) {
				/* Print the content of the current rule.
				 * We're interested in the destination of the
				 * production, so dereference. After printing,
				 * we increment our pointer to the next
				 * possible element and we check again.
				 */
				printf("%c -> %s\n", 'A' + i, *right++);
			}
		}
	}
}

static bool run_pda(grammar g, const char *word, struct stack stack)
{
	char top_stack, top_input;
	int i;
	rule rule_pointer;

	/* Show the grammar that we use */
	printf("Word: %s\t\t Stack: ", word);
	/* The uppermost (last element in the array) must come first.  So read
	 * the array in reverse order.
	 */
	for (i = stack.top - 1; i >= 0; i--)
		printf("%c", stack.content[i]);
	printf("\n");

	/* If our stack is empty, we still might have characters left in our
	 * word. If so, the run of our PDA was not successful. Otherwise, it
	 * was successful.
	 */
	if (stack.top == 0)
		return strlen(word) == 0;

	/* Pop the uppermost element from our stack.
	 * First we decrease stack.top to point to the current uppermost
	 * element, because stack.top points to the next free slot.  Thus we
	 * can get the uppermost element while maintaining the stack's
	 * position.
	 */
	top_stack = stack.content[--stack.top]; // POP

	/* Check if we have a nonterminal character on our stack */
	if (isupper(top_stack)) {
		/* Iterate over each available production rule for the
		 * non-terminal 'top_stack'
		 */
		for_each_production(g, top_stack, rule_pointer) {
			/* Make a copy of our stack so it doesn't get modified
			 * when we recursively call ourself.
			 */
			struct stack tmp = stack;
			const char *rule = *rule_pointer;
			const int rule_len = strlen(rule);

			/* back points to the highest element in our stack
			 * after we push the rule contents on it. */
			char *back = tmp.content + tmp.top + rule_len - 1;

			/* Iterate over every character of the current rule */
			while (*rule) {
				/* And copy it over to our stack. We do start
				 * from the end, thus making the first element
				 * of our rule the uppermost.
				 */
				*back-- = *rule++;
			}

			/* Our stack just grew by rule_len elements. Maintain
			 * its size.
			 */
			tmp.top += rule_len;

			/* Recursively run the PDA with the new stack. Return
			 * if it was successful.
			 */
			if (run_pda(g, word, tmp))
				return true;
		}
		/* If no production led to a positive result, the overall
		 * result is false.
		 */
		return false;
	}

	/* Here we land if we have a terminal char on our stack. */

	/* Check if we actually have any chars left in our word. If no, the
	 * stack is not empty, and the PDA does not accept the word.
	 */
	if (strlen(word) == 0)
		return false;

	/* Get the next char from our word and move the read head one char to
	 * the right.
	 */
	top_input = word[0];
	word++;

	/* If the current char on the stack is equal to the left-most char of
	 * the word, run the PDA on the rest-word and the stack.
	 */
	if (top_input == top_stack)
		return run_pda(g, word, stack);

	/* Either the char on the stack and the next char are not equal or the
	 * PDA didn't recognize the word in any path.
	 */
	return false;
}

int main(int argc, char **argv)
{
	bool ret;

	/* Initially, our stack holds one element: the start symbol 'S' */
	struct stack s = {
		.top = 1,
		.content = { 'S' },
	};

	/* Check if we do have a single command line argument (2 because argc
	 * and argv are build like this: <program_name> <argument1> ...
	 */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s word\n", argv[0]);
		return -1;
	}

	dump_grammar(wtf);

	ret = run_pda(wtf, argv[1], s);
	printf("%s\n", ret ? "Yep" : "Nay");

	return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}
