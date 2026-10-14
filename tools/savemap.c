/* savemap: print the coarse structure of a RealLive save file.
 * Splits the file into zero runs (>= MINZ bytes) and everything else, and
 * shows the first few int32 words of each non-zero segment. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MINZ 256

int main(int argc, char **argv) {
  if (argc != 2) { fprintf(stderr, "usage: %s save.sav\n", argv[0]); return 2; }
  FILE *f = fopen(argv[1], "rb");
  if (!f) { perror("open"); return 1; }
  fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
  unsigned char *d = malloc(n);
  if (fread(d, 1, n, f) != (size_t)n) { perror("read"); return 1; }
  fclose(f);
  printf("%s: %ld bytes\n", argv[1], n);
  long i = 0, seg = 0;
  while (i < n) {
    long j = i;
    if (d[i] == 0) {                       /* zero run */
      while (j < n && d[j] == 0) j++;
      if (j - i >= MINZ) { printf("  zeros   %7ld..%7ld  (%ld bytes)\n", i, j, j - i); i = j; continue; }
    }
    /* data segment: until a long zero run starts */
    j = i;
    while (j < n) {
      if (d[j] == 0) {
        long k = j; while (k < n && d[k] == 0) k++;
        if (k - j >= MINZ) break;
        j = k;
      } else j++;
    }
    printf("  data    %7ld..%7ld  (%ld bytes)  first words:", i, j, j - i);
    for (long w = i; w + 4 <= j && w < i + 24; w += 4) printf(" %d", *(int32_t *)(d + w));
    printf("\n");
    i = j; seg++;
  }
  free(d);
  return 0;
}
