#include <stdlib.h>
#include <stdio.h>
#include "source.h"
#include "shear.h"
#include "log.h"
#include "defs.h"

void usage_and_exit(void) {
    wlog("usage: sobjshear-gsens config_url lens_cat\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char** argv) {
    int64 nsource=0;

    if (argc < 3) {
        usage_and_exit();
    }

    const char* config_url=argv[1];
    const char* lens_url=argv[2];
    struct shear* shear=shear_init(config_url, lens_url);

    struct source* src=source_new(shear->config);
    src->zlens = shear->config->zl;

    while (source_read(stdin, src)) {
        nsource++;

        if (src->scstyle == SCSTYLE_TRUE) {
            src->dc = Dc(shear->cosmo, 0.0, src->z);
        }

        if (nsource == 1) {
            wlog("first source:\n");
            source_print(src);
        }
        if ((nsource % 10000) == 0) {
            wlog(".");
        }


        shear_process_source(shear, src);
    }
    wlog("\nlast source:\n");
    source_print(src);

    wlog("processed %ld sources\n", nsource);

    // print some summary info
    shear_print_sum(shear);

    wlog("Writing results to stdout\n");
    shear_write(shear, stdout);

    src=source_delete(src);
    shear=shear_delete(shear);
    wlog("Done\n");

    return 0;
}
