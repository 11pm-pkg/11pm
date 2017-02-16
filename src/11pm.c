#include <stdio.h>
#include <errno.h>
#include <err.h>

#include "11pm.h"
#include "util.h"


int main(int argc, char **argv)
{
    if (argc < 3) {
        errx(1, "path arguments required");
    }

    errno = xipm_symlink(argv[1], argv[2]);
    if (errno)
        err(errno, NULL);
    return 0;
}
