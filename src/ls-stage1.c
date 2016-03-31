#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

int simpleList(char* filename, bool oneFile);
void printError(char* filename);
int lscompare(const void * x, const void * y);

int main(int argc, char *argv[]) {
    // If there was no argument supplied, treat the command as though '.'
    // was the argument.
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

        // Attempts to open the directory
        DIR *dir = opendir(filename);
        if (dir == NULL) {
            printError(filename);
            return -1;
        }

        struct dirent *dent;

        // Counts the number of files in the directory
        int file_count = 0;
        while ((dent = readdir(dir)) != NULL)
            if (dent->d_name[0] != '.')
                file_count++;

        rewinddir(dir);

        // Reads the names of the files in the directory and stores
        // them in an array.
        char filenames[file_count][256];
        int i = 0;
        while ((dent = readdir(dir)) != NULL) {
            // Ignore files beginning with '.'
            if (dent->d_name[0] != '.') {
                strcpy(filenames[i++],
                       dent->d_name);
            }
        }
        closedir(dir);

        // Sorts the array of filenames and prints
        // them alphabetically
        //qsort(filenames, sizeof(filenames) / sizeof(char *),
          //      sizeof(char*), lscompare);
        
        // Prints out the filenames in the directory
        for (int i = 0; i < file_count; i++) {
            // unless we are just printing one file, tab-indent
            // the directory's sub-files
            if (!oneFile)
                printf("\t");
            printf("%s\n", filenames[i]);
        }
        if (!oneFile)
            printf("\n");
    }

    return 0;
}

void printError(char* filename) {
    char errorMsg[300];
    strcpy(errorMsg, "ls-stage1: cannot access ");
    strcat(errorMsg, filename);
    perror(errorMsg);
}

int lscompare (const void * x, const void * y ) {
    const char *px = *(const char**)x;
    const char *py = *(const char**)y;
    return strcmp(px,py);
}
