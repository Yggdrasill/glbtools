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

void reset_state(struct State *state)
{
  state->key_pos = calculate_key_pos(strlen(DEFAULT_KEY) );
  state->prev_byte = DEFAULT_KEY[state->key_pos];

  return;
}

static int decrypt_varlen(struct State *state, void *data, size_t size)
{
  uint8_t *current_byte;
  uint8_t *prev_byte;
  uint8_t *key_pos;
  uint8_t *byte_data;

  current_byte = &state->current_byte;
  prev_byte = &state->prev_byte;
  key_pos = &state->key_pos;
  byte_data = (uint8_t *)data;

  int i;
  for(i = 0; i < size; i++) {
    *current_byte = byte_data[i];
    byte_data[i] = *current_byte - DEFAULT_KEY[*key_pos] - *prev_byte;
    byte_data[i] &= 0xFF;
    (*key_pos)++;
    *key_pos %= strlen(DEFAULT_KEY);
    *prev_byte = *current_byte;
  }

  return i;
}

int decrypt_uint32(struct State *state, uint32_t *data32)
{
  return decrypt_varlen(state, data32, READ8_MAX);
}

int decrypt_filename(struct State *state, char *str)
{
  *(str + MAX_FILENAME_LEN - 1) = '\0';
  return decrypt_varlen(state, str, MAX_FILENAME_LEN - 1);
}

int decrypt_file(struct State *state, char *str, size_t length)
{
  *(str + length) = '\0';
  return decrypt_varlen(state, str, length - 1);
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
  struct FATable fat = {0};
  struct State state = {0};
  struct Buffer *mem_buffer = mem_buffer_init("file0000.glb");
  union Bytes bytes = {0};

  if(strncmp(RAW_HEADER, mem_buffer->buffer, RAW_HEADER_LEN) ) {
    printf("Not a GLB file!\n");
    return 1;
  }

  reset_state(&state);

  mem_buffer_read(mem_buffer, &fat.flags, READ8_MAX);
  mem_buffer_read(mem_buffer, &fat.offset, READ8_MAX);
  mem_buffer_read(mem_buffer, &fat.length, READ8_MAX);
  mem_buffer_read(mem_buffer, &fat.filename, MAX_FILENAME_LEN);

  decrypt_uint32(&state, &fat.flags);
  decrypt_uint32(&state, &fat.offset);
  decrypt_uint32(&state, &fat.length);
  decrypt_filename(&state, fat.filename);

  printf("%d %d %s\n", fat.flags, fat.length, fat.filename);
  printf("Found %d files\n", fat.offset);

  mem_buffer_free(&mem_buffer);

  return 0;
}
