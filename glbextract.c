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

uint32_t decrypt_uint32(struct State *state, uint32_t data32)
{
  uint8_t *current_byte;
  uint8_t *prev_byte;
  uint8_t *key_pos;
  uint8_t *data8;

  current_byte = &state->current_byte;
  prev_byte = &state->prev_byte;
  key_pos = &state->key_pos;
  data8 = (char *)&data32;

  for(int i = 0; i < READ8_MAX; i++) {
    *current_byte = data8[i];
    data8[i] = (*current_byte) - *(DEFAULT_KEY + *key_pos) - (*prev_byte);
    data8[i] &= 0xFF;
    *key_pos = ++(*key_pos) % strlen(DEFAULT_KEY);
    *prev_byte = (*current_byte);
  }

  return data32;
}

struct Buffer *mem_buffer_init(const char *path)
{
  FILE *glb;
  struct stat st;
  struct Buffer *mem_buffer;

  mem_buffer = malloc(sizeof(*mem_buffer) );
  glb = fopen(path, "rb");
  stat(path, &st);

  mem_buffer->length =  st.st_size;
  mem_buffer->position = 0;
  mem_buffer->buffer = malloc(st.st_size);

  fread(mem_buffer->buffer, 1, mem_buffer->length, glb);
  fclose(glb);

  return mem_buffer;
}

void mem_buffer_free(struct Buffer **mem_buffer)
{
  free( (*mem_buffer)->buffer);
  free(*mem_buffer);
  *mem_buffer = NULL;

  return;
}

size_t mem_buffer_read(struct Buffer *mem_buffer, void *dest, size_t nmemb)
{
  size_t retval;

  retval = 0;

  if(mem_buffer->position + nmemb < mem_buffer->length) {
    memcpy(dest, mem_buffer->buffer + mem_buffer->position, nmemb);

    mem_buffer->position += nmemb;
    retval = nmemb;
  }

  return retval;
}

int mem_buffer_relative_seek(struct Buffer *mem_buffer, long long target)
{
  if(mem_buffer->position + target > mem_buffer->length ||
      (target > mem_buffer->position && target < 0) ) {
    return -1;
  }

  mem_buffer->position += target;

  return 0;
}

int mem_buffer_absolute_seek(struct Buffer *mem_buffer, size_t target)
{
  if(mem_buffer->position + target > mem_buffer->length) return -1;

  mem_buffer->position = target;

  return 0;
}

int main(void)
{
  FILE *glb;

  glb = fopen("FILE0000.GLB", "rb");

  struct FATable fat = {0};
  struct State state = {0};
  struct stat st;
  union Bytes bytes = {0};

  size_t fsize;
  size_t fpos;

  uint8_t *buffer;

  stat("FILE0000.GLB", &st);

  fsize = st.st_size;
  fpos = 0;
  buffer = calloc(fsize, sizeof(*buffer) );
  fread(buffer, 1, fsize, glb);
  fclose(glb);

  memcpy(bytes.read8, buffer + fpos, READ8_MAX);
  fpos += READ8_MAX;

  /* First 4 bytes of file are always the same. They first 4 bytes are always
   * zero, but because they're encrypted, they always have the specific bit
   * pattern defined in RAW_HEADER
   */

  if(strncmp(RAW_HEADER, bytes.read8, RAW_HEADER_LEN) ) {
    printf("Not a GLB file!\n");
    return 1;
  }

  state.key_pos = calculate_key_pos(strlen(DEFAULT_KEY) );
  state.key_pos += READ8_MAX;
  state.prev_byte = bytes.read8[READ8_MAX - 1];

  /* We don't bother decrypting the previous 4 bytes, since we know they're all
   * zeroed out
   */

  memcpy(bytes.read8, buffer + fpos, READ8_MAX );
  fpos += READ8_MAX;

  for(int i = 0; i < strlen(DEFAULT_KEY); i++)
    printf("%x\n", DEFAULT_KEY[i]);

  bytes.read32 = decrypt_uint32(&state, bytes.read32);

  printf("Found %d files\n", bytes.read32);

  free(buffer);

  return 0;
}
