#include "ls-stage2.h"
#include <strings.h>    // strcasecmp

int notDotFile(const struct dirent * dent) {
    return dent->d_name[0] != '.';
}

int ascComp(const struct dirent ** dentA,
        const struct dirent ** dentB) {
    return strcasecmp((*dentA)->d_name, (*dentB)->d_name);
}

int descComp(const struct dirent ** dentA,
        const struct dirent ** dentB) {
    return strcasecmp((*dentB)->d_name, (*dentA)->d_name);
}
