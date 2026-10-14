/* rlunpack: decompress a RealLive-compressed block (as used by Seen.txt scenes and save files).
 * Block layout at OFFSET: int32 compressed_size, int32 uncompressed_size, then the stream. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "xor_mask.h"

int main(int argc, char **argv) {
  if (argc < 4) { fprintf(stderr, "usage: %s in.sav offset out.bin [nomask]\n", argv[0]); return 2; }
  int use_mask = !(argc > 4 && strcmp(argv[4], "nomask") == 0);
  FILE *f = fopen(argv[1], "rb"); if (!f) { perror("open"); return 1; }
  fseek(f, 0, SEEK_END); long n = ftell(f);
  long off = atol(argv[2]);
  unsigned char *src = malloc(n - off);
  fseek(f, off, SEEK_SET);
  if (fread(src, 1, n - off, f) != (size_t)(n - off)) { perror("read"); return 1; }
  fclose(f);
  int32_t csize, usize; memcpy(&csize, src, 4); memcpy(&usize, src + 4, 4);
  fprintf(stderr, "compressed=%d uncompressed=%d (available %ld)\n", csize, usize, n - off);
  unsigned char *dst = calloc(usize, 1);
  long si = 8, di = 0; unsigned mask = 8; int bit = 1;
  long send = n - off < csize ? n - off : csize;
  unsigned char flag = src[si++] ^ (use_mask ? xor_mask[mask++ & 255] : 0);
  while (si < send && di < usize) {
    if (bit == 256) { bit = 1; flag = src[si++] ^ (use_mask ? xor_mask[mask++ & 255] : 0); }
    if (flag & bit) {
      dst[di++] = src[si++] ^ (use_mask ? xor_mask[mask++ & 255] : 0);
    } else {
      int count = src[si++] ^ (use_mask ? xor_mask[mask++ & 255] : 0);
      count += (src[si++] ^ (use_mask ? xor_mask[mask++ & 255] : 0)) << 8;
      long rep = di - ((count >> 4) - 1) - 1;
      count = (count & 0x0f) + 2;
      if (rep < 0 || rep >= di) { fprintf(stderr, "corrupt data at out=%ld in=%ld\n", di, si); return 1; }
      for (int i = 0; i < count && di < usize; i++) dst[di++] = dst[rep++];
    }
    bit <<= 1;
  }
  fprintf(stderr, "produced %ld of %d bytes\n", di, usize);
  FILE *o = fopen(argv[3], "wb"); if (!o) { perror("out"); return 1; }
  fwrite(dst, 1, di, o); fclose(o);
  return di == usize ? 0 : 3;
}
