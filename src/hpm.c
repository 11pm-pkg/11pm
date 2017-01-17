#include <stdio.h>
#include <errno.h>
#include <err.h>

#include "hpm.h"
#include "util.h"


int main(int argc, char **argv)
{
    if (argc < 3) {
        errx(1, "path arguments required");
    }

    errno = hpm_symlink(argv[1], argv[2]);
    if (errno)
        err(errno, NULL);
    return 0;
}
