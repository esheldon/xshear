#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "urls.h"

FILE* open_url(const char* filename, const char* mode) {
    FILE* fptr=fopen(filename,mode);
    if (fptr==NULL) {
        wlog("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    return fptr;
}
