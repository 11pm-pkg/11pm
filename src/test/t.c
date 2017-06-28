#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef int (*inline_func_t)(char **args);
extern inline_func_t inlines[];
int n_inlines;


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
