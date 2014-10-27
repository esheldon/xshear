#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "urls.h"

FILE* open_url(const char* filename, const char* mode) {
    FILE* stream=fopen(filename,mode);
    if (stream==NULL) {
        wlog("Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    return stream;
}

size_t count_lines(FILE* stream)
{
    size_t nlines=0;

    while (EOF != (fscanf(stream,"%*[^\n]"), fscanf(stream,"%*c")))  {
        ++nlines;
    }
    return nlines;
}

size_t count_lines_and_rewind(FILE* stream)
{
    rewind(stream);

    size_t nlines=count_lines(stream);

    rewind(stream);
    return nlines;
}
