#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "ast.h"
#include "lines.h"
#include "error.h"
#include "analyzer.h"

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
        printf("not continuing to codegen\n");
        return EXIT_FAILURE;
    }

    printf("codegen lol\n");

    return EXIT_SUCCESS;
}
