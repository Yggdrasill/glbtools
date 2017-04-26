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

#ifndef GLBEXTRACT_H
#define GLBEXTRACT_H

#define HELP_TEXT "[ARGS...] [FILE]\n" \
                  "-h\tprint this help text\n" \
                  "-l\tprint list of files in archive\n" \
                  "-x\textract all files in archive\n" \
                  "-e\textract specific comma-separated files in archive"

struct Tokens {
  char *table[MAX_FILES];
  size_t ntokens;
};

int strcompar(const void *, const void *);
void die(const char *);
void warn(const char *, const char *);
void print_usage(const char *);
void args_tokenize(char *, struct Tokens *);
int args_parse(int, char **, struct Tokens *);

#endif
