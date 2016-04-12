#include "ls-stage2.h"

#include <stdio.h> // fprintf()

int setFlag(Flags* flagStruct, char flagChar);

// Passed the command line arguments and edits the flagStruct to
// reflect passed flags. Returns the number of flags supplied
int getFlagsFromArgs(int argc, char* argv[], Flags* flagStruct) {
    // Updates the flag struct to reflect the supplied flags
    int flagCount = 0;
    if (argc > 1) {       

        // "flagCount" marks the current argument we're looking
        // at. This while loop checks if there are still flags left as command
        // line arguments. The second boolean check here makes
        // sure the argument begins with '-' and hence is a flag.
        // The first check makes sure that we do not try to access
        // an argument which overflows the size of argv[].
        while (flagCount <= argc - 2 &&
                argv[flagCount + 1][0] == '-') {

            // Gets the character used as a flag
            char flag = argv[flagCount + 1][1];

            // Attempts to set the flag info struct to reflect the
            // current flag. If the flag is not regognised, -1 is
            // returned and we exit with an error.
            if (setFlag(flagStruct, flag) == -1) {
                fprintf(stderr,
                        "ls-stage2: invalid option -- '%c'\n",
                        flag);
                return -1;
            }

            flagCount++;
        }
    }
    return flagCount;
}

int setFlag(Flags* flagStruct, char flagChar) {
    switch (flagChar) {
        case 'n' :
            flagStruct->n = true;
            // No break here as -n turns on -l

        case 'l' :
            flagStruct->l = true;
            break;
        
        case 'i' :
            flagStruct->i = true;
            break;

        case 'r' :
            flagStruct->r = true;
            break;

        case 'S' :
            flagStruct->S = true;
            break;

        default :
            return -1;
    }
    return 0;
}
