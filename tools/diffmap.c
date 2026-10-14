/* diffmap: show where two equal-sized files differ, merging nearby changes into ranges. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static unsigned char *load(const char *p, long *n) {
  FILE *f = fopen(p, "rb"); if (!f) { perror(p); exit(1); }
  fseek(f, 0, SEEK_END); *n = ftell(f); rewind(f);
  unsigned char *d = malloc(*n); if (fread(d, 1, *n, f) != (size_t)*n) exit(1); fclose(f); return d;
}
int main(int argc, char **argv) {
  if (argc < 3) { fprintf(stderr, "usage: %s a b [gap] [max_ranges]\n", argv[0]); return 2; }
  long na, nb; unsigned char *a = load(argv[1], &na), *b = load(argv[2], &nb);
  if (na != nb) { fprintf(stderr, "size differs: %ld vs %ld\n", na, nb); return 1; }
  long gap = argc > 3 ? atol(argv[3]) : 32, maxr = argc > 4 ? atol(argv[4]) : 60;
  long total = 0, ranges = 0, start = -1, last = -1;
  for (long i = 0; i <= na; i++) {
    int diff = i < na && a[i] != b[i];
    if (diff) { total++; if (start < 0) start = i; last = i; }
    if (start >= 0 && (i == na || (!diff && i - last > gap))) {
      ranges++;
      if (ranges <= maxr) {
        printf("  %7ld..%7ld (%4ld bytes)  words a:", start, last + 1, last + 1 - start);
        for (long w = start & ~3L; w < last + 1 && w < (start & ~3L) + 20; w += 4) { int32_t v; memcpy(&v, a + w, 4); printf(" %d", v); }
        printf("  b:");
        for (long w = start & ~3L; w < last + 1 && w < (start & ~3L) + 20; w += 4) { int32_t v; memcpy(&v, b + w, 4); printf(" %d", v); }
        printf("\n");
      }
      start = -1;
    }
  }
  printf("total changed bytes: %ld in %ld ranges\n", total, ranges);
  return 0;
}
