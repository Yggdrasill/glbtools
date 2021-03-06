/*
 * Copyright (C) 2016-2017 Yggdrasill (kaymeerah@lambda.is)
 *
 * This file is part of glbtools.
 *
 * glbtools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glbtools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glbtools.  If not, see <http://www.gnu.org/licenses/>.
 */

#define POSIXLY_CORRECT
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include "../include/glb.h"
#include "../include/fat.h"
#include "../include/crypt.h"

static int encrypt_varlen(struct State *state, void *data, size_t size)
{
  char *current_byte;
  char *prev_byte;
  char *byte_data;
  uint8_t *key_pos;

  size_t i;

  current_byte = &state->current_byte;
  prev_byte = &state->prev_byte;
  key_pos = &state->key_pos;
  byte_data = (char *)data;

  for(i = 0; i < size; i++) {
    *current_byte = byte_data[i];
    byte_data[i] = *current_byte + DEFAULT_KEY[*key_pos] + *prev_byte;
    byte_data[i] &= 0xFF;
    (*key_pos)++;
    *key_pos %= strlen(DEFAULT_KEY);
    *prev_byte = byte_data[i];
  }

  return i;
}

int encrypt_uint32(struct State *state, uint32_t *data32)
{
  return encrypt_varlen(state, data32, INT32_SIZE);
}

int encrypt_filename(struct State *state, char *str)
{
  *(str + MAX_FILENAME_LEN - 1) = '\0';
  return encrypt_varlen(state, str, MAX_FILENAME_LEN);
}

int encrypt_fat_single(struct State *state, struct FATable *fat)
{
  int retval;

  reset_state(state);

  retval = encrypt_uint32(state, &fat->flags);
  retval += encrypt_uint32(state, &fat->offset);
  retval += encrypt_uint32(state, &fat->length);
  retval += encrypt_filename(state, fat->filename);

  return retval;
}

int encrypt_fat_many(struct State *state,
                    struct FATable *ffat,
                    char *buffer,
                    uint32_t nfiles)
{
  struct FATable *end;

  char *newptr;

  int retval;

  end = ffat + nfiles;
  retval = 0;

  for( ; ffat < end; ffat++) {
    newptr = buffer_copy_fat(ffat, buffer);
    retval += encrypt_fat_single(state, ffat);
    buffer = newptr;
  }

  return retval;
}

int encrypt_file(struct State *state, char *str, size_t length)
{
  if(!length) return 0;
  reset_state(state);
  return encrypt_varlen(state, str, length - 1);
}
