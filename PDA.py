#!/usr/bin/env python3

# PDA, an implementation of a nondeterministic push down automata based on
# backtracking
#
# Copyright (c) Ralf Ramsauer, 2017
#
# Authors:
#  Ralf Ramsauer <ralf.ramsauer@oth-regensburg.de>
#
# This work is licensed under the terms of the GNU GPL, version 2. See the
# COPYING file in the top-level directory.

# production rules can be implemented very easily as a dictionary that
# associates nonterminal symbols (left side of a production) the set of
# available productions.
#
# Python allows us to write those rules almost in the same way, as we defined
# them in our lecture. We follow the convention that capital letters are used
# for nonterminal symbols, terminal symbols are written in lowercase.
#
# These are the production rules for the contextfree language a^n b^m c^m.
# Grammar:
#
# S -> AB
# A -> aA | a
# B -> bBc | bc
P = { 'S': {'AB'},
      'A': {'aA', 'a'},
      'B': {'bBc', 'bc'}}

# This is our PDA. Note that we don't pass the state 'q' as argument, as the
# PDA doesn't need a state if we want our PDA to decide a cf grammar.
#
# So the automata only takes a word and the content of the stack, and returns
# either True or False, iow word is element of the language or not.
def run_pda(word, stack):
    print('%s %s' % (word, stack))

    # if the stack is empty then we have two cases:
    #  - word is empty     -> Yay, we can accept the word, as this is the
    #                         criterion of acceptance of the PDA
    #
    #  - word is not empty -> What a bummer, we ran into an error case.
    if len(stack) == 0:
        return len(word) == 0

    # Now we have two cases left. The automata may either do an
    # epsilon-transition, or consume a symbol from the input tape.
    # In either case, we have to pop the uppermost element from the stack.
    top_stack = stack[0]
    stack = stack[1:]

    # The uppermost element of the stack can either be a capital letter (=^
    # nonterminal symbol) or a lowercase letter (=^ terminal symbol).
    #
    # If we hit a nonterminal symbol, try to apply every available
    # production rule for the symbol, and recursively run the PDA again.
    if top_stack.isupper():
        for production in P[top_stack]:
            if run_pda(word, production + stack):
                return True
        # Dead end, no rule was successful.
        return False

    # Here we land if the uppermost element of our stack is a terminal symbol.
    # But hey, we first have to check if we have anything left to chomp on the
    # input tape. If not, dead end.
    if len(word) == 0:
        return False

    # Pop the uppermost element from the input tape
    top_word = word[0]
    word = word[1:]

    # Can we consume it? IOW, is it the same as the uppermost element of the
    # stack? If yes, consume it and run the PDA on the rest of the word and the
    # new stack. If not, dead end.
    if (top_word == top_stack):
        return run_pda(word, stack)

    return False


# And here we start our PDA with some input word, the initial content of the
# stack is our start symbol S.
print(run_pda('aabbbccc', 'S'))

# Let's go a bit further, and check if the word a^100 b^23 c^23 is element
# of the grammar... Play around with the length of the words, your stack will
# blow up at some point.

# word = 'a' * 100 + 'b' * 50 + 'c' * 50
# print(run_pda(word, 'S'))
