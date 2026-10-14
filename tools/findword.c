/* findword: list offsets (4-byte aligned or not) where an int32 value occurs, with the gap to the previous hit. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
int main(int argc, char **argv) {
  if (argc != 3) { fprintf(stderr, "usage: %s file value\n", argv[0]); return 2; }
  FILE *f = fopen(argv[1], "rb"); if (!f) { perror("open"); return 1; }
  fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
  unsigned char *d = malloc(n); if (fread(d, 1, n, f) != (size_t)n) return 1; fclose(f);
  int32_t v = atoi(argv[2]); long prev = -1; int count = 0;
  for (long i = 0; i + 4 <= n; i++) {
    int32_t w; memcpy(&w, d + i, 4);
    if (w == v) { printf("%ld%s gap=%ld\n", i, (i % 4) ? "(unaligned)" : "", prev < 0 ? 0 : i - prev); prev = i; if (++count >= 40) break; }
  }
  printf("hits shown: %d\n", count); return 0;
}
