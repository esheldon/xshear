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
    wlog("   cat source_cat | xshear config_file lens_cat pair_logfile > outfile\n");
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
    
    if(shear->config->rbin_print_max>0) { // print pair ids to file
      if (argc < 4) {
	wlog("no pair logfile given\n");
        usage_and_exit();
      }
      sconfig_open_pair_url(shear->config,argv[3]);
      fprintf(shear->config->pair_fd, "# lens_id source_id r_bin weight\n");
    }

    Source* src=source_new(shear->config);
    src->zlens = (const dvector*) shear->config->zl;

    while (source_read(stdin, src)) {
        nsource++;

        if (src->scstyle == SIGMACRIT_STYLE_POINT) {
            src->dc = Dc(shear->cosmo, 0.0, src->z);
        } else if (src->scstyle == SIGMACRIT_STYLE_SAMPLE) {
            src->dc  = Dc(shear->cosmo, 0.0, src->z);
            src->dcs = Dc(shear->cosmo, 0.0, src->zs);
        }

        if (nsource == 1) {
            wlog("first source:\n");
            source_print(src);
        }
        if (nsource == 2) {
            wlog("second source:\n");
            source_print(src);
        }
        if ((nsource % 10000) == 0) {
            wlog(".");
        }


        shear_process_source(shear, src);
    }
    wlog("\nlast source:\n");
    source_print(src);

    wlog("processed %lld sources\n", nsource);

    // print some summary info
    shear_print_sum(shear);

    wlog("Writing results to stdout\n");
    lensums_write(shear->lensums, stdout);

    if(shear->config->rbin_print_max>0) {
     fclose(shear->config->pair_fd);
    }
    
    src=source_free(src);
    shear=shear_free(shear);
    wlog("Done\n");

    return 0;
}
