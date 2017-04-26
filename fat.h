#ifndef FAT_H
#define FAT_H

struct Tokens;

struct FATable {
  uint32_t extract;
  uint32_t flags;
  uint32_t offset;
  uint32_t length;
  char filename[MAX_FILENAME_LEN];
};

char *buffer_copy_fat(struct FATable *, char *);
struct FATable *fat_array_init(uint32_t);
void fat_array_free(struct FATable **);
void fat_names_fix(struct FATable *, uint32_t);
void fat_flag_extraction(struct FATable *, struct Tokens *, uint32_t, int);
void fat_array_print(struct FATable *, uint32_t);
struct FATable *fat_find_largest(struct FATable *, uint32_t);

#endif
