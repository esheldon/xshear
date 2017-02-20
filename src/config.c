/*

   A very simple config file format, parser

   Erin Sheldon, Brookhaven National Laboratory
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "config.h"

static char *cfg_status_names[]= {
    "SUCCESS",
    "FOUND_END",
    "READ_ERROR",
    "PARSE_BLANK",
    "PARSE_COMMENT",
    "PARSE_FAILURE",
    "EOF",
    "NOT_FOUND",
    "CFG_EMPTY",
    "TYPE_ERROR"
};

/*
 * public
 *
 * Free an array of strings
 */
char **cfg_strarr_free(char **arr, size_t size)
{
    size_t i=0;
    if (arr) {
        for (i=0; i<size; i++) {
            free(arr[i]);
            arr[i]=NULL;
        }
        free(arr); arr=NULL;
    }
    return NULL;
}

/*
 * public
 *
 * Return a string version.  Do not free!
 */
const char* cfg_status_string(enum cfg_status status)
{
    int imax=sizeof(cfg_status_names)-1;

    if (status < 0 || status > imax) {
        return NULL;
    } else {
        return cfg_status_names[status];
    }
}

static char *cfg_strdup(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if(p) {
        strcpy(p, s);
    }
    return p;
}

static struct cfg_string *cfg_string_free(struct cfg_string *str)
{
    if (str) {
        free(str->data);
        free(str);
    }
    return NULL;
}
static struct cfg_strvec *cfg_strvec_new()
{
    struct cfg_strvec *vec=NULL;
    vec=calloc(1, sizeof(struct cfg_strvec));
    if (vec==NULL) {
        fprintf(stderr,"Could not allocate cfg_strarr\n");
        exit(1);
    }
    vec->data = calloc(1, sizeof(char*));
    if (vec->data == NULL) {
        fprintf(stderr,"Failed to allocate config list\n");
        exit(1);
    }
    vec->capacity=1;
    return vec;
}
static void cfg_strvec_realloc(struct cfg_strvec* vec, size_t newcap)
{
    size_t oldcap = 0, num_new_bytes=0;
    if (!vec) {
        return;
    }
    oldcap = vec->capacity;
    if (newcap > oldcap) {
        vec->data = realloc(vec->data, newcap*sizeof(char*));

        if (vec->data == NULL) {
            fprintf(stderr,"failed to realloc string vec\n");
            exit(1);
        }
        num_new_bytes = (newcap-oldcap)*sizeof(char*);
        memset(vec->data + oldcap, 0, num_new_bytes);
        vec->capacity = newcap;
    }
}

/*
 * might need this for creating new configs
static void cfg_strvec_append_copy(struct cfg_strvec* vec, char *str)
{
    size_t size=0;
    size = vec->size;

    if (size == vec->capacity) {
        cfg_strvec_realloc(vec, vec->capacity*2);
    }
    vec->size++;
    vec->data[vec->size-1] = cfg_strdup(str);
}
*/
/* ownership is transferred to the vector */
static void cfg_strvec_append_nocopy(struct cfg_strvec* vec, char *str)
{
    size_t size=0;
    size = vec->size;

    if (size == vec->capacity) {
        cfg_strvec_realloc(vec, vec->capacity*2);
    }
    vec->size++;
    vec->data[vec->size-1] = str;
}

static struct cfg_strvec *cfg_strvec_free(struct cfg_strvec* vec)
{
    if (vec) {
        if (vec->data) {
            vec->data = cfg_strarr_free(vec->data, vec->capacity);
        }
        free(vec);
    }
    return NULL;
}

/*
 * copy the data
 */
static char **cfg_strvec2arr(struct cfg_strvec *vec, size_t *size)
{
    char **sarr=NULL;
    size_t i=0;

    *size=0;
    if (!vec || vec->size == 0) {
        return sarr;
    }

    *size=vec->size;
    sarr=calloc((*size), sizeof(char*));
    if (!sarr) {
        fprintf(stderr,"could not allocate for copy of strvec\n");
        exit(1);
    }
    for (i=0; i<(*size); i++) {
        sarr[i] = cfg_strdup(vec->data[i]);
    }

    return sarr;
}
static struct cfg_field *cfg_field_new()
{
    struct cfg_field *self=NULL;

