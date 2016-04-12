#ifndef LSREPLICAHEADERFILE_H
#define LSREPLICAHEADERFILE_H

#include <stdbool.h>
#include <dirent.h>     // dirent
#include <sys/types.h>  // mode_t
#include <sys/stat.h>   // stat

typedef struct {
    bool l, n, i, r, S, a, h;
} Flags;

int getFlagsFromArgs(int argc, char* argv[], Flags* flagStruct);

int notDotFile(const struct dirent* dent);
int ascComp(const struct dirent** dentA,
        const struct dirent** dentB);
int descComp(const struct dirent** dentA,
        const struct dirent** dentB);
void sortBySize(struct stat* st, int fileCount);

void getPermissions (mode_t st_mode, char* s);
void getIDs(char* user, char* group, struct stat* st, bool asNumber);
void getFileSize(size_t st_size, char* s, bool humanReadable);
void getTime(time_t* tt, char* r);

#endif
