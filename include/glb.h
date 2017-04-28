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

#ifndef GLB_H
#define GLB_H

#define RAW_HEADER "\x64\x9B\xD1\x09"

#define INT32_SIZE        4
#define RAW_HEADER_LEN    (sizeof(RAW_HEADER) - 1)
#define MAX_FILES         4096
#define MAX_FILENAME_LEN  16
#define FAT_SIZE          28

struct Tokens {
  char *table[MAX_FILES];
  size_t ntokens;
};

enum ARGS {
  ARGS_LIST = 0x01,
  ARGS_EXTA = 0x02,
  ARGS_EXTS = 0x04
};

#endif