    self=calloc(1, sizeof(struct cfg_field));
    if (NULL == self) {
        fprintf(stderr,"Failed to create config field\n");
        exit(1);
    }

    self->strvec = cfg_strvec_new();
    if (!self->strvec) {
        fprintf(stderr,"Failed to create config field\n");
        exit(1);
    }
    return self;
}

static struct cfg_field *cfg_field_free(struct cfg_field *self)
{
    if (self) {
        // We always zero memory on creation, so this is OK
        free(self->name);
        self->strvec = cfg_strvec_free(self->strvec);
        self->sub = cfg_free(self->sub);
        free(self);
    }
    return NULL;
}

static void print_spaces(int n, FILE* stream) {
    int i=0;
    for (i=0; i<n; i++) {
        fprintf(stream," ");
    }
}
static void cfg_field_print_data(struct cfg_field *self, size_t el, FILE* stream)
{
    char *tmp=NULL;
    size_t len=0, i=0;
    int c=0;
    if (CFG_ELTYPE_QUOTED_STRING==CFG_FIELD_ELTYPE(self)) {
        tmp = CFG_FIELD_GET_DATA(self,el);
        len=strlen(tmp);
        fprintf(stream,"\"");
        for (i=0; i<len; i++) {
            c=tmp[i];
            if (c == '"') {
                fprintf(stream,"\\\"");
            } else {
                fprintf(stream,"%c", c);
            }
        }
        fprintf(stream,"\"");
    } else {
        fprintf(stream, "%s", CFG_FIELD_GET_DATA(self,el));
    }
}
static void _cfg_print(struct cfg *self, int n_indent, FILE* stream);
static void cfg_field_print(struct cfg_field *self, int n_indent, FILE* stream)
{
    size_t el=0;
    if (!self)
        return;

    // scalars not allowed to be "empty"
    if (CFG_TYPE_SCALAR == CFG_FIELD_TYPE(self) && 0==CFG_FIELD_ARR_SIZE(self))
        return;

    print_spaces(n_indent, stream);
    fprintf(stream, "%s", CFG_FIELD_NAME(self));
    fprintf(stream," %c ", CFG_ASSIGN);
    if (CFG_TYPE_ARRAY==CFG_FIELD_TYPE(self)) {

        fprintf(stream,"%c", CFG_ARRAY_BEG);
        for (el=0; el<CFG_FIELD_ARR_SIZE(self); el++) {
            cfg_field_print_data(self, el, stream);
            /*
            if (el < (CFG_FIELD_ARR_SIZE(self)-1)) {
                fprintf(stream,"%c", CFG_ARRAY_SEP);
            }
            */
            if (el < (CFG_FIELD_ARR_SIZE(self)-1)) {
                fprintf(stream," ");
            }
        }
        fprintf(stream,"%c", CFG_ARRAY_END);

    } else if (CFG_TYPE_CFG == CFG_FIELD_TYPE(self)) {

        fprintf(stream,"%c\n", CFG_CFG_BEG);
        _cfg_print(CFG_FIELD_GET_SUB(self),n_indent+4,stream);
        print_spaces(n_indent, stream);
        fprintf(stream,"%c", CFG_CFG_END);

    } else {
        cfg_field_print_data(self, 0, stream);
    }
    fprintf(stream,"\n");
}

static struct cfg *cfg_new()
{
    struct cfg *self=NULL;

    self=calloc(1, sizeof(struct cfg));
    if (NULL == self) {
        fprintf(stderr,"Failed to create config struct\n");
        exit(1);
    }

    self->fields = calloc(1, sizeof(struct cfg_field*));
    self->capacity=1;
    return self;

}
static void cfg_realloc(struct cfg* self, size_t newcap)
{
    size_t oldcap = 0, num_new_bytes=0;
    if (!self) {
        return;
    }
    oldcap = self->capacity;
    if (newcap > oldcap) {
        self->fields = realloc(self->fields, newcap*sizeof(struct cfg_field*));

        if (self->fields == NULL) {
            fprintf(stderr,"failed to realloc config\n");
            exit(1);
        }
        num_new_bytes = (newcap-oldcap)*sizeof(struct cfg_field*);
        memset(self->fields + oldcap, 0, num_new_bytes);
        self->capacity = newcap;
    }
}


