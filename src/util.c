#include <stdio.h>
#include <alloca.h>
#include <math.h>

void comma_print(FILE* stream, long long n)
{
    int i = -1, len, negative = 0;
    char *c=NULL, *p=NULL;

    if (n == 0) {
        len = 2;
    } else if (n < 0) {
        negative = 1;
        n = -n;
        len = 4 * floor(log10(n)) / 3 + 3;
    } else {
        len = 4 * floor(log10(n)) / 3 + 2;
    }

    c = alloca(len);
    p = c + len - 1;
    *p-- = 0;

    do {
        if (++i == 3) {
            *p-- = ',';
            i = 0;
        }
        *p-- = '0' + n % 10;
        n /= 10;
    } while (n);

    if (negative) *p = '-';

    fprintf(stream, "%s", c);
}


