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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/glb.h"

int strcompar(const void *s1, const void *s2)
{
  return strcmp(*(char **)s1, *(char **)s2);
}

void die(const char *str, const char *file, unsigned int linenum)
{
  error_at_line(errno, errno, file, linenum, "%s", str);
}

void term(const char *str)
{
  fprintf(stderr, "error: %s\n", str);
  exit(EXIT_FAILURE);
}

void warn(const char *str, const char *filename)
{
  fprintf(stderr, "warn: %s %s\n", str, filename);

  return;
}

void print_usage(const char *name, const char *text)
{
  printf("%s %s\n", name, text);

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

void tokens_truncate(struct Tokens *tokens)
{
  size_t len;

  uint32_t i;

  for(i = 0; i < tokens->ntokens; i++) {
    tokens->table[i] = basename(tokens->table[i]);
    len = strlen(tokens->table[i]);
    if(len > MAX_FILENAME_LEN) tokens->table[i] += len - MAX_FILENAME_LEN + 1;
  }

  return;
}

int add_args(char **argv, char **files, uint32_t nfiles)
{
  uint32_t i;

  if(nfiles > MAX_FILES) return nfiles;

  for(i = 0; i < MAX_FILES && i < nfiles && nfiles; i++) {
    if(access(argv[i], F_OK) ) {
      warn(WARN_NENT, argv[i]);
      nfiles--;
    } else {
      files[i] = argv[i];
    }
  }

  return nfiles;
}

int add_tokens(struct Tokens *tokens, char **files, uint32_t nfiles)
{
  uint32_t i;
  uint32_t smallest;
  uint32_t nnew;

  int fok;
  int diff;

  if(nfiles > MAX_FILES) return nfiles;

  qsort(tokens->table, tokens->ntokens, sizeof(*tokens->table), strcompar);
  qsort(files, nfiles, sizeof(*files), strcompar);

  nnew = 0;
  smallest = tokens->ntokens > nfiles ? nfiles : tokens->ntokens;

  for(i = 0; i < smallest; i++) {
    diff = strcmp(files[i], tokens->table[i]);
    fok = access(tokens->table[i], F_OK);
    if(diff && !fok) {
      files[nfiles + i] = tokens->table[i];
      nnew++;
    } else if(fok) {
      warn(WARN_NENT, tokens->table[i]);
    }
  }

  nfiles += nnew;
  if(nfiles > MAX_FILES) {
    return nfiles;
  } else if(tokens->ntokens > smallest &&
            tokens->ntokens + nfiles - nnew > MAX_FILES)
  {
    return tokens->ntokens + nfiles - nnew;
  }

  for(i = nnew; tokens->ntokens > smallest && i < tokens->ntokens; i++) {
    fok = access(tokens->table[i], F_OK);
    if(!fok) {
      files[nfiles + i] = tokens->table[i];
      nnew++;
    } else if(fok) {
      warn(WARN_NENT, tokens->table[i]);
    }
  }

  nfiles += nnew;

  return nfiles;
}
