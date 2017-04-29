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

static int encrypt_varlen(struct State *, void *, size_t);
int encrypt_uint32(struct State *, uint32_t *);
int encrypt_filename(struct State *, char *);
int encrypt_fat_single(struct State *, struct FATable *);
int encrypt_fat_many(struct State *, struct FATable *, char *, uint32_t);
int encrypt_file(struct State *, char *, size_t);

#endif
