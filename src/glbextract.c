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

void die(const char *str)
{
  error(errno ? errno : EXIT_FAILURE, errno, "%s", str ? str : "");

  return;
}

void warn(const char *str, const char *filename)
{
  fprintf(stderr, "warn: %s %s\n", str, filename);

  return;
}

void print_usage(const char *name)
{
  printf("%s %s\n", name, HELP_TEXT);

  return;
}

void args_tokenize(char *arg, struct Tokens *tokens)
{
  char *last;
  int i;

  i = 0;
  while(i < MAX_FILES && (tokens->table[i] = strtok_r(arg, ",", &last) ) ) {
    tokens->ntokens++;
    arg = NULL;
    i++;
  }

  return;
}

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
        print_usage(argv[0]);
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

  ssize_t bytes;

  int arg_mask;
  int rd, wd;

  char *buffer;
  buffer = NULL;

  if(argc < 2) {
    print_usage(argv[0]);
    die("Too few arguments");
  }

  arg_mask = args_parse(argc, argv, &tokens);

  if(!argv[optind]) {
    die("No input file");
  }

  glb = fopen(argv[optind], "rb");

  if(!glb) {
    die(argv[optind]);
  }

  rd = fileno(glb);
  buffer = malloc(FAT_SIZE);

  if(!buffer) {
    die(argv[0]);
  }

  bytes = pread(rd, buffer, FAT_SIZE, 0);

  if(bytes == -1) {
    die(argv[optind]);
  } else if(bytes != FAT_SIZE) {
    die("Couldn't read header FAT. Giving up.");
  } else if(strncmp(RAW_HEADER, buffer, READ8_MAX) ) {
    die("Not a GLB file!");
  }

  reset_state(&state);
  buffer_copy_fat(&hfat, buffer);
  decrypt_fat_single(&state, &hfat);

  ffat = fat_array_init(hfat.offset);

  if(!ffat) {
    die(argv[0]);
  }

  buffer = realloc(buffer, hfat.offset * FAT_SIZE);

  if(!buffer) {
    die(argv[0]);
  }

  bytes = pread(rd, buffer, hfat.offset * FAT_SIZE, FAT_SIZE);

  if(bytes == -1) {
    die(argv[optind]);
  } else if(bytes != hfat.offset * FAT_SIZE) {
    die("Couldn't read file allocation tables. Giving up.");
  }

  decrypt_fat_many(&state, ffat, buffer, hfat.offset);
  fat_names_fix(ffat, hfat.offset);

  if(arg_mask & ARGS_LIST) fat_array_print(ffat, hfat.offset);

  if(arg_mask & (ARGS_EXTA | ARGS_EXTS) ) {
    fat_flag_extraction(ffat, &tokens, hfat.offset, arg_mask);

    largest = fat_find_largest(ffat, hfat.offset);

    buffer = realloc(buffer, largest->length);

    if(!buffer) {
      die(argv[optind]);
    }

    for(uint32_t i = 0; i < hfat.offset &&
        (arg_mask & (ARGS_EXTA | ARGS_EXTS) ); i++) {

      if(!ffat[i].extract) continue;

      bytes = pread(rd, buffer, ffat[i].length, ffat[i].offset);
      if(bytes == -1) {
        die(argv[optind]);
      } else if (bytes != ffat[i].length) {
        warn("Bytes read not equal to file length", ffat[i].filename);
      }

      out = fopen(ffat[i].filename, "wb");
      if(!out) die(ffat[i].filename);
      wd = fileno(out);

      if(ffat[i].flags) decrypt_file(&state, buffer, ffat[i].length);

      bytes = write(wd, buffer, ffat[i].length);
      if(bytes == -1) {
        die(ffat[i].filename);
      } else if(bytes != ffat[i].length) {
        warn("Bytes written not equal to file length", ffat[i].filename);
      }

      fclose(out);
    }
  }

  free(buffer);
  fclose(glb);
  fat_array_free(&ffat);

  return 0;
}
