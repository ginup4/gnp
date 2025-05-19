cflags := -Wall -Wextra -Wshadow -Wno-unused-parameter -Wno-unused-function -g -O0 -fsanitize=address,undefined
#cflags := -O3
ldflags := -lasan -lubsan
#ldflags :=

.PHONY: make
make: build out out/gnpc out/prelude.o

build:
	mkdir build

out:
	mkdir out

out/prelude.o: prelude.s
	as -o $@ $<

out/gnpc: build/scanner.o build/parser.o build/main.o build/ast.o build/lines.o build/error.o build/analyzer.o build/type_checker.o build/generator.o
	gcc $(ldflags) -o $@ $^

build/main.o: main.c ast.h print_ast.c lines.h error.h analyzer.h
	gcc $(cflags) -c -o $@ $<

build/ast.o: ast.c ast.h
	gcc $(cflags) -c -o $@ $<

build/analyzer.o: analyzer.c analyzer.h ast.h error.h type_checker.h
	gcc $(cflags) -c -o $@ $<

build/type_checker.o: type_checker.c ast.h error.h
	gcc $(cflags) -c -o $@ $<

build/generator.o: generator.c ast.h
	gcc $(cflags) -c -o $@ $<

build/lines.o: lines.c lines.h
	gcc $(cflags) -c -o $@ $<

build/error.o: error.c error.h lines.h
	gcc $(cflags) -c -o $@ $<

build/scanner.o: build/scanner.c build/parser.c ast.h lines.h
	gcc $(cflags) -c -o $@ $<

build/parser.o: build/parser.c ast.h error.h
	gcc $(cflags) -c -o $@ $<

build/scanner.c: scanner.l
	flex -s -o $@ $<

build/parser.c: parser.y
	bison --header=build/parser.h -o $@ $<

