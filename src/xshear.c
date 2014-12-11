#include <stdlib.h>
#include <stdio.h>
#include "source.h"
#include "shear.h"
#include "log.h"
#include "defs.h"
#include "vector.h"

void usage_and_exit(void) {
    wlog("usage: \n");
    wlog("   cat source_cat | xshear config_file lens_cat > outfile\n");
    wlog("   cat source_cat1 source_cat2 .. | xshear config_file lens_cat > outfile\n");
    wlog("   cat source_cat | some_filter | xshear config_file lens_cat > outfile\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char** argv) {
    int64 nsource=0;

    if (argc < 3) {
        usage_and_exit();
    }

    const char* config_url=argv[1];
    const char* lens_url=argv[2];
    Shear* shear=shear_init(config_url, lens_url);

    Source* src=source_new(shear->config);
    src->zlens = (const dvector*) shear->config->zl;

    while (source_read(stdin, src)) {
        nsource++;

        if (src->scstyle == SIGMACRIT_STYLE_POINT) {
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
    lensums_write(shear->lensums, stdout);

    src=source_delete(src);
    shear=shear_delete(shear);
    wlog("Done\n");

    return 0;
}
