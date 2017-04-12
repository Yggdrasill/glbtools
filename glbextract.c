#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
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
  char *current_byte;
  char *prev_byte;
  char *byte_data;
  uint8_t *key_pos;

  current_byte = &state->current_byte;
  prev_byte = &state->prev_byte;
  key_pos = &state->key_pos;
  byte_data = (char *)data;

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

int decrypt_fat(struct State *state, struct FATable *fat)
{
  int retval;

  retval = decrypt_uint32(state, &fat->flags);
  retval += decrypt_uint32(state, &fat->offset);
  retval += decrypt_uint32(state, &fat->length);
  retval += decrypt_filename(state, fat->filename);

  return retval;
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

  if(!mem_buffer) {
    errsv = errno;
    return NULL;
  }

  glb = fopen(path, "rb");
  mem_buffer->glb = glb;

  if(!glb) {
    errsv = errno;
    free(mem_buffer);

    return NULL;
  }

  if(stat(path, &st) ) {
    errsv = errno;
    free(mem_buffer);
    fclose(glb);

    return NULL;
  }

  mem_buffer->length =  st.st_size;
  mem_buffer->position = 0;
  mem_buffer->buffer = malloc(st.st_size);

  if(!mem_buffer->buffer) {
    errsv = errno;
    free(mem_buffer);
    fclose(glb);

    return NULL;
  }

  fread(mem_buffer->buffer, 1, mem_buffer->length, glb);

  return mem_buffer;
}

void mem_buffer_free(struct Buffer **mem_buffer)
{
  if(*mem_buffer) {
    fclose( (*mem_buffer)->glb);
    free( (*mem_buffer)->buffer);
    free(*mem_buffer);
    *mem_buffer = NULL;
  }

  return;
}

size_t mem_buffer_read_varlen(struct Buffer *mem_buffer,
                              void *dest, size_t nmemb)
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

size_t mem_buffer_read_fat(struct Buffer *mem_buffer, struct FATable *fat)
{
  size_t retval;

  retval = mem_buffer_read_varlen(mem_buffer, &fat->flags, READ8_MAX);
  retval += mem_buffer_read_varlen(mem_buffer, &fat->offset, READ8_MAX);
  retval += mem_buffer_read_varlen(mem_buffer, &fat->length, READ8_MAX);
  retval += mem_buffer_read_varlen(mem_buffer, fat->filename, MAX_FILENAME_LEN);

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

int main(int argc, char **argv)
{
  struct FATable hfat = {0};
  struct State state = {0};
  struct Buffer *mem_buffer;

  if(argc < 2) {
    puts("No file to decrypt!");
    return 3;
  }

  mem_buffer = mem_buffer_init(argv[1]);

  if(!mem_buffer) {
    printf("%s: %s\n", argv[0], strerror(errsv) );
    return 2;
  }

  if(strncmp(RAW_HEADER, mem_buffer->buffer, RAW_HEADER_LEN) ) {
    puts("Not a GLB file!");
    return 1;
  }

  reset_state(&state);

  mem_buffer_read_fat(mem_buffer, &hfat);
  decrypt_fat(&state, &hfat);

  printf("%d %d %s\n", hfat.flags, hfat.length, hfat.filename);
  printf("Found %d files\n", hfat.offset);

  mem_buffer_free(&mem_buffer);

  return 0;
}
