#include "ls-stage2.h"
#include <strings.h>    // strcasecmp
#include <stdlib.h>     // qsort

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

int sizeComp (const void * a, const void * b) {
    return ( ((struct stat*)b)->st_size - ((struct stat*)a)->st_size );
}

void sortBySize(struct stat* st, int fileCount) {
    qsort(st, fileCount, sizeof(struct stat), sizeComp);
}
