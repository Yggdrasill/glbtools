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

#ifndef GLBCREATE_H
#define GLBCREATE_H

struct Tokens;

const char *DEFAULT_NAME = "output.glb";

int args_parse(int, char **, char **, struct Tokens *);

#endif

