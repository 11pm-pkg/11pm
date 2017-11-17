#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


typedef int (*inline_func_t)(char **args);
extern inline_func_t inlines[];
int n_inlines;

int statuspipe;


struct t_getopt_s {
    int argc;
    char **argv;
    int argprog;

    char option;
    char *value;
    int used_value;
};
static void t_getopt_init(struct t_getopt_s *state, int argc, char **argv);
static int t_getopt_next(struct t_getopt_s *state);


static int t_assert(int argc, char **argv);
static int t_eval(int argc, char **argv);
static int t_test(int argc, char **argv);


int main(int argc, char **argv)
{
    statuspipe = open(".tstatus", O_WRONLY | O_CLOEXEC);
    for (int i = 0; i < argc; i += 1) {
        dprintf(statuspipe, "\"%s\" ", argv[i]);
    }
    dprintf(statuspipe, "\n");

    // TODO: what path to take?
}


/* Implementation of `t assert`.
 * Writes an assertion result to test pipe.
 * Optionally also exits with the relevant status.
 */
static int t_assert(int argc, char **argv)
{
    struct t_getopt_s go_state;
    t_getopt_init(&go_state, argc, argv);
    while (t_getopt_next(&go_state)) {
        switch (go_state.option) {
        case '\0':     // An argument with no option
            if (go_state.value[0] == '[') {
                t_test(go_state.argc, go_state.argv);
            } else {
                errx(1, "assert: invalid value %s", go_state.value);
            }
            break;
        case '?':
            go_state.used_value = 1;
            if (strcmp(go_state.value, "-") == 0) {
                // t exit code to match assert result
            } else {
                // Assert that exit code matches exec result
                int x = strtol(go_state.value, NULL, 0);
            }
            break;
        case '-':
            // This assertion is on the exit code of a program
            break;
        case 'a':
            // Next option is related to lhs of test
            break;
        case 'b':
            // Next option is related to rhs of test
            break;
        case 'm':
            // Next option is related to entire assertion message
            break;
        case 'x':
            // Replaces assertion message
            break;
        default:
            // Option not supported
            errx(1, "assert: invalid option -%c", go_state.option);
        }
    }
}


/* Implemenation of `t eval`.
 * Allows calling of "inline" C.
 * If suffixed with `: assert ...`, also allows
 * performing assertions on the result.
 */
static int t_eval(int argc, char **argv)
{
    // Get len(inlines)
    for (n_inlines = 0; inlines[n_inlines]; n_inlines += 1) {}
}


/* Implementation of `[`.
 * Should be largely compatible with bash's `test`.
 * Also supports using something like `[- op ... ]`
 * to test against stdin.
 */
static int t_test(int argc, char **argv)
{
    int real_argc = -1;
    for (int i = 0; i < argc; i += 1) {
        if (strcmp(argv[i], "]") == 0 ||
                strcmp(argv[i], "<]") == 0) {
            real_argc = i;  // Not subtracting 1 excludes the ],
                            // which is what we want.
            break;
        } else if (strcmp(argv[i], "-]") == 0) {
            errx(2, "[: cannot use stdin for rhs");
        }
    }

    if (real_argc == -1) {
        errx(2, "[: no matching ] or <]");
    }

    char *lhs, *rhs;
    enum { ISFILE_NO = 0, ISFILE_STDIN, ISFILE_PATH }
        lhs_isfile, rhs_isfile;
    int argc_mod = 0;

    if (strcmp(argv[0], "[-") == 0) {
        lhs_isfile = ISFILE_STDIN;
        argc_mod = 1;
    } else if (strcmp(argv[0], "[<") == 0) {
        lhs_isfile = ISFILE_PATH;
    }

    if (real_argc == 2 - argc_mod) {
        // This is a unary operator
        if (argv[1][0] != '-')
            errx(2, "[: unary operator must start with -");
    } else if (real_argc == 3 - argc_mod) {
        // This is a binary operator

        if (!argc_mod) {
            lhs = argv[1];
        }

        rhs = argv[3 - argc_mod];
        if (strcmp(argv[real_argc], "<]") == 0) {
            rhs_isfile = ISFILE_PATH;
        }

        op = argv[2 - argc_mod];
        if (op[0] == '-') {
            // Numeric comparison
            if (lhs_isfile)
                errx(2, "[: lhs is file content in numeric comparison");
            if (rhs_isfile)
                errx(2, "[: rhs is file content in numeric comparison");
            
            if (lhs[0] == '\0')
                errx(2, "[: lhs is empty");
            if (rhs[0] == '\0')
                errx(2, "[: rhs is empty");
            
            char *end;
            long lhs_i = strtol(lhs, &end, 10);
            if (end[0] != '\0')
                errx(2, "[: lhs '%s' is not a valid integer", lhs);
            
            long rhs_i = strtol(rhs, &end, 10);
            if (end[0] != '\0')
                errx(2, "[: rhs '%s' is not a valid integer", rhs);
            
            if (strcmp(op, "-eq") == 0) {

            } else if (strcmp(op, "-ne") == 0) {

            } else if (strcmp(op, "-lt") == 0) {

            } else if (strcmp(op, "-le") == 0) {

            } else if (strcmp(op, "-gt") == 0) {

            } else if (strcmp(op, "-ge") == 0) {

            }
        } else {
            // String comparison
        }
    } else {
        // Don't have an operator for this many args :(
        
        if (real_argc < 2 - argc_mod)
            errx(2, "[: too few arguments");
        else
            errx(2, "[: too many arguments");
    }
}


static void t_getopt_init(struct t_getopt_s *state, int argc, char **argv)
{
    state->argc = argc;
    state->argv = argv;
    state->argprog = 0;
    state->used_value = 0;
}

static int t_getopt_next(struct t_getopt_s *state)
{
    if (state->argc == 0) {
        return 0;
    }

    if (state->used_value) {
        state->argc -= 1;
        state->argv += 1;
        state->argprog = 0;
        state->option = '\0';
        state->used_value = 0;
    }

    if (state->argprog == 0) {
        if (state->argv[0][0] == '-') {
            state->argprog += 1;
        } else {
            state->value = state->argv[0];
            return 1;
        }
    }

    state->option = state->argv[0][state->argprog++];
    state->value = state->argv[0] + state->argprog;
    return 1;
}
