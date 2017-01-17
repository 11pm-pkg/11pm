#include <string.h>

#include "util.h"


char *relativepath(const char const *path, const char const *base, char *out)
{
    int ci = 0;
    int oi = 0;

    // Find where paths diverge
    int sepi = 0;
    for (; path[ci] == base[ci]; ++ci) {
        if (!path[ci]) {
            out[oi] = 0;
            return out;
        }

        if (path[ci] == '/') {
            sepi = ci;
        }
    }
    if (base[ci] == 0 && path[ci] == '/') {
        sepi = ci;
    }

    ci = sepi + 1;
    int start = ci;

    // Add ../ for each extra directory down base
    for (; base[ci]; ++ci) {
        if (base[ci] == '/') {
            strcpy(out + oi, "../");
            oi += 3;
        }
    }

    // Add diverged part of path
    strcpy(out + oi, path + start);
    return out;
}
