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

#define RAW_HEADER    "\x64\x9B\xD1\x09"

#define HELP_EXTRACT  "[ARGS...] [FILE]\n" \
                      "-h\tprint this help text\n" \
                      "-l\tprint list of files in archive\n" \
                      "-x\textract all files in archive\n" \
                      "-e\textract specific comma-separated files in archive"

#define ERR_NOARGS    "Too few arguments"
#define ERR_NOFILE    "No input file"
#define ERR_TMFILE    "Too many files"
#define ERR_CRHFAT    "Couldn't read header FAT"
#define ERR_NOTGLB    "Not a GLB file"
#define ERR_CRFFAT    "Couldn't read file allocation tables"

#define DIE_NOMEM     "malloc() call failed"

#define WARN_RNEL     "Bytes read not equal to file length"
#define WARN_WNEL     "Bytes written not equal to file length"
#define WARN_NENT     "File does not exist"

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
  ARGS_LIST = 1<<0,
  ARGS_EXTA = 1<<1,
  ARGS_EXTS = 1<<2,
  ARGS_NAME = 1<<3,
  ARGS_ENCA = 1<<4,
  ARGS_ENCS = 1<<5
};

#endif
