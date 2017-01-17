/* install.c - functions related to symlinking
 * (installing) already faked packages */

#include <sys/types.h>
#include <sys/stat.h>

#include <fts.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "util.h"
#include "hpm.h"


int hpm_symlink(char *from, char *to)
{
    char *args[] = { from, NULL };
    size_t fromlen = strlen(from);
    size_t tolen = strlen(to);
    if (tolen > 0 && to[tolen-1] != '/') {
        return errno = EINVAL;
    }

    FTS *fts = fts_open(args, FTS_PHYSICAL | FTS_NOCHDIR, NULL);
    FTSENT *f;
    while (f = fts_read(fts)) {
        if (f->fts_level <= 0) continue;
        if (f->fts_info == FTS_D) {
            char newdir[tolen + (f->fts_pathlen - fromlen)];
            strcpy(newdir, to);
            strcpy(newdir+tolen, f->fts_path + fromlen + 1);
            printf("mkdir %s...", newdir);
            if (mkdir(newdir, 0777)) {
                int errsv = errno;
                printf(" %s\n", strerror(errno));
                errno = errsv;
                break;
            } else {
                printf(" OK\n");
            }
        } else if (f->fts_info == FTS_F) {
            char newfile[tolen + (f->fts_pathlen - fromlen)];
            strcpy(newfile, to);
            strcpy(newfile+tolen, f->fts_path + fromlen + 1);

            char lntarget[strlen(newfile) + (int)(f->fts_pathlen * 1.4)];
            relativepath(f->fts_path, newfile, lntarget);

            printf("ln %s -> %s (%s)...", newfile, lntarget, f->fts_path);
            if (symlink(lntarget, newfile)) {
                int errsv = errno;
                printf(" %s\n", strerror(errno));
                errno = errsv;
                break;
            } else {
                printf(" OK\n");
            }
        }
    }

    int errsv = errno;
    fts_close(fts);

    return errsv;
}
