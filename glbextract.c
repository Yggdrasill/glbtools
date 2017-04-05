#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "glbextract.h"

int calculate_key_pos(size_t len)
{
  return 25 % len;
}

int main(void)
{
  FILE *glb;

  glb = fopen("FILE0000.GLB", "rb");

  struct FATable fat = {0};
  struct stat st;
  union Bytes bytes = {0};

  size_t fsize;
  size_t fpos;

  uint8_t *buffer;
  uint8_t prev_byte;
  uint8_t key_pos;

  stat("FILE0000.GLB", &st);

  filesize = st.st_size;
  position = 0;
  buffer = calloc(filesize, sizeof(*buffer) );
  fread(buffer, 1, filesize, glb);
  fclose(glb);

  memcpy(bytes.read8, buffer, 4);

  /* First 4 bytes of file are always the same. They first 4 bytes are always
   * zero, but because they're encrypted, they always have the specific bit
   * pattern defined in RAW_HEADER
   */

  if(strncmp(RAW_HEADER, bytes.read8, RAW_HEADER_LEN) ) {
    printf("Not a GLB file!\n");
    return 1;
  }

  free(buffer);

  return 0;
}
