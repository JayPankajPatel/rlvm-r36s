/* clusters: list non-zero clusters (merging gaps < GAP bytes) in a region, with word counts. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: %s file [gap] [from] [to]\n", argv[0]); return 2; }
  long gap = argc > 2 ? atol(argv[2]) : 128;
  FILE *f = fopen(argv[1], "rb"); if (!f) { perror("open"); return 1; }
  fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
  unsigned char *d = malloc(n); if (fread(d, 1, n, f) != (size_t)n) return 1; fclose(f);
  long from = argc > 3 ? atol(argv[3]) : 0, to = argc > 4 ? atol(argv[4]) : n;
  long start = -1, last = -1, nz = 0;
  for (long i = from; i <= to; i += 4) {
    int32_t v = 0; if (i + 4 <= n) memcpy(&v, d + i, 4);
    if (v != 0) { if (start < 0) { start = i; nz = 0; } last = i; nz++; }
    if (start >= 0 && (i >= to || (v == 0 && i - last > gap))) {
      printf("  %7ld..%7ld  %6ld bytes  %5ld nonzero words\n", start, last + 4, last + 4 - start, nz);
      start = -1;
    }
  }
  return 0;
}
