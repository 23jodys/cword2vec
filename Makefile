UNAME_S := $(shell uname -s)
HEADERS := $(wildcard *.h)
VPATH := src:include:tests:sds

ifeq ($(UNAME_S),Darwin)
GCOV := xcrun llvm-cov gcov
CFLAGS += -I/opt/homebrew/include
CFLAGS += -I/opt/homebrew/opt/libarchive/include
LDFLAGS += -L/opt/homebrew/lib
LDFLAGS += -I/opt/homebrew/opt/libarchive/lib
else
GCOV := llvm-cov gcov
endif

CC   := clang

CFLAGS += $(if $(COVERAGE), -fprofile-arcs -ftest-coverage )
CFLAGS += $(if $(DEBUG), -DDEBUG=1 )
CFLAGS += -Werror -Iinclude -Isds -g

LDLIBS += $(if $(or $(COVERAGE),$(DEBUG)), -g )
LDLIBS += $(if $(COVERAGE), --coverage ) 
LDLIBS += -larchive

test_cword2vec: LDLIBS += -lcmocka
test_cword2vec: cword2vec.o test_cword2vec.o sds.o

.PHONY: test
test: test_cword2vec
	./test_cword2vec 

cword2vec.o: cword2vec.h sds.o

cword2vec: cword2vec.o sds.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o cword2vec


valgrind_%: %
	valgrind --leak-check=full --error-exitcode=1 ./$* 

coverage: COVERAGE=1
coverage: test
	$(GCOV) $(SRCS)

TAGS: $(SRCS) include/cword2vec.h tests/test_*.[ch]
	ctags $^

docs: $(HEADERS)
	doxygen

.PHONY: clean
clean:
	rm -rf *.o *.gcda *.gcno test_cword2vec *.dSYM html/ latex/
