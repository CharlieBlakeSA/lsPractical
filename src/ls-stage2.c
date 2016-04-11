#define _GNU_SOURCE // Needed for scandir() to work
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>
#include <strings.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>

typedef struct {
    char* filename;
    struct stat fileStat; 
} ListingInfo;

typedef struct {
    bool l, n, i, S;
} FlagInfo;

int setFlagInfo(FlagInfo* f, char c);
int simpleList(char* filename, bool oneFile, FlagInfo* flagInfo);
void printFileError(char* filename);
int notDotFile(const struct dirent* dent);
int ascComp(const struct dirent** dentA,
        const struct dirent** dentB);
int descComp(const struct dirent** dentA,
        const struct dirent** dentB);
void getPermissions (mode_t st_mode, char* s);
void getIDs(char* user, char* group, struct stat* st, bool asNumber);
void getTime(time_t* tt, char* r);
void printIndividualFile(ListingInfo* li, FlagInfo* f);
void printFileListing(char* directoryname, bool oneFile, int noOfFiles,
        ListingInfo l[noOfFiles], FlagInfo* f);


int main(int argc, char *argv[]) {
    // Counts the number of arguments representing
    // flags and creates a struct to represent the set flags
    int flagCount = 0;
    // Struct representing the flags which are set
    FlagInfo flagInfo = {false, false, false, false};

    if (argc > 1) {
        // Checks if there are still flags left as command
        // line arguments. The second boolean check here makes
        // sure the argument begins with '-' and hence is a flag.
        // The first check makes sure that we do not try to access
        // an argument which overflows the size of argv[].
        while (flagCount <= argc - 2 &&
                argv[flagCount + 1][0] == '-') {

            char flag = argv[flagCount + 1][1];
            // Attempts to set the flag info struct to reflect the
            // current flag. If the flag is not regognised, -1 is
            // returned and we exit with an error.
            if (setFlagInfo(&flagInfo, flag) == -1) {
                fprintf(stderr,
                        "ls-stage2: invalid option -- '%c'\n",
                        flag);
                return -1;
            }

            flagCount++;
        }
    }

    int fileCount = argc - 1 - flagCount;
    // If there was no file supplied, treat the command
    // as though '.' was the file.
    if (fileCount == 0)
        simpleList(".", true, &flagInfo);
    // Otherwise call simpleList() on all of the files.
    else {
        // Determine if we're only printing one file
        bool oneFile = fileCount == 1;
        for (int i = 1 + flagCount; i < argc; i++)
            simpleList(argv[i], oneFile, &flagInfo);
    }
    return 0;
}

// Outputs a listing of the given directory
int simpleList(char* filename, bool oneFile, FlagInfo* flagInfo) {

    // Gets a struct containing all the information about the given
    // file. 
    struct stat st;
    if(stat(filename, &st)) {
        // If the file can't be found, print out a suitable error
        printFileError(filename);
        return -1;
    }

    // If "filename" is not a directory, print its name
    if(!S_ISDIR(st.st_mode)) {
        // gets the relevant information about the file
        ListingInfo li = {filename, st};
        printIndividualFile(&li, flagInfo);
    }

    // If it is, then print the filename, followed by a list of its
    // directory contents.
    else {
        // Sets our ordering function depending on the ordering flag
        int (* ord )(const struct dirent**, const struct dirent**);
        if (flagInfo->S)
            ord = &descComp;
        else
            ord = &ascComp;

        // Attempts to open the directory, sorting the contents
        // in the specified order, ignoring
        // those that begin with a dot.
        struct dirent **namelist;
        int noOfFiles = scandir(filename,
                &namelist, &notDotFile, ord);
        
        if (noOfFiles < 0) {
            // If scandir fails, print an error
            printFileError(filename);
            return -1;
        }
        
        // Unless this is a one file "ls-stage" command, print
        // the filename above the directory
        if (!oneFile)
            printf("\n%s:\n", filename);
   

        //COMMENT
        DIR* parentDir = opendir(filename);
        if (parentDir == NULL) {
            printFileError(filename);
        }

        for (int i = 0; i < noOfFiles; i++) {
            struct stat subSt;
            if (fstatat(dirfd(parentDir), namelist[i]->d_name, &subSt, AT_SYMLINK_NOFOLLOW)) {
                printFileError(namelist[i]->d_name);
                return -1;
            }
            
            if (!oneFile)
                printf("\t");

            ListingInfo li = {namelist[i]->d_name, subSt};
            printIndividualFile(&li, flagInfo);
        }
    }
    return 0;
}

void printIndividualFile(ListingInfo* li, FlagInfo* f) {
    struct stat* st = &li->fileStat;

    // Print inode number if flag set
    if (f->i)
        printf("%ld ", (long) st->st_ino);
    
    if (f->l) {
        // Permissions
        char perms[11];
        getPermissions(st->st_mode, perms);
        printf("%s\t", perms);
        
        // Number of links
        printf("%ld\t", (long) st->st_nlink);

        // Print user & group IDs depending on '-n' flag
        char user[50], group[50];
        getIDs(user, group, st, f->n);
        printf("%s\t%s\t", user, group);

        // File size
        printf("%ld\t", (long) st->st_size);

        // Time of last modification
        char time[100];
        getTime(&st->st_mtime, time);
        printf( "%s\t", time); 
    }

    printf("%s\n", li->filename);
}

void printFileListing(char* directoryName, bool oneFile, int noOfFiles,
        ListingInfo l[noOfFiles], FlagInfo* f) {

    // Unless this is a one file "ls-stage" command, print
    // the filename above the directory
    if (!oneFile)
        printf("\n%s:\n", directoryName);
   
    for (int i = 0; i < noOfFiles; i++) {
        if (!oneFile)
            printf("\t");

        printIndividualFile(&l[i], f);
    }
}

int setFlagInfo(FlagInfo* f, char c) {
    switch (c) {
        case 'n' :
            f->n = true;
            // No break here as -n turns on -l

        case 'l' :
            f->l = true;
            break;
        
        case 'i' :
            f->i = true;
            break;

        case 'S' :
            f->S = true;
            break;

        default :
            return -1;
    }
    return 0;
}

void printFileError(char* filename) {
    char errorMsg[300];
    strcpy(errorMsg, "ls-stage1: cannot access ");
    strcat(errorMsg, filename);
    perror(errorMsg);
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

void getTime(time_t* tt, char* r) {
    struct tm * p = localtime(tt);
    strftime(r, 50, "%b %e %H:%M", p);
}

int notDotFile(const struct dirent * dent) {
    return dent->d_name[0] != '.';
}

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

int ascComp(const struct dirent ** dentA,
        const struct dirent ** dentB) {
    return strcasecmp((*dentA)->d_name, (*dentB)->d_name);
}

int descComp(const struct dirent ** dentA,
        const struct dirent ** dentB) {
    return strcasecmp((*dentB)->d_name, (*dentA)->d_name);
}
