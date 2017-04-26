#ifndef CRYPT_H
#define CRYPT_H

struct FATable;

struct State {
  char current_byte;
  char prev_byte;
  uint8_t key_pos;
};

static const char *DEFAULT_KEY = "32768GLB";

int calculate_key_pos(size_t len);
void reset_state(struct State *state);

static int decrypt_varlen(struct State *, void *, size_t);
int decrypt_uint32(struct State *, uint32_t *);
int decrypt_filename(struct State *, char *);
int decrypt_fat_single(struct State *, struct FATable *);
int decrypt_fat_many(struct State *, struct FATable *, char *, uint32_t);
int decrypt_file(struct State *, char *, size_t length);

#endif
