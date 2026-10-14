/* steam2resume: turn a Steam RealLive saveNNN.sav into a small "resume" file for rlvm.
 *
 * Output layout (little endian int32):
 *   magic 'RSM1', scene, byte offset in scene bytecode, then 6 banks (A..F) of 2000 ints.
 *
 * Save layout (found by reverse engineering, see README): a ~2 KB header, then at
 * offset 2024 a RealLive-LZ block (no xor mask) that unpacks to a ~2.5 MB memory image.
 * In the image: scene number at SCENE_OFF, program offset at SCENE_OFF+8, and the
 * local int banks A..F back to back at BANKS_OFF (same order as rlvm's LocalMemory). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_OFF 2024
#define SCENE_OFF 92588
#define BANKS_OFF 5848
#define BANK_INTS 2000
#define NBANKS 6

static int unpack(const unsigned char* src, long avail, unsigned char** out, long* outlen) {
  int32_t csize, usize;
  memcpy(&csize, src, 4);
  memcpy(&usize, src + 4, 4);
  if (usize <= 0 || usize > (64 << 20))
    return -1;
  unsigned char* dst = calloc((size_t)usize, 1);
  long si = 8, di = 0;
  int bit = 1;
  long send = avail < csize ? avail : csize;
  unsigned flag = src[si++];
  while (si < send && di < usize) {
    if (bit == 256) {
      bit = 1;
      flag = src[si++];
    }
    if (flag & (unsigned)bit) {
      dst[di++] = src[si++];
    } else {
      int count = src[si++];
      count += src[si++] << 8;
      long rep = di - ((count >> 4) - 1) - 1;
      count = (count & 0x0f) + 2;
      if (rep < 0 || rep >= di)
        return -1;
      for (int i = 0; i < count && di < usize; i++)
        dst[di++] = dst[rep++];
    }
    bit <<= 1;
  }
  *out = dst;
  *outlen = di;
  return di == usize ? 0 : -1;
}

int main(int argc, char** argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s saveNNN.sav out.resume\n", argv[0]);
    return 2;
  }
  FILE* f = fopen(argv[1], "rb");
  if (!f) {
    perror(argv[1]);
    return 1;
  }
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  if (n <= BLOCK_OFF + 8) {
    fprintf(stderr, "file too small\n");
    return 1;
  }
  unsigned char* src = malloc((size_t)(n - BLOCK_OFF));
  fseek(f, BLOCK_OFF, SEEK_SET);
  if (fread(src, 1, (size_t)(n - BLOCK_OFF), f) != (size_t)(n - BLOCK_OFF)) {
    perror("read");
    return 1;
  }
  fclose(f);

  unsigned char* img;
  long len;
  if (unpack(src, n - BLOCK_OFF, &img, &len) != 0) {
    fprintf(stderr, "decompression failed\n");
    return 1;
  }
  const long need = BANKS_OFF + (long)NBANKS * BANK_INTS * 4;
  if (len < SCENE_OFF + 12 || len < need) {
    fprintf(stderr, "image too small\n");
    return 1;
  }

  int32_t head[3];
  memcpy(&head[0], "RSM1", 4);
  memcpy(&head[1], img + SCENE_OFF, 4);
  memcpy(&head[2], img + SCENE_OFF + 8, 4);

  FILE* o = fopen(argv[2], "wb");
  if (!o) {
    perror(argv[2]);
    return 1;
  }
  fwrite(head, 4, 3, o);
  fwrite(img + BANKS_OFF, 4, (size_t)NBANKS * BANK_INTS, o);
  fclose(o);
  fprintf(stderr, "scene %d offset %d\n", head[1], head[2]);
  return 0;
}
