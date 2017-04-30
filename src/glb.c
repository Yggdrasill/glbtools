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

#include <errno.h>
#include <error.h>
#include <unistd.h>
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

  int i;

  for(i = 0; i < tokens->ntokens; i++) {
    len = strlen(tokens->table[i]);
    if(len > MAX_FILENAME_LEN) tokens->table[i] += len - MAX_FILENAME_LEN + 1;
  }

  return;
}

int verify_files(char **argv, char **files, int nfiles)
{
  int i;

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

int fix_missing(struct Tokens *tokens, char **files, int nfiles)
{
  int i;
  int retval;

  retval = nfiles;

  for(i = 0; i < MAX_FILES && i < tokens->ntokens; i++) {
    if(access(tokens->table[i], F_OK) ) {
      warn(WARN_NENT, tokens->table[i]);
    } else {
      files[nfiles + i] = tokens->table[i];
      retval++;
    }
  }

  return retval;
}

int dedup_files(char **files, int nfiles)
{
  int i;
  int j;

  int diff;

  qsort(files, nfiles, sizeof(*files), strcompar);

  i = 0;
  j = i + 1;

  while(j < nfiles) {
    diff = strcmp(files[i], files[j]);
    if(j < MAX_FILES && !diff) {
      files[i] = files[j];
      files[j] = files[j + 1];
      nfiles--;
    }

    i++;
    j = i + 1;
  }

  return nfiles;
}
