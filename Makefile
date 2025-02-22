ginuc: build/scanner.o build/parser.o build/main.o build/ast.o
	gcc -o ginuc build/scanner.o build/parser.o build/main.o build/ast.o

build/main.o: main.c ast.h
	gcc -c -o build/main.o main.c

build/ast.o: ast.c ast.h
	gcc -c -o build/ast.o ast.c

build/scanner.o: build/scanner.c build/parser.h ast.h
	gcc -c -o build/scanner.o build/scanner.c

build/parser.o: build/parser.c ast.h
	gcc -c -o build/parser.o build/parser.c

build/scanner.c: scanner.l
	flex -s -o build/scanner.c scanner.l

build/parser.c build/parser.h: parser.y
	bison --header=build/parser.h -o build/parser.c parser.y

