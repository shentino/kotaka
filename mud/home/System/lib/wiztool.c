/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018  Raymond Jennings
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <kernel/kernel.h>
#include <kernel/user.h>
#include <kernel/access.h>
#include <kotaka/paths/system.h>

inherit a SECOND_AUTO;
inherit w LIB_WIZTOOL;

static void create(int size)
{
	w::create(size);
}

/*
 * NAME:	compile_object()
 * DESCRIPTION:	compile_object wrapper
 */
static object compile_object(string path, string source...)
{
    int kernel;

    path = DRIVER->normalize_path(path, query_directory(), query_owner());
    kernel = sscanf(path, "/kernel/%*s");
    if ((sizeof(source) != 0 && kernel) ||
	!access(query_owner(), path,
		((sscanf(path, "%*s" + INHERITABLE_SUBDIR) != 0 ||
		  !DRIVER->creator(path)) && sizeof(source) == 0 && !kernel) ?
		 READ_ACCESS : WRITE_ACCESS)) {
	message(path + ": Permission denied.\n");
	return nil;
    }
    return (sizeof(source) != 0) ?
	    a::compile_object(path, source...) : a::compile_object(path);
}

/*
 * NAME:	clone_object()
 * DESCRIPTION:	clone_object wrapper
 */
static object clone_object(string path)
{
    path = DRIVER->normalize_path(path, query_directory(), query_owner());
    if (sscanf(path, "/kernel/%*s") != 0 || !access(query_owner(), path, READ_ACCESS)) {
	message(path + ": Permission denied.\n");
	return nil;
    }
    return a::clone_object(path);
}

/*
 * NAME:	dump_state()
 * DESCRIPTION:	create a state dump
 */
static void dump_state(varargs int increment)
{
    if (!access(query_owner(), "/", FULL_ACCESS)) {
	message("Permission denied.\n");
    } else {
	a::dump_state(increment);
    }
}
