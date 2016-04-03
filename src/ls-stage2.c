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

int main(int argc, char *argv[]) {
    // Counts and handles the number of arguments representing
    // flags
    int flagCount = 0;
    FlagInfo flagInfo = {false, false, false, false};
    if (argc > 1) {
        // Keep handling flags as long as we still have arguments
        // and the next argument begins '-'
        while (flagCount <= argc - 2 &&
                argv[flagCount + 1][0] == '-') {

            char flag = argv[flagCount + 1][1];
printf("%c\n", flag);
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

printf("%d %d %d %d\n", flagInfo->l, flagInfo->n, flagInfo->i, flagInfo->S);

// Gets a struct containing all the information about the given
    // file. 
    struct stat st;
    if(stat(filename, &st)) {
        // If the file can't be found, print out a suitable error
        printFileError(filename);
        return -1;
    }

    // If "filename" is not a directory, print its name
    if(!S_ISDIR(st.st_mode))
        printf("%s\n", filename);
    // If it is, then print the filename, followed by a list of its
    // directory contents.
    else {
        // Unless this is a one file "ls-stage1" command, print
        // the filename above the directory
        if (!oneFile)
            printf("\n%s:\n", filename);

        // Sets our ordering function depending on the ordering flag
        int (* ord )(const struct dirent**, const struct dirent**);
        if (flagInfo->S)
            ord = &ascComp;
        else
            ord = &descComp;

        // Attempts to open the directory, sorting the contents
        // in the specified order, ignoring
        // those that begin with a dot.
        struct dirent **namelist;
        int noOfFiles = scandir(filename,
                &namelist, &notDotFile, ord);
        
        if (noOfFiles == -1) {
            // If scandir fails, print an error
            printFileError(filename);
            return -1;
        } else {
            // Print the files in the directory
            for (int i = 0; i < noOfFiles; i++) {
                // Tab-indent directory sub-files unless this
                // is a one-file listing
                if (!oneFile)
                    printf("\t");
                printf("%s\n", namelist[i]->d_name);
            }
        }
    }
    return 0;
}

int setFlagInfo(FlagInfo* f, char c) {
    switch (c) {
        case 'l' :
            f->l = true;
            break;
        
        case 'n' :
            f->n = true;
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
