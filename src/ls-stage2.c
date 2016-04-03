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

int simpleList(char* filename, bool oneFile);
void printError(char* filename);
int notDotFile(const struct dirent * dent);
int fComp(const struct dirent ** dentA,
        const struct dirent ** dentB);

int main(int argc, char *argv[]) {
    // If there was no argument supplied, treat the command
    // as though '.' was the argument.
    if (argc == 1)
        simpleList(".", true);
    // Otherwise call simpleList() on all of the arguments.
    else {
        // Determine if we're only printing one file
        bool oneFile = argc == 2;
        for (int i = 1; i < argc; i++)
            simpleList(argv[i], oneFile);
    }
}

// Outputs a listing of the given directory
int simpleList(char* filename, bool oneFile) {
    // Gets a struct containing all the information about the given
    // file. 
    struct stat st;
    if(stat(filename, &st)) {
        // If the file can't be found, print out a suitable error
        printError(filename);
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

        // Attempts to open the directory, sorting the contents
        // in alphabetical (case-insensitive) order, ignoring
        // those that begin with a dot.
        struct dirent **namelist;
        int noOfFiles = scandir(filename,
                &namelist, &notDotFile, &fComp);
        
        if (noOfFiles == -1) {
            // If scandir fails, print an error
            printError(filename);
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

void printError(char* filename) {
    char errorMsg[300];
    strcpy(errorMsg, "ls-stage1: cannot access ");
    strcat(errorMsg, filename);
    perror(errorMsg);
}

int notDotFile(const struct dirent * dent) {
    return dent->d_name[0] != '.';
}

int fComp(const struct dirent ** dentA,
        const struct dirent ** dentB) {
    return strcasecmp((*dentA)->d_name, (*dentB)->d_name);
}
