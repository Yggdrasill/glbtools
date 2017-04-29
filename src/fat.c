/*
 * Copyright (C) 2016-2017 Yggdrasill (kaymeerah@lambda.is)
 *
 * This file is part of glbextract.
 *
 * glbextract is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glbextract is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glbextract.  If not, see <http://www.gnu.org/licenses/>.
 */

#define POSIXLY_CORRECT
#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <search.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "../include/glb.h"
#include "../include/fat.h"

int strcompar(const void *s1, const void *s2)
{
  return strcmp(*(char **)s1, *(char **)s2);
}

char *buffer_copy_fat(struct FATable *fat, char *buffer)
{
  memcpy(&fat->flags, buffer, INT32_SIZE);
  buffer += INT32_SIZE;
  memcpy(&fat->offset, buffer, INT32_SIZE);
  buffer += INT32_SIZE;
  memcpy(&fat->length, buffer, INT32_SIZE);
  buffer += INT32_SIZE;
  memcpy(fat->filename, buffer, MAX_FILENAME_LEN);
  buffer += MAX_FILENAME_LEN;

  return buffer;
}

struct FATable *fat_array_init(uint32_t nfiles)
{
  struct FATable *ffat;

  ffat = malloc(sizeof(*ffat) * nfiles);

  return ffat;
}

void fat_array_free(struct FATable **ffat)
{
  if(ffat) {
    free(*ffat);
    *ffat = NULL;
  }

  return;
}

int fat_entry_init(struct FATable *fat, char *path, uint32_t offset)
{
  struct stat st;

  size_t len;

  if(stat(path, &st) ) {
    return -1;
  }

  fat->flags = 0;
  fat->offset = offset;
  fat->length = st.st_size;

  len = strlen(path);
  if(len > MAX_FILENAME_LEN) path += len - MAX_FILENAME_LEN + 1;

  strncpy(fat->filename, path, MAX_FILENAME_LEN);

  return 0;
}

void fat_names_fix(struct FATable *ffat, uint32_t nfiles)
{
  struct FATable *end;

  int i;

  end = ffat + nfiles;

  for(i = 0; ffat < end; ffat++, i++) {
    if(!ffat->filename[0]) {
      snprintf(ffat->filename, MAX_FILENAME_LEN, "%d", i);
    }
  }

  return;
}

void fat_flag_extraction(struct FATable *ffat,
                        struct Tokens *tokens,
                        uint32_t nfiles,
                        int arg_mask)
{
  struct FATable *end;

  char **result;
  char *str;

  end = ffat + nfiles;

  for( ; ffat < end; ffat++) {
    str = ffat->filename;
    result = lfind(&str, tokens->table, &tokens->ntokens,
                  sizeof(*tokens->table), strcompar);

    if(result || (arg_mask & ARGS_EXTA) ) {
      ffat->extract = 1;
    } else {
      ffat->extract = 0;
    }
  }

  return;
}

void fat_flag_encryption(struct FATable *ffat,
                        struct Tokens *tokens,
                        uint32_t nfiles,
                        int arg_mask)
{
  struct FATable *end;

  char **result;
  char *str;

  end = ffat + nfiles;

  for( ; ffat < end; ffat++) {
    str = ffat->filename;
    result = lfind(&str, tokens->table, &tokens->ntokens,
                  sizeof(*tokens->table), strcompar);

    if(result || (arg_mask & ARGS_ENCA) ) {
      ffat->flags = 1;
    } else {
      ffat->flags = 0;
    }
  }

  return;
}

void fat_array_print(struct FATable *ffat, uint32_t nfiles)
{
  struct FATable *end;

  end = ffat + nfiles;

  for( ; ffat < end; ffat++) {
    printf("%*s %c %u bytes\n", -MAX_FILENAME_LEN, ffat->filename,
          ffat->flags ? 'E' : 'N', ffat->length);
  }

  return;
}

struct FATable *fat_find_largest(struct FATable *ffat, uint32_t nfiles)
{
  struct FATable *end;
  struct FATable *largest;

  end = ffat + nfiles;
  largest = ffat;

  for( ; ffat < end; ffat++) {
    if(ffat->length > largest->length) largest = ffat;
  }

  return largest;
}

