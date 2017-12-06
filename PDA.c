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
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define NUM_NONTERMS ('Z' - 'A' + 1)

typedef const char ** rule;

#define DEFINE_GRAMMAR(name) \
	rule name[NUM_NONTERMS]

#define RULE(NTERM, ...) \
	[ NTERM - 'A' ] = (const char *[]){__VA_ARGS__, NULL}

struct stack {
	char content[1024];
	unsigned int top;
};

#define for_each_production(GRAMMAR, NTERM, RULE) \
	for (RULE = GRAMMAR[NTERM - 'A']; RULE && *RULE; RULE++)

const static DEFINE_GRAMMAR(wtf) = {
	RULE('S', "AB"),
	RULE('A', "aA", "a"),
	RULE('B', "bBc", "bc"),
};

static void dump_grammar(const rule *g)
{
	unsigned int i;
	const char **right;

	for (i = 0; right = g[i]; i++)
		while (*right)
			printf("%c -> %s\n", 'A' + i, *right++);
}

static bool run_pda(const rule *g, const char *word, struct stack s)
{
	char top, top_input;
	int i;
	rule r;

	printf("Word: %s\t\t Stack:", word);
	for (i = s.top - 1; i >= 0; i--)
		printf("%c", s.content[i]);
	printf("\n");

	if (s.top == 0)
		return strlen(word) == 0;

	top = s.content[--s.top]; // POP

	if (isupper(top)) {
		for_each_production(g, top, r) {
			struct stack tmp = s;
			const char *rule = *r;
			const int rule_len = strlen(rule);
			char *back = tmp.content + tmp.top + rule_len - 1;

			while (*rule)
				*back-- = *rule++;
			tmp.top += rule_len;

			if (run_pda(g, word, tmp))
				return true;
		}
		return false;
	}

	if (strlen(word) == 0)
		return false;

	top_input = word[0];
	word++;

	if (top_input == top)
		return run_pda(g, word, s);

	return false;
}

int main(int argc, char **argv)
{
	bool ret;
	struct stack s = {
		.top = 1,
		.content = { 'S' },
	};

	if (argc != 2) {
		fprintf(stderr, "Usage: %s word\n", argv[0]);
		return -1;
	}

	dump_grammar(wtf);

	ret = run_pda(wtf, argv[1], s);
	printf("%s\n", ret ? "Yep" : "Nay");

	return !ret;
}
