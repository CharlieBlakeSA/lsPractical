#include "ls-stage2.h"
#include <stdio.h>
#include <math.h>
#include <pwd.h>    // getpwuid
#include <grp.h>    // getgrgid
#include <time.h>   // localtime, strftime
#include <string.h> // strcpy, strcat
#include <stdlib.h> // malloc

void getPermissions (mode_t st_mode, char* perms) {
    perms[0] = S_ISDIR(st_mode) ? 'd' : '-';
    perms[1] = (st_mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (st_mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (st_mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (st_mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (st_mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (st_mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (st_mode & S_IROTH) ? 'r' : '-';
    perms[8] = (st_mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (st_mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';
}
void getIDs(char* user, char* group, struct stat* st, bool asNumber) {
    if (asNumber) {
        sprintf(user , "%ld ", (long) st->st_uid);
        sprintf(group, "%ld ", (long) st->st_gid);
    } else {
        struct passwd* pwd = getpwuid(st->st_uid);
        char* username = pwd ? pwd->pw_name : "?";
        sprintf(user, "%s ", username);

        struct group* grp = getgrgid(st->st_gid);
        char* groupname = grp ? grp->gr_name : "?";
        sprintf(group, "%s ", groupname);
    }
    return;
}

void getFileSize(size_t st_size, char* s, bool humanReadable) {
    long bytes = (long) st_size;
    
    if (humanReadable) {
        if (bytes == 0)
            sprintf(s, "0B");
        
        else {
            const char * const suffixes[] = { "B", "KB", "MB", "GB", "TB" } ;
            long l = floor(log(bytes) / log(1024));
            double v = bytes / pow(1024, l);
            sprintf(s, "%.1f%s", v, suffixes[l]);
        }   
    } else {
        sprintf(s, "%ld", bytes);
    }
}

void getTime(time_t* tt, char* r) {
    struct tm * p = localtime(tt);
    strftime(r, 50, "%b %e %H:%M", p);
}