// ownership is given to the vector
static void cfg_append(struct cfg *self, struct cfg_field *field)
{
    size_t size=0;
    size = self->size;

    if (size == self->capacity) {
        cfg_realloc(self, self->capacity*2);
    }
    self->size++;
    self->fields[self->size-1] = field;
}

/*
 * Public
 *
 * Delete a config structure
 */
struct cfg *cfg_free(struct cfg *self)
{
    if (self) {
        if (self->fields) {
            size_t i=0;
            for (i=0; i<self->size; i++) {
                self->fields[i] = cfg_field_free(self->fields[i]);
            }
        }
        free(self->fields); self->fields=NULL;
        free(self);
    }
    return NULL;
}

static void _cfg_print(struct cfg *self, int n_indent, FILE* stream)
{
    size_t el=0;

    if (!self)
        return;

    for (el=0; el<self->size; el++) {
        cfg_field_print(self->fields[el], n_indent, stream);
    }

}
/*
 * Public
 *
 * Print the config to the input stream
 */
void cfg_print(struct cfg *self, FILE* stream)
{
    size_t el=0;
    int n_indent=0;

    if (!self)
        return;

    for (el=0; el<self->size; el++) {
        cfg_field_print(self->fields[el], n_indent, stream);
    }

}

static size_t find_char(const struct cfg_string *str, size_t current, int ch, 
                        enum cfg_status *status)
{
    size_t i=0, loc=-1;

    *status=CFG_EOF;
    // this accounts for case where current is out of bounds
    for (i=current; i<str->size; i++) {
        if (str->data[i] == ch) {
            *status=CFG_SUCCESS;
            loc=i;
            break;
        }
    }
    return loc;
}
static size_t skip_comment(const struct cfg_string *str, size_t current, enum cfg_status *status)
{
    size_t loc=0;
    loc = find_char(str, current, '\n', status);
    return loc;
}

/*
 * search for the next non-white character starting from the 
 * current position
 */
static size_t find_nonwhite(const struct cfg_string *str, size_t current, enum cfg_status *status)
{
    size_t i=0, loc=0;

    loc=current;

    *status=CFG_EOF;
    // this accounts for case where current is out of bounds
    for (i=current; i<str->size; i++) {
        if (!isspace(str->data[i])) {

            if (str->data[i] == CFG_COMMENT) {
                // found a comment, skip to next EOL
                i = skip_comment(str, i, status);
                // also set loc in case we exit the loop
                loc=i;
                if (*status==CFG_EOF) { // EOF, loc remains at current
                    break;
                }
                continue;
            }

            *status=CFG_SUCCESS;
            loc=i;
            break;
        }
    }
    return loc;
}
/*
static size_t find_white(const struct cfg_string *str, size_t current, enum cfg_status *status)
{
    size_t i=0, loc=-1;

    *status=CFG_EOF;
    // this accounts for case where current is out of bounds
    for (i=current; i<str->size; i++) {
        if (isspace(str->data[i])) {
            *status=CFG_SUCCESS;
            loc=i;
            break;
        }
    }
    return loc;
}
*/
/* find white space and cut off with a null char */
static void rstrip_inplace(char *str, size_t len)
{
    size_t i=0;
    
    for (i=0;i<len;i++) {
        if (isspace(str[i])) {
            str[i] = '\0';
	    break;
        }
    }
}





static struct cfg_string *read_whole_file(const char* filename)
{
    FILE* fp=NULL;
    char *data=NULL;
    size_t nchar=0, nread=0;
    struct cfg_string *str=NULL;

