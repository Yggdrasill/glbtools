#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
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

void die(const char *str)
{
  error(errno ? errno : EXIT_FAILURE, errno, "%s", str ? str : "");

  return;
}

void warn(const char *str, const char *filename)
{
  fprintf(stderr, "warn: %s %s\n", str, filename);

  return;
}

void print_usage(char *name)
{
  printf("%s %s\n", name, HELP_TEXT);

  return;
}

void args_tokenize(char *arg, struct Tokens *tokens)
{
  char *last;
  int i;

  i = 0;
  while(i < MAX_FILES && (tokens->table[i] = strtok_r(arg, ",", &last) ) ) {
    tokens->ntokens++;
    arg = NULL;
    i++;
  }

  return;
}

int args_parse(int argc, char **argv, struct Tokens *tokens)
{
  int arg;
  int mask;

  mask = 0;

  while( (arg = getopt(argc, argv, "hlxe:") ) != -1) {
    switch(arg) {
      case 'l':
        mask |= ARGS_LIST;
        break;
      case 'x':
        mask |= ARGS_EXTA;
        break;
      case 'e':
        mask |= ARGS_EXTS;
        args_tokenize(optarg, tokens);
        break;
      case 'h':
      default:
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if(arg == -1 && argc == 2) mask = ARGS_LIST;

  return mask;
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

  reset_state(state);

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

struct FATable *file_list_init(uint32_t nfiles)
{
  struct FATable *ffat;

  ffat = malloc(sizeof(*ffat) * nfiles);

  return ffat;
}

void file_list_free(struct FATable **ffat)
{
  if(ffat) free(*ffat);
  *ffat = NULL;

  return;
}

void file_list_print(struct FATable *ffat, uint32_t nfiles)
{
  for(int i = 0; i < nfiles; i++) {
    if(ffat[i].filename[0] == '\0') {
      printf("%*d", -MAX_FILENAME_LEN, i);
    } else {
      printf("%*s", -MAX_FILENAME_LEN, ffat[i].filename);
    }

    printf("%c %d bytes\n", ffat[i].flags ? 'E' : 'N', ffat[i].length);
  }

  return;
}

int main(int argc, char **argv)
{
  struct FATable hfat = {0};
  struct State state = {0};

  struct FATable *ffat = {0};
  struct Buffer *mem_buffer = {0};

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

  printf("Found %d files\n", hfat.offset);

  ffat = file_list_init(hfat.offset);

  if(!ffat) {
    perror(NULL);
  }

  for(int i = 0; i < hfat.offset; i++) {
    mem_buffer_read_fat(mem_buffer, ffat + i);
    decrypt_fat(&state, ffat + i);
  }

  file_list_print(ffat, hfat.offset);

  mem_buffer_free(&mem_buffer);
  file_list_free(&ffat);

  return 0;
}
