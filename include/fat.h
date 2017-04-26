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