    fp = fopen(filename,"r");
    if (fp==NULL) {
        fprintf(stderr,"Could not open file: %s\n", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    nchar = ftell(fp);
    rewind(fp);

    data = calloc(sizeof(char), nchar + 1);
    if (!data) {
        fprintf(stderr,"Could not allocate %lu bytes for whole file read\n", nchar);
        exit(1);
    }

    nread=fread(data, 1, nchar, fp);
    if(ferror(fp) || nread!=nchar){
        fprintf(stderr,"Error reading file\n");
        free(data);
        goto _read_whole_file_bail;
    }

    str = calloc(1, sizeof(struct cfg_string));
    if (str == NULL) {
        fprintf(stderr,"Could not allocate cfg_string container for file string\n");
        exit(1);
    }

    str->size = nchar;
    str->data = data;
_read_whole_file_bail:
    fclose(fp);
    return str;
}

/*
 * end will be left on the assignment character
 */
static void find_next_identifier(const struct cfg_string *str, 
                                 size_t current, 
                                 size_t *beg, 
                                 size_t *end, 
                                 enum cfg_status *status)
{
    while (1) {
        // skip any whitespace and comments
        current = find_nonwhite(str, current, status);
        if (*status) { // EOF
            return;
        }

        *beg=current;

        // now find the assignment character
        current = find_char(str, current, CFG_ASSIGN, status);
        if (*status) {
            // this can't be good; must have left off the assignment char
            return;
        }
        *status = CFG_SUCCESS;
        *end=current;
        break;
    }
}
static char *copy_next_identifier(const struct cfg_string *str, 
                           size_t current, 
                           size_t *end, 
                           enum cfg_status *status)
{
    char *name=NULL;
    size_t sbeg=0, send=0, size=0;
    find_next_identifier(str, current, &sbeg, end, status);
    if (*status) {
        return NULL;
    }

    send=*end-1;
    size=(send-sbeg+1);
    name=calloc(size+1, sizeof(char));
    if (!name) {
        fprintf(stderr,"could not allocate for name\n");
        exit(1);
    }
    strncpy(name, &str->data[sbeg], size);

    rstrip_inplace(name,size);
    return name;
}
// current should point to the first letter
/*
static size_t find_end_of_whitespace_token(const struct cfg_string *str, 
                                           size_t current, 
                                           enum cfg_status *status)
{
    size_t loc=0;
    loc = find_white(str, current, status);
 
    // the end of the word could come with no white space, that is OK too
    if (CFG_EOF==*status) {
        *status=CFG_SUCCESS;
        loc=str->size-1;
    } else {
        // scoot back one so we sit on the last letter
        loc--;
    }
    return loc;
}
*/
/*
 * current should already point to the first letter
 *
 * return value will point to last letter in the token
 */
/*
static size_t copy_bare_token(struct cfg_field *field,
                              const struct cfg_string *str,
                              size_t current,
                              enum cfg_status *status)
{
    size_t beg=0, end=0, size=0;
    char *tmp=NULL;
    // single token expected now.  no need to check status

    beg = current;
    end = find_end_of_whitespace_token(str,current,status);

    size=(end-beg+1);

    tmp=calloc(size+1, sizeof(char));

    strncpy(tmp, &str->data[beg], size);
    // ownership transferred to vector
    cfg_strvec_append_nocopy(field->strvec, tmp);

    return end; 
}
*/
/*
 * current should already point to the first letter
 *
 * return value will point to 
 *   array separator 
 *   array end char
 *   white space
 */

static size_t copy_bare_array_token(struct cfg_field *field,
                                    const struct cfg_string *str,
                                    size_t current,
                                    enum cfg_status *status)
{
    size_t beg=0, end=0, size=0;
    int c=0;
    char *tmp=NULL;
    // single token expected now.  no need to check status

    beg=current;
    end=current;
    *status=CFG_EOF;
    do {
        current++;
        if (current > (str->size-1)) {
            goto _copy_bare_array_token_bail;
        }
        c=str->data[current];
    } while (!isspace(c) && c != CFG_ARRAY_SEP && c != CFG_ARRAY_END && c != CFG_CFG_END);

    *status=CFG_SUCCESS;

    end=current-1;
    size=(end-beg+1);

    tmp=calloc(size+1, sizeof(char));

    strncpy(tmp, &str->data[beg], size);
    // ownership transferred to vector
    cfg_strvec_append_nocopy(field->strvec, tmp);

_copy_bare_array_token_bail:
    return current;
}

/*
static struct cfg_strvec *copy_bare_token_as_vec(const struct cfg_string *str,
                                                       size_t current,
                                                       size_t *end,
                                                       enum cfg_status *status)
{
    struct cfg_strvec *vec=NULL;
    char *tmp=NULL;
    tmp = copy_bare_token(str, current, end, status);
    if (*status == CFG_SUCCESS) {
        vec=cfg_strvec_new();
        // vec will now own the string
        cfg_strvec_append_nocopy(vec, tmp);
    }
    return vec;
}
*/

/* current should point to the opening quote
 * here we return the location of the ending quote
 *
 * We allow empty strings and escaped quote characters
 *
 * on error we return zero and status is set
 */
static size_t find_end_of_quoted_string(const struct cfg_string *str, 
                                        size_t current, 
                                        enum cfg_status *status)
{
    size_t loc=0;

    *status = CFG_EOF;
    if (current == (str->size-1)) {
        return 0;
    }

    // skip past the quote
    current++;
    while (1) {
        loc = find_char(str, current, CFG_QUOTE, status);
        // the end of the word could come with no white space, that is OK too
        if (*status) {
            break;
        }

        if (str->data[loc-1] == CFG_ESCAPE) {
            current=loc+1;
            continue;
        } else {
            break;
        }
    }
    return loc;
}

/* current should point to the opening quote
 * here we return the location of the ending quote
 *
 * We allow empty strings and escaped quote characters
 *
 */
static size_t copy_quoted_string(struct cfg_field *field,
                                 const struct cfg_string *str,
                                 size_t current,
                                 enum cfg_status *status)
{
    size_t sbeg=0, endquote=0, send=0, size=0, isrc=0, idst=0;
    int c=0, cold=0;
    char *tmp=NULL;

    // skip past opening quote
    sbeg = current+1;
    endquote = find_end_of_quoted_string(str,current,status);
    if (*status) {
        return endquote;
    }

    // step back from final quote
    send = endquote-1;

    size=(send-sbeg+1);
    tmp=calloc(size+1, sizeof(char));

    // now copy but skip leading \ for \" imbedded quotes
    for (isrc=0; isrc<size; isrc++) {
        c=str->data[sbeg+isrc];
        if (cold == '\\' && c == '"') {
            tmp[idst-1] = '"';
        } else {
            tmp[idst] = c;
            idst++;
        }
        cold=c;
    }
    // ownership given to the vector
    cfg_strvec_append_nocopy(field->strvec, tmp);

    return endquote;
}
/* current should point to the opening quote
 * here we return the location of the ending quote
 *
 * We allow empty strings and escaped quote characters
 *
 */
/*
static struct cfg_strvec *copy_quoted_string_as_vec(const struct cfg_string *str,
                                                    size_t current,
                                                    size_t *end,
                                                    enum cfg_status *status)
{
    struct cfg_strvec *vec=NULL;
    char *tmp=NULL;
    tmp = copy_quoted_string(str, current, end, status);
    if (*status == CFG_SUCCESS) {
        vec=cfg_strvec_new();
        // vec will now own the string
        cfg_strvec_append_nocopy(vec, tmp);
    }
    return vec;
}
*/

/*
 * On entry, current should point to the opening array character
 * On exit end will point to the closing character.
 */
static size_t copy_array(struct cfg_field *self,
                         const struct cfg_string *str,
                         size_t current,
                         enum cfg_status *status)
{
    // skip past the opening brace
    current++;

    CFG_FIELD_ELTYPE(self) = CFG_ELTYPE_UNKNOWN;
    while (1) {
        current = find_nonwhite(str, current, status);
        if (*status) { // EOF
            goto _copy_array_bail;
        }
        if (str->data[current] == CFG_ARRAY_SEP) {
            // this is the separator between elements of array, skip
            current++;
            continue;
        }
        if (str->data[current] == CFG_ARRAY_END) {
            break;
        }

        if (str->data[current] == '#') {
            // found a comment, skip to next EOL
            current = find_char(str, current, '\n', status);
            if (*status) { // EOF
                goto _copy_array_bail;
            }
            continue;
        }
        if (str->data[current] == CFG_QUOTE) {
            current = copy_quoted_string(self, str, current, status);
            if (*status) {
                goto _copy_array_bail;
            }

            CFG_FIELD_ELTYPE(self) = CFG_ELTYPE_QUOTED_STRING;
            // skip ending quote
            current ++;
        } else {
            current = copy_bare_array_token(self, str, current, status);
        }
    }

_copy_array_bail:
    if (CFG_ELTYPE_QUOTED_STRING != CFG_FIELD_ELTYPE(self)) {
        // no quoted strings were found, use bare
        CFG_FIELD_ELTYPE(self)=CFG_ELTYPE_BARE;
    }
    return current;
}

static struct cfg *cfg_parse(const struct cfg_string *str, size_t *current, enum cfg_status *status);

static struct cfg_field *cfg_get_field(const struct cfg_string *str, 
                                       size_t current, 
                                       size_t *end, 
                                       enum cfg_status *status) {

    char *name=NULL;
    struct cfg_field *field=NULL;

    // end will be on assignment character
    name=copy_next_identifier(str, current, end, status);
    if (*status) {
        if (CFG_EOF == *status) {
            // no field found at all.  This is OK so we
            // will set status back to success
            *status=CFG_SUCCESS;
            goto _cfg_get_field_bail;
        }
    }

    field=cfg_field_new();
    field->name=name;

    current = *end+1;
    current = find_nonwhite(str, current, status);
    if (*status) { // EOF
        return 0;
    }

    if (str->data[current] == CFG_ARRAY_BEG) {

        // leaves end at array end char
        CFG_FIELD_TYPE(field)=CFG_TYPE_ARRAY;
        *end = copy_array(field, str, current, status);

    } else if (CFG_CFG_BEG == str->data[current]) {

        //fprintf(stderr,"Found begin of sub config: '%s'\n", name);
        CFG_FIELD_TYPE(field)=CFG_TYPE_CFG;
        field->sub = cfg_parse(str, &current, status);
        //*end=current;
        // +1 cause we need to skip the ending brace
        *end=current+1;
    } else if (CFG_CFG_END == str->data[current]) {
        // this never happens
        *status = CFG_FOUND_END;
        *end=current;
    } else if (str->data[current] == CFG_QUOTE) {

        CFG_FIELD_TYPE(field)=CFG_TYPE_SCALAR;
        CFG_FIELD_ELTYPE(field)=CFG_ELTYPE_QUOTED_STRING;
        *end = copy_quoted_string(field, str, current, status);

    } else {

        CFG_FIELD_TYPE(field)=CFG_TYPE_SCALAR;
        CFG_FIELD_ELTYPE(field)=CFG_ELTYPE_BARE;
        //*end = copy_bare_token(field, str, current, status);
        *end = copy_bare_array_token(field, str, current, status);

    }

_cfg_get_field_bail:
    if (*status) {
        field=cfg_field_free(field);
    }
    return field;
}

static struct cfg *cfg_parse(const struct cfg_string *str, size_t *current, enum cfg_status *status) 
{
    struct cfg *cfg=NULL;
    //int beg_found=0;

    struct cfg_field *field=NULL;
    size_t end=0;
    if (!str) {
        *status = CFG_READ_ERROR;
        goto _cfg_parse_bail;
    }

    // beginning { is optional for overall config, but not sub-configs
    if (CFG_CFG_BEG == str->data[*current]) {
        (*current) += 1;
    }
    cfg = cfg_new();
    while (1) {
        *current = find_nonwhite(str, *current, status);
        if (*status) { // EOF OK here
            *status=CFG_SUCCESS;
            break;
        }
        if (str->data[*current] == CFG_CFG_END) {
            break;
        }

        field=cfg_get_field(str, (*current), &end, status);
        if (CFG_FOUND_END == (*status)) {
            // just found end of config, break out
            *status=CFG_SUCCESS;
            break;
        }
        if (*status) {
            // an error occured
            break;
        }
        if (!field) {
            // probably natural end of file found
            break;
        }


        cfg_append(cfg,field);

        if (str->data[end] == CFG_CFG_END) {
            *current = end;
            break;
        }

        // skip past either the last letter, end quote, end array delim, etc.
        (*current) = end+1;
    }

_cfg_parse_bail:
    if (*status) {
        cfg=cfg_free(cfg);
    }
    return cfg;
};

/* Public
 *
 * Read and parse the config
 */
struct cfg *cfg_read(const char* filename, enum cfg_status *status) 
{
    struct cfg *cfg=NULL;
    struct cfg_string *str=NULL;
    size_t current=0;

    str = read_whole_file(filename);
    if (!str) {
        *status = CFG_READ_ERROR;
        return NULL;
    }

    cfg=cfg_parse(str, &current, status);
    str=cfg_string_free(str);
    return cfg;
}


/* Returns a const reference to the field if found, otherwise NULL */
static const struct cfg_field *cfg_find(const struct cfg *self, 
                                        const char* name, 
                                        enum cfg_status *status)
{
    size_t i=0;
    const struct cfg_field *field=NULL, *tmp=NULL;

    *status=CFG_NOT_FOUND;
    if (!self) {
        return field;
    }
    for (i=0; i<CFG_SIZE(self); i++) {
        tmp = CFG_FIELD(self,i);
        if (0 == strcmp(name, CFG_FIELD_NAME(tmp))) {
            *status=CFG_SUCCESS;
            field=tmp;
            break;
        }
    }

    return field;
}

static double extract_double(const char *str, enum cfg_status *status)
{
    char *endptr=NULL;
    double val=0;

    *status = CFG_SUCCESS;

    endptr=(char*) str;
    val = strtod(str, &endptr);
    if (endptr == str) {
        fprintf(stderr,"Failed to convert data to a double: '%s'\n", str);
        *status=CFG_TYPE_ERROR;
    }
    return val;
}
static double *extract_dblarr(const struct cfg_field *self,
                              size_t *size,
                              enum cfg_status *status)
{
    double *out=NULL, tmp=0;
    struct cfg_strvec *vec=NULL;
    size_t i=0;

    *size=0;

    *status = CFG_SUCCESS;
    if (!self) {
        // not OK to have null field!
        *status = CFG_EMPTY;
        goto _extract_dblarr_bail;
    }

    vec = CFG_FIELD_GET_VEC(self);

    if (!vec || vec->size==0) {
        // ok to have empty array, just exit
        goto _extract_dblarr_bail;
    }

    out = calloc(vec->size, sizeof(double));

    if (!out){
        fprintf(stderr,"Failed to allocate double array copy for field '%s'\n", 
                CFG_FIELD_NAME(self));
        exit(1);
    }

    for (i=0; i<vec->size; i++) {
        tmp = extract_double(vec->data[i], status);
        if (*status) {
            goto _extract_dblarr_bail;
        }
        out[i] = tmp;
    }
    *size = vec->size;

_extract_dblarr_bail:
    if (*status) {
        free(out);
        out=NULL;
    }

    return out;
}


static long extract_long(const char *str, enum cfg_status *status)
{
    char *endptr=NULL;
    long val=0;

    *status = CFG_SUCCESS;

    endptr=(char*) str;
    val = strtol(str, &endptr, 10);
    if (endptr == str) {
        fprintf(stderr,"Failed to convert data to a long : '%s'\n", str);
        *status=CFG_TYPE_ERROR;
    }
    return val;
}

static long *extract_lonarr(const struct cfg_field *self,
                            size_t *size,
                            enum cfg_status *status)
{
    long *out=NULL, tmp=0;
    struct cfg_strvec *vec=NULL;
    size_t i=0;

    *size=0;

    *status = CFG_SUCCESS;
    if (!self) {
        // not OK to have null field!
        *status = CFG_EMPTY;
        goto _extract_lonarr_bail;
    }

    vec = CFG_FIELD_GET_VEC(self);

    if (!vec || vec->size==0) {
        // ok to have empty array, just exit
        goto _extract_lonarr_bail;
    }

    out = calloc(vec->size, sizeof(long));

    if (!out){
        fprintf(stderr,"Failed to allocate long array copy for field '%s'\n", 
                CFG_FIELD_NAME(self));
        exit(1);
    }

    for (i=0; i<vec->size; i++) {
        tmp = extract_long(vec->data[i], status);
        if (*status) {
            goto _extract_lonarr_bail;
        }
        out[i] = tmp;
    }
    *size = vec->size;

_extract_lonarr_bail:
    if (*status) {
        free(out);
        out=NULL;
    }

    return out;
}


/*
 * Public
 *
 * Extract a scalar as a double
 */
double cfg_get_double(const struct cfg *self,
                      const char *name,
                      enum cfg_status *status)
{
    double val=0;
    const struct cfg_field *field=NULL;
    char *str=NULL;

    field = cfg_find(self, name, status);
    if (*status==CFG_SUCCESS) {
        if (CFG_TYPE_SCALAR != CFG_FIELD_TYPE(field)) {
            *status = CFG_TYPE_ERROR;
        } else {
            str = CFG_FIELD_GET_DATA(field, 0);
            val = extract_double(str, status);
        }
    }
    return val;
}
/*
 * Public
 *
 * Extract an array as an array of doubles
 */
double *cfg_get_dblarr(const struct cfg *self,
                       const char *name,
                       size_t *size,
                       enum cfg_status *status)
{
    double *arr=NULL;
    const struct cfg_field *field=NULL;

    field = cfg_find(self, name, status);
    if (*status==CFG_SUCCESS) {
        if (CFG_TYPE_ARRAY != CFG_FIELD_TYPE(field)) {
            *status = CFG_TYPE_ERROR;
        } else {
            arr = extract_dblarr(field, size, status);
        }
    }
    return arr;
}

/*
 * Public
 *
 * Extract a scalar as a long
 */
long cfg_get_long(const struct cfg *self,
                  const char *name,
                  enum cfg_status *status)
{
    long val=0;
    const struct cfg_field *field=NULL;
    char *str=NULL;

    field = cfg_find(self, name, status);
    if (*status==CFG_SUCCESS) {
        if (CFG_TYPE_SCALAR != CFG_FIELD_TYPE(field)) {
            *status = CFG_TYPE_ERROR;
        } else {
            str = CFG_FIELD_GET_DATA(field, 0);
            val = extract_long(str, status);
        }
    }
    return val;
}
/*
 * Public
 *
 * Extract an array as an array of longs
 */
long *cfg_get_lonarr(const struct cfg *self,
                       const char *name,
                       size_t *size,
                       enum cfg_status *status)
{
    long *arr=NULL;
    const struct cfg_field *field=NULL;

    field = cfg_find(self, name, status);
    if (*status==CFG_SUCCESS) {
        if (CFG_TYPE_ARRAY != CFG_FIELD_TYPE(field)) {
            *status = CFG_TYPE_ERROR;
        } else {
            arr = extract_lonarr(field, size, status);
        }
    }
    return arr;
}


/*
 * Public
 *
 * Extract a scalar as a string
 */
char *cfg_get_string(const struct cfg *self,
                     const char *name,
                     enum cfg_status *status)
{
    char *str=NULL;
    const struct cfg_field *field=NULL;

    field = cfg_find(self, name, status);
    if (*status==CFG_SUCCESS) {
        if (CFG_TYPE_SCALAR != CFG_FIELD_TYPE(field)) {
            *status = CFG_TYPE_ERROR;
        } else {
            if (0==CFG_FIELD_ARR_SIZE(field)) {
                *status = CFG_EMPTY;
            } else {
                str = cfg_strdup( CFG_FIELD_GET_DATA(field, 0) );
            }
        }
    }
    return str;
}

/*
 * Public
 *
 * Extract an array as an array of strings
 *
 * You can use cfg_strarr_free convenience function to free this
 */
char **cfg_get_strarr(const struct cfg *self,
                      const char *name,
                      size_t *size,
                      enum cfg_status *status)
{
    char **sarr=NULL;
    const struct cfg_field *field=NULL;

    field = cfg_find(self, name, status);
    if (*status==CFG_SUCCESS) {
        if (CFG_TYPE_ARRAY != CFG_FIELD_TYPE(field)) {
            *status = CFG_TYPE_ERROR;
        } else {
            sarr = cfg_strvec2arr(CFG_FIELD_GET_VEC(field), size);
            if (!sarr) {
                *status=CFG_EMPTY;
            }
        }
    }
    return sarr;
}

/*
 * Public
 *
 * Get a sub-config
 */

struct cfg *cfg_get_sub(const struct cfg *self,
                        const char *name,
                        enum cfg_status *status)
{
    struct cfg *sub=NULL;
    const struct cfg_field *field=NULL;

    field = cfg_find(self, name, status);
    if (*status==CFG_SUCCESS) {
        if (CFG_TYPE_CFG != CFG_FIELD_TYPE(field)) {
            *status = CFG_TYPE_ERROR;
        } else {
            sub = CFG_FIELD_GET_SUB(field);
            if (!sub) {
                *status=CFG_EMPTY;
            }
        }
    }
    return sub;
}
