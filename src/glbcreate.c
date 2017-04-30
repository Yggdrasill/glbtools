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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>

#include "../include/glb.h"
#include "../include/fat.h"
#include "../include/crypt.h"

const char *DEFAULT_NAME = "output.glb";

int args_parse(int argc, char **argv, char **str, struct Tokens *tokens)
{
  int arg;
  int mask;

  mask = 0;

  while( (arg = getopt(argc, argv, "ho:ae:") ) != -1) {
    switch(arg) {
      case 'o':
        mask |= ARGS_NAME;
        *str = optarg;
        break;
      case 'a':
        mask |= ARGS_ENCA;
        break;
      case 'e':
        mask |= ARGS_ENCS;
        args_tokenize(optarg, tokens);
        break;
      case 'h':
      default:
        print_usage(argv[0], HELP_CREATE);
        exit(EXIT_FAILURE);
    }
  }

  return mask;
}

ssize_t io_write_fat(struct FATable *fat, int fd)
{
  char buffer[FAT_SIZE];

  ssize_t retval;

  int pos;

  pos = 0;

  memcpy(buffer, &fat->flags, INT32_SIZE);
  pos += INT32_SIZE;
  memcpy(buffer + pos, &fat->offset, INT32_SIZE);
  pos += INT32_SIZE;
  memcpy(buffer + pos, &fat->length, INT32_SIZE);
  pos += INT32_SIZE;
  memcpy(buffer + pos, fat->filename, MAX_FILENAME_LEN);
  pos += MAX_FILENAME_LEN;

  return write(fd, buffer, FAT_SIZE);
}

int main(int argc, char **argv)
{
  FILE *glb;
  FILE *input;

  struct State state = {0};
  struct Tokens tokens = {0};
  struct FATable hfat = {0};
  struct FATable temp = {0};
  struct FATable *ffat = {0};

  char *files[MAX_FILES];
  char *buffer;
  char *filename;

  size_t largest;
  size_t offset;
  size_t bytes;

  int rd, wd;
  int arg_mask;
  int num_files;

  filename = NULL;
  largest = 0;

  if(argc < 2) {
    print_usage(argv[0], HELP_CREATE);
    term(ERR_NOARGS);
  }

  arg_mask = args_parse(argc, argv, &filename, &tokens);

  num_files = add_args(&argv[optind], files, argc - optind);
  num_files = add_tokens(&tokens, files, num_files);

  tokens_truncate(&tokens);

  if(num_files <= 0) {
    term(ERR_NOFILE);
  } else if(num_files > MAX_FILES) {
    term(ERR_TMFILE);
  }

  if(!(arg_mask & ARGS_NAME) ) filename = (char *)DEFAULT_NAME;

  glb = fopen(filename, "wb");
  if(!glb) {
    die(filename, __FILE__, __LINE__);
  }
  wd = fileno(glb);

  hfat.offset = num_files;
  encrypt_fat_single(&state, &hfat);
  io_write_fat(&hfat, wd);

  ffat = malloc(sizeof(*ffat) * num_files);
  if(!ffat) {
    die(DIE_NOMEM, __FILE__, __LINE__);
  }

  offset = num_files * FAT_SIZE + FAT_SIZE;

  for(int i = 0; i < num_files; i++) {
    if(fat_entry_init(&ffat[i], files[i], offset) ) {
      die(files[i], __FILE__, __LINE__);
    }

    if(ffat[i].length > largest) largest = ffat[i].length;
    offset += ffat[i].length;
  }

  if(arg_mask & (ARGS_ENCA | ARGS_ENCS) ) {
    fat_flag_encryption(ffat, &tokens, num_files, arg_mask);
  }

  for(int i = 0; i < num_files; i++) {
    memcpy(&temp, &ffat[i], sizeof(ffat[i]) );
    encrypt_fat_single(&state, &temp);

    bytes = io_write_fat(&temp, wd);
    if(bytes == -1) {
      die(filename, __FILE__, __LINE__);
    } else if(bytes != FAT_SIZE) {
      warn(WARN_WNEL, ffat[i].filename);
    }
  }

  buffer = malloc(largest);
  if(!buffer) {
    die(DIE_NOMEM, __FILE__, __LINE__);
  }

  for(int i = 0; i < num_files; i++) {
    input = fopen(files[i], "rb");
    if(!input) {
      die(files[i], __FILE__, __LINE__);
    }
    rd = fileno(input);

    bytes = read(rd, buffer, ffat[i].length);
    if(bytes == -1) {
      die(files[i], __FILE__, __LINE__);
    } else if(bytes != ffat[i].length) {
      warn(WARN_RNEL, files[i]);
    }

    if(ffat[i].flags) {
      encrypt_file(&state, buffer, ffat[i].length);
    }

    bytes = write(wd, buffer, ffat[i].length);
    if(bytes == -1) {
      die(filename, __FILE__, __LINE__);
    } else if(bytes != ffat[i].length) {
      warn(WARN_WNEL, filename);
    }

    fclose(input);
  }

  free(ffat);
  free(buffer);
  fclose(glb);

  return 0;
}
