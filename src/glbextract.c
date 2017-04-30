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

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>

#include "../include/glb.h"
#include "../include/fat.h"
#include "../include/crypt.h"
#include "../include/glbextract.h"

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
        print_usage(argv[0], HELP_EXTRACT);
        exit(EXIT_FAILURE);
    }
  }

  if(arg == -1 && argc == 2) mask = ARGS_LIST;

  return mask;
}

int main(int argc, char **argv)
{
  FILE *glb;
  FILE *out;

  struct FATable hfat = {0};
  struct FATable *ffat = {0};
  struct FATable *largest = {0};
  struct State state = {0};
  struct Tokens tokens = {0};

  char *buffer;

  ssize_t bytes;
  uint32_t i;
  int arg_mask;
  int rd, wd;

  if(argc < 2) {
    print_usage(argv[0], HELP_EXTRACT);
    term(ERR_NOARGS);
  }

  arg_mask = args_parse(argc, argv, &tokens);
  if(!argv[optind]) {
    term(ERR_NOFILE);
  }

  glb = fopen(argv[optind], "rb");
  if(!glb) {
    die(argv[optind], __FILE__, __LINE__);
  }
  rd = fileno(glb);

  buffer = malloc(FAT_SIZE);
  if(!buffer) {
    die(DIE_NOMEM, __FILE__, __LINE__);
  }

  bytes = pread(rd, buffer, FAT_SIZE, 0);
  if(bytes == -1) {
    die(argv[optind], __FILE__, __LINE__);
  } else if(bytes != FAT_SIZE) {
    term(ERR_CRHFAT);
  } else if(strncmp(RAW_HEADER, buffer, INT32_SIZE) ) {
    term(ERR_NOTGLB);
  }

  buffer_copy_fat(&hfat, buffer);
  decrypt_fat_single(&state, &hfat);

  if(hfat.offset > MAX_FILES) {
    term(ERR_TMFILE);
  }

  ffat = fat_array_init(hfat.offset);
  if(!ffat) {
    die(DIE_NOMEM, __FILE__, __LINE__);
  }

  buffer = realloc(buffer, hfat.offset * FAT_SIZE);
  if(!buffer) {
    die(DIE_NOMEM, __FILE__, __LINE__);
  }

  bytes = pread(rd, buffer, hfat.offset * FAT_SIZE, FAT_SIZE);
  if(bytes == -1) {
    die(argv[optind], __FILE__, __LINE__);
  } else if(bytes != hfat.offset * FAT_SIZE) {
    term(ERR_CRFFAT);
  }

  decrypt_fat_many(&state, ffat, buffer, hfat.offset);
  fat_names_fix(ffat, hfat.offset);

  if(arg_mask & ARGS_LIST) fat_array_print(ffat, hfat.offset);

  if(arg_mask & (ARGS_EXTA | ARGS_EXTS) ) {
    fat_flag_extraction(ffat, &tokens, hfat.offset, arg_mask);

    largest = fat_find_largest(ffat, hfat.offset);
    buffer = realloc(buffer, largest->length);
    if(!buffer) {
      die(DIE_NOMEM, __FILE__, __LINE__);
    }

    for(i = 0; i < hfat.offset; i++) {

      if(!ffat[i].extract) continue;

      bytes = pread(rd, buffer, ffat[i].length, ffat[i].offset);
      if(bytes == -1) {
        die(argv[optind], __FILE__, __LINE__);
      } else if (bytes != ffat[i].length) {
        warn(WARN_RNEL, ffat[i].filename);
      }

      out = fopen(ffat[i].filename, "wb");
      if(!out) die(ffat[i].filename, __FILE__, __LINE__);
      wd = fileno(out);

      if(ffat[i].flags) decrypt_file(&state, buffer, ffat[i].length);

      bytes = write(wd, buffer, ffat[i].length);
      if(bytes == -1) {
        die(ffat[i].filename, __FILE__, __LINE__);
      } else if(bytes != ffat[i].length) {
        warn(WARN_WNEL, ffat[i].filename);
      }

      fclose(out);
    }
  }

  free(buffer);
  fclose(glb);
  fat_array_free(&ffat);

  return 0;
}
