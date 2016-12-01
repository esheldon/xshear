#include <stdlib.h>
#include <stdio.h>
#include "lensum.h"
#include "sconfig.h"
#include "log.h"
#include "defs.h"
#include "lens.h"
#include "util.h"

#include "uthash.h"

#define HASH_FIND_INT64(head,findint64,out)        \
    HASH_FIND(hh,head,findint64,sizeof(int64),out)
#define HASH_ADD_INT64(head,int64field,add)        \
    HASH_ADD(hh,head,int64field,sizeof(int64),add)


typedef struct {
    Lensum* lensum;
    UT_hash_handle hh; /* makes this structure hashable */
} LensumHash;

LensumHash* lensum_hash_fromlensum(Lensum* lensum) {
    LensumHash* lh = calloc(1, sizeof(LensumHash));
    lh->lensum = lensum_copy(lensum);
    return lh;
}
LensumHash* find_lens(LensumHash* lensums, int64 id) {
    // a single file reference, don't allocate
    LensumHash* alensum=NULL;
    HASH_FIND_INT64(lensums, &id, alensum);
    return alensum;
}


void usage_and_exit(void) {
    wlog("usage: \n");
    wlog("   cat lensum_file1 lensum_file2 ... | redshear config_file > lensums_total\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int64 counter=0;
    if (argc < 2) {
        usage_and_exit();
    }

    const char* config_url = argv[1];
    ShearConfig* config=sconfig_read(config_url);

    // this is the beginning of the table
    LensumHash* hash = NULL;

    Lensum* lensum = lensum_new(config->nbin, config->shear_style, config->scstyle);
    Lensum* lensum_tot = lensum_new(config->nbin, config->shear_style, config->scstyle);
    while (lensum_read_into(lensum, stdin)) {
        counter++;
        if (counter == 1) {
            wlog("first lensum: %ld %.8g %ld\n", 
                 lensum->index, lensum->weight, 
                 lensum->totpairs);
        }
        if ((counter % 10000) == 0) {
            wlog(".");
        }
        if (((counter+1) % 1000000) == 0) {
            wlog("\n");
            comma_print(stderr,counter+1);
            wlog("\n");
        }

        LensumHash* this_lens = find_lens(hash, lensum->index);
        if (this_lens == NULL) {
            // copy of lensum made inside
            LensumHash* lh = lensum_hash_fromlensum(lensum);
            // this gets expanded to lh->lensum->index
            HASH_ADD_INT64(hash, lensum->index, lh);
        } else {
            lensum_add(this_lens->lensum, lensum);
        }
        lensum_add(lensum_tot, lensum);

    }

    wlog("\nlast lensum: %ld %.8g %ld\n", 
            lensum->index, lensum->weight, lensum->totpairs);

    wlog("Read a total of %ld\n", counter);

    // this is the summary
    lensum_print(lensum_tot);

    wlog("Writing results to stdout\n");
    LensumHash *tlensum=NULL;
    for(tlensum=hash; tlensum != NULL; tlensum=tlensum->hh.next) {
        lensum_write(tlensum->lensum, stdout);
    }

    wlog("Done\n");

    return 0;
}
