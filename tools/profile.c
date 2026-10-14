/* profile: classify a file in fixed windows by what the int32 words look like. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: %s file [window] [start]\n", argv[0]); return 2; }
  long win = argc > 2 ? atol(argv[2]) : 4096, start = argc > 3 ? atol(argv[3]) : 0;
  FILE *f = fopen(argv[1], "rb"); if (!f) { perror("open"); return 1; }
  fseek(f, 0, SEEK_END); long n = ftell(f); rewind(f);
  unsigned char *d = malloc(n); if (fread(d, 1, n, f) != (size_t)n) return 1; fclose(f);
  printf("offset   zero%% small%% ascii%% bgra-ish%%  (words: zero=0, small=|v|<65536, ascii=all 4 bytes printable, bgra=alpha byte 0xff)\n");
  for (long o = start; o + win <= n + win - 1 && o < n; o += win) {
    long e = o + win > n ? n : o + win, words = 0, z = 0, s = 0, a = 0, b = 0;
    for (long i = o; i + 4 <= e; i += 4) {
      int32_t v; memcpy(&v, d + i, 4); words++;
      if (v == 0) z++;
      else if (v > -65536 && v < 65536) s++;
      int pr = 1; for (int k = 0; k < 4; k++) if (d[i + k] < 32 || d[i + k] > 126) pr = 0;
      if (pr) a++;
      if (d[i + 3] == 0xff) b++;
    }
    printf("%7ld  %4ld  %4ld  %4ld  %4ld\n", o, 100 * z / words, 100 * s / words, 100 * a / words, 100 * b / words);
  }
  return 0;
}
