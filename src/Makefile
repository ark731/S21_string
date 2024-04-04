#  ///////////////////  VARIABLES  ////////////////////
NAME := s21_string.a
EXECNAME := 

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Werror
GCOVFLAGS := -fprofile-arcs -ftest-coverage -g
LDFLAGS := -lm -lcheck
SOURCES := $(wildcard s21*.c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))

UNAME := $(shell uname -s)

#  ////////////////////  OS CHECK  ////////////////////
ifeq ($(UNAME),Linux)
	LDFLAGS += -lrt -lpthread -lsubunit
	OPEN=xdg-open
endif
ifeq ($(UNAME),Darwin)
	OPEN=open
endif

#  ////////////////////  TARGETS  /////////////////////
all: $(NAME)

$(NAME): $(OBJECTS)
	ar rcs $(NAME) $(OBJECTS)
	rm -rf *.o
	ranlib $(NAME)

%.o: %.c
	@$(CC) $(CFLAGS) -c $^ -o $@

test: unittests $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) unittests.c $(LDFLAGS) -o unittests.out
	rm -rf *.o
	./unittests.out

valgrind: unittests
	$(CC) $(CFLAGS) -g $(SOURCES) unittests.c $(LDFLAGS) -o unittests.out
	rm -rf *.o
	valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes ./unittests.out

gcov_report: unittests
	@$(CC) $(GCOVFLAGS) $(SOURCES) unittests.c $(LDFLAGS) -o gcov_report.out
	@./gcov_report.out
	@gcov -a -c -f $(SOURCES)
	@lcov --quiet -t "gcov_report" -o Coverage_Report.info -c -d ./
	@lcov --quiet -r Coverage_Report.info '*/unittests.c' -o Coverage_Report.info
	@genhtml -q -s -o ./report Coverage_Report.info
	rm -rf *.o *.a *.gcno *.gcda *.info *.out *.gcov
	@$(OPEN) ./report/index-sort-f.html

rebuild: clean all

clean:
	rm -rf *.o *.a *.out *.gcov *.gcda *.gcno *.info report unittests.c unit_tests/_unittests.check

#  ////////////////////  INCLUDE  //////////////////////
include makefiles/check.mk

#  /////////////////////  EXTRAS  //////////////////////
.PHONY: all test clean rebuild gcov_report clang-format

clang-format:
	clang-format -i -style=Google *.h *.c