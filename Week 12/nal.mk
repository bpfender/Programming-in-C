# A single file (nal.c) is used to build both the parser & interpreter
CFLAGS = -Wall -Wextra -pedantic -ansi -O2 -lm
SFLAGS = -fsanitize=address -fsanitize=undefined -g3 -pedantic -ansi -Wall -Wextra -lm
DFLAGS = -Wall -Wextra -pedantic -ansi -g3 -lm
CC = clang

BASE = nal
TBASE = test
PARSEF = tokenizer.c symbols.c parser.c error.c
PARSED = $(PARSEF) symbols.h parser.c parser.h error.c error.h
INTERPF = $(PARSEF) interpreter.c
INTERPD = $(PARSED) interpreter.c interpreter.h
EXTF = $(PARSEF)
EXTD = $(PARSED)

BEXECS = parse interp extension
SEXECS = parse_s interp_s extension_s
DEXECS = parse_d interp_d extension_d
TEXECS = test test_s test_d

EXECS = $(BEXECS) $(SEXECS) $(DEXECS) $(TEXECS)

all: $(BEXECS)

# PARSE RULES #
parse : $(BASE).c $(PARSED)
	$(CC) $(BASE).c $(PARSEF) $(CFLAGS) -o $@
parse_s : $(BASE).c $(PARSED)
	$(CC) $(BASE).c $(PARSEF) $(SFLAGS) -o $@
parse_d : $(BASE).c $(PARSED)
	$(CC) $(BASE).c $(PARSEF) $(DFLAGS) -o $@

# INTERP RULES #
interp : $(BASE).c $(INTERPD)
	$(CC) $(BASE).c $(INTERPF) $(CFLAGS) -o $@ -DINTERP
interp_s : $(BASE).c $(INTERPD)
	$(CC) $(BASE).c $(INTERPF) $(SFLAGS) -o $@ -DINTERP
interp_d : $(BASE).c $(INTERPD)
	$(CC) $(BASE).c $(INTERPF) $(DFLAGS) -o $@ -DINTERP

# EXTENSION RULES #
extension: $(BASE).c $(EXTD)
	$(CC) $(BASE).c $(EXTF) $(CFLAGS) -o $@ -DEXTENSION

extension_s: $(BASE).c $(EXTD)
	$(CC) $(BASE).c $(EXTF) $(SFLAGS) -o $@ -DEXTENSION

extension_d: $(BASE).c $(EXTD)
	$(CC) $(BASE).c $(EXTF) $(DFLAGS) -o $@ -DEXTENSION

# TESTFILE RULES #
testbase: $(TBASE).c $(INTERPD)
	$(CC) $(TBASE).c $(INTERPF) $(CFLAGS) -o $@

testbase_s: $(TBASE).c $(INTERPD)
	$(CC) $(TBASE).c $(INTERPF) $(SFLAGS) -o $@

testbase_d: $(TBASE).c $(INTERPD)
	$(CC) $(TBASE).c $(INTERPF) $(DFLAGS) -o $@


# CALL TESTING #
test: testfile testparse testinterp

testfile: testbase_s testbase_d
	./testbase_s
	valgrind ./testbase_d

testparse : parse_s parse_d
	./parse_s test1.$(BASE)
	./parse_s test2.$(BASE)
	./parse_s test3.$(BASE)
	./parse_s test4.$(BASE)
	./parse_s test5.$(BASE)
	./parse_s test6.$(BASE)
	valgrind ./parse_d test1.$(BASE)
	valgrind ./parse_d test2.$(BASE)
	valgrind ./parse_d test3.$(BASE)
	valgrind ./parse_d test4.$(BASE)
	valgrind ./parse_d test5.$(BASE)
	valgrind ./parse_d test6.$(BASE)

testinterp : interp_s interp_d
	./interp_s test1.$(BASE)
	./interp_s test2.$(BASE)
	./interp_s test4.$(BASE)
	./interp_s test5.$(BASE)
	valgrind ./interp_d test1.$(BASE)
	valgrind ./interp_d test2.$(BASE)
	valgrind ./interp_d test4.$(BASE)
	valgrind ./interp_d test5.$(BASE)

clean :
	rm -fr $(EXECS)
