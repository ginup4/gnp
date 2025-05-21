#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "ast.h"
#include "lines.h"
#include "error.h"
#include "analyzer.h"
#include "generator.h"

extern int yyparse();
extern FILE *yyin;

void usage(char *progname) {
    fprintf(stderr, "Usage: %s [INPUT]", progname);
    exit(EXIT_FAILURE);
}

filename init_file = { NULL, NULL, 0, { 0, 0, 0, 0, NULL} };

int main(int argc, char **argv) {
    if(argc > 1) {
        yyin = fopen(argv[1], "r");
        if(!yyin) {
            perror(argv[1]);
            return EXIT_FAILURE;
        }
        init_file.name = argv[1];
    } else {
        yyin = stdin;
        init_file.name = "<stdin>";
    }
    first_file = current_file = last_file = &init_file;

    if(yyparse()) {
        return EXIT_FAILURE;
    }

    if(errors) {
        return EXIT_FAILURE;
    }

    analyze_ast(&glob_program);

    if(errors) {
        return EXIT_FAILURE;
    }

    //FILE *file = fopen("/tmp/gnp_asm_out.s", "w");
    //if(!file) {
    //    panic(strerror(errno));
    //}

    generate(stdout, &glob_program);

    //if(fclose(file)) {
    //    panic(strerror(errno));
    //}

    //system("cat /tmp/gnp_asm_out.s"); // temp

    //if(system("as -o /tmp/gnp_obj_out.o /tmp/gnp_asm_out.s")) {
    //    return EXIT_FAILURE;
    //}

    //if(system("ld -o out_gnp_prog -dynamic-linker /lib/ld-linux-x86-64.so.2 /usr/lib/crt1.o /usr/lib/crti.o -lc /tmp/gnp_obj_out.o /home/ginup4/Programming/c/gnp/out/prelude.o /usr/lib/crtn.o")) {
    //    return EXIT_FAILURE;
    //}

    return EXIT_SUCCESS;
}
