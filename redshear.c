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


struct lensum_hash {
    struct lensum* lensum;
    UT_hash_handle hh; /* makes this structure hashable */
};

struct lensum_hash* lensum_hash_fromlensum(struct lensum* lensum) {
    struct lensum_hash* lh = calloc(1, sizeof(struct lensum_hash));
    lh->lensum = lensum_copy(lensum);
    return lh;
}
struct lensum_hash* find_lens(struct lensum_hash* lensums, int64 id) {
    // a single file reference, don't allocate
    struct lensum_hash* alensum=NULL;
    HASH_FIND_INT64(lensums, &id, alensum);
    return alensum;
}


void usage_and_exit(void) {
    wlog("usage: redshear [config_url]\n");
    wlog("  If config_url is not sent as an argument, the CONFIG_URL env variable is used\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int64 counter=0;
    if (argc < 2) {
        usage_and_exit();
    }

    const char* config_url = argv[1];
    struct sconfig* config=sconfig_read(config_url);

    // this is the beginning of the table
    struct lensum_hash* hash = NULL;

    struct lensum* lensum = lensum_new(config->nbin);
    struct lensum* lensum_tot = lensum_new(config->nbin);
    while (lensum_read(stdin, lensum)) {
        counter++;
        if (counter == 1) {
            wlog("first lensum: %ld %ld %.8g %ld\n", 
                 lensum->index, lensum->zindex, lensum->weight, 
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

        struct lensum_hash* this_lens = find_lens(hash, lensum->zindex);
        if (this_lens == NULL) {
            // copy of lensum made inside
            struct lensum_hash* lh = lensum_hash_fromlensum(lensum);
            // this gets expanded to lh->lensum->zindex
            HASH_ADD_INT64(hash, lensum->zindex, lh);
        } else {
            lensum_add(this_lens->lensum, lensum);
        }
        lensum_add(lensum_tot, lensum);

    }

    wlog("\nlast lensum: %ld %ld %.8g %ld\n", 
            lensum->index, lensum->zindex, lensum->weight, lensum->totpairs);

    wlog("Read a total of %ld\n", counter);

    // this is the summary
    lensum_print(lensum_tot);

    wlog("Writing results to stdout\n");
    struct lensum_hash *tlensum=NULL;
    for(tlensum=hash; tlensum != NULL; tlensum=tlensum->hh.next) {
        lensum_write(tlensum->lensum, stdout);
    }

    wlog("Done\n");

    return 0;
}
