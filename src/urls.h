#ifndef _URLS_HEADER
#define _URLS_HEADER



FILE* open_url(const char* filename, const char* mode);

/*
   count the number of lines from the current position
*/
size_t count_lines(FILE* stream);
/*
   rewind, count lines, and then rewind again
*/
size_t count_lines_and_rewind(FILE* stream);

#endif
