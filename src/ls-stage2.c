#define _GNU_SOURCE // Needed for scandir() to work

#include "ls-stage2.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>  // opendir

int simpleList(char* filename, int parentDirfd, bool oneFile, Flags* flagStruct);
void printFileListing(char* filename, struct stat* st, Flags* f);
void printFileError(char* filename);

int main(int argc, char *argv[]) {
    // Counts the number of arguments representing
    // flags and creates a struct to represent the set flags
    Flags flagStruct = {};

    // Updates the above struct to reflect the user-supplied flags
    int flagCount = getFlagsFromArgs(argc, argv, &flagStruct);
    if (flagCount < 0) return 1; // Exit message printed by above function

    // Gets the file descriptor for the current directory (must be
    // passed to simpleList)
    DIR* cdStream = opendir(".");
    if (cdStream == NULL) {
        printFileError(".");
        return -1;
    }
    int cdFd = dirfd(cdStream);
    if (cdFd < 0) {
        printFileError(".");
        return -1;
    }

    // If there was no file supplied, treat the command
    // as though '.' was the file.
    int fileCount = argc - 1 - flagCount;
    if (fileCount == 0)
        simpleList(".", cdFd, true, &flagStruct);
    
    // Otherwise call simpleList() on all of the files.
    else {
        bool oneFile = fileCount == 1;

        // For each file supplied, list that file / its contents
        for (int i = 1 + flagCount; i < argc; i++)
            simpleList(argv[i], cdFd, oneFile, &flagStruct);
    }
    return 0;
}

// Outputs a listing of the given directory
int simpleList(char* filename, int parentDirfd, bool oneFile,
    Flags* flagStruct) {

    // Gets a struct containing all the information about the given
    // file. NOTE: with lstat() if path is a symbolic link, then the
    // link itself is stat-ed, not the file that it refers to.
    struct stat st;
    if (lstat(filename, &st)) {
        // If the file can't be found, print out a suitable error
        printFileError(filename);
        return -1;
    }

    // If "filename" is not a directory, simply print out the
    // relevant information about that file
    if (!S_ISDIR(st.st_mode))
        printFileListing(filename, &st, flagStruct);

    // If "filename" is a directory, then print the filename,
    // followed by a list of its directory contents.
    else {
        // Sets our directory ordering function
        // depending on the ordering flag
        int (*ord) (const struct dirent**, const struct dirent**);
        if (flagStruct->r)
            ord = &descComp;
        else
            ord = &ascComp;

        // Sets our directory filtering function
        // depending on the "all" flag
        int (*filter) (const struct dirent*);
        if (flagStruct->a)
            filter = NULL;
        else
            filter = &notDotFile;

        // Attempts to open the directory returning dirent objects
        // for its subfiles into subFileDirents, sorting the
        // contents in the specified order, ignoring
        // those that begin with a dot.
        struct dirent **subFileDirents;
        int subFileCount = scandirat(parentDirfd, filename,
                &subFileDirents, filter, ord);

        if (subFileCount < 0) {
            // If scandir fails, print an error
            printFileError(filename);
            return -1;
        }

        // Gets a pointer to the directory stream and then gets
        // the file descriptor associated with that stream
        // (needed for fstatat() below)
        DIR* dir = opendir(filename);
        if (dir == NULL) {
            printFileError(filename);
            return -1;
        }
        int fd = dirfd(dir);
        if (fd < 0) {
            printFileError(filename);
            return -1;
        }
        
        // Creates an array of the stat structs for each subfile
        struct stat subFileStats[subFileCount];
        for (int i = 0; i < subFileCount; i++) {
            char* subFileName = subFileDirents[i]->d_name;
            struct stat subSt;
            // fstatat() lets us use the directory stream pointer
            // as a relative path for the subfile name, then works
            // exactly the same as stat(). The argument at the
            // end of the function gives us the symbolic link
            // handling of lstat().
            if (fstatat(fd, subFileName, &subSt, AT_SYMLINK_NOFOLLOW)) {
                printFileError(subFileName);
                return -1;
            }
            subFileStats[i] = subSt;
        }

        // If the sort-by-size flag is set, calls a function to sort
        // our array of stat objects
        if (flagStruct->S)
            sortBySize(subFileStats, subFileCount);

        // Unless this is a one file "ls-stage" command, print
        // the filename above the directory
        if (!oneFile)
            printf("\n%s:\n", filename);

        // Iterates through the array of subfile stats, calling a
        // function to print the relevant info
        for (int i = 0; i < subFileCount; i++) {
            // Tab indent subfiles (unless this is a
            // "one file" listing)
            if (!oneFile)
                printf("\t");

            printFileListing(subFileDirents[i]->d_name,
                    &subFileStats[i], flagStruct);
        }
    }
    return 0;
}

// Prints out the information about a file specified by the provided flags
void printFileListing(char* filename, struct stat* st, Flags* f) {
    // Print inode number if flag set
    if (f->i)
        printf("%ld\t ", (long) st->st_ino);
    
    // Print long listing if flag set
    if (f->l) {
        // Permissions
        char perms[11];
        getPermissions(st->st_mode, perms);
        printf("%s\t ", perms);
        
        // Number of links
        printf("%ld\t ", (long) st->st_nlink);

        // Print user & group IDs depending on '-n' flag
        char user[50], group[50];
        getIDs(user, group, st, f->n);
        printf("%s\t %s\t ", user, group);

        // File size
        char size[50];
        getFileSize(st->st_size, size, f->h);
        printf("%s\t ", size);

        // Time of last modification
        char time[100];
        getTime(&st->st_mtime, time);
        printf( "%s\t ", time); 
    }

    printf("%s\n", filename);
}

void printFileError(char* filename) {
    char errorMsg[300];
    strcpy(errorMsg, "ls-stage2: cannot access ");
    strcat(errorMsg, filename);
    perror(errorMsg);
}
