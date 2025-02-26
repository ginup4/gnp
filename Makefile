cflags := -Wall -Wextra -Wno-unused-parameter
ldflags :=

gnpc: build/scanner.o build/parser.o build/main.o build/ast.o build/lines.o build/error.o build/analyzer.o
	gcc $(ldflags) -o $@ $^

build/main.o: main.c ast.h print_ast.c lines.h error.h analyzer.h
	gcc $(cflags) -c -o $@ $<

build/ast.o: ast.c ast.h
	gcc $(cflags) -c -o $@ $<

build/analyzer.o: analyzer.c analyzer.h ast.h
	gcc $(cflags) -c -o $@ $<

build/lines.o: lines.c lines.h
	gcc $(cflags) -c -o $@ $<

build/error.o: error.c error.h lines.h
	gcc $(cflags) -c -o $@ $<

build/scanner.o: build/scanner.c build/parser.c ast.h lines.h
	gcc -c -o $@ $<

build/parser.o: build/parser.c ast.h error.h
	gcc -c -o $@ $<

build/scanner.c: scanner.l
	flex -s -o $@ $<

build/parser.c: parser.y
	bison --header=build/parser.h -o $@ $<

