.PHONY: all test clean editor client common server build \
	format format-check cpplint cppcheck lint valgrind

# Código propio: se excluye el código de terceros de la cátedra (common/net, common/lib).
SOURCES := $(shell git ls-files --cached --others --exclude-standard '*.cpp' '*.h' \
	| grep -vE '^common/(net|lib)/')

# Mismos flags que el .pre-commit-config.yaml de la cátedra, más googletest para entender los TEST_F.
CPPCHECK_FLAGS := --quiet --inline-suppr --enable=all \
	--suppress=missingIncludeSystem --suppress=missingInclude \
	--suppress=unusedFunction --suppress=unusedStructMember \
	--suppress=unmatchedSuppression --suppress=checkersReport \
	--library=googletest --language=c++ --std=c++20 --error-exitcode=1 -I .

VALGRIND_FLAGS := --quiet --leak-check=full --errors-for-leak-kinds=definite --error-exitcode=1

compile-debug:
	mkdir -p build/
	cmake -S . -B ./build -DCMAKE_BUILD_TYPE=Debug $(EXTRA_GENERATE)
	cmake --build  build/ $(EXTRA_COMPILE)

run-tests: compile-debug
	./build/taller_tests

valgrind: compile-debug
	valgrind $(VALGRIND_FLAGS) ./build/taller_tests --gtest_brief=1

all: clean run-tests

clean:
	rm -Rf build/

# Reformatea in place: usar antes de commitear, nunca en CI.
format:
	clang-format --style=file -i $(SOURCES)

format-check:
	clang-format --style=file --dry-run --Werror $(SOURCES)

cpplint:
	cpplint --quiet $(SOURCES)

cppcheck:
	cppcheck $(CPPCHECK_FLAGS) $(SOURCES)

lint: format-check cpplint cppcheck
