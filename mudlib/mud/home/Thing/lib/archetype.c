/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012-2013  Raymond Jennings
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
#include <kotaka/checkarg.h>
#include <kotaka/paths.h>
#include <kotaka/privilege.h>

private object *archetypes;

static void create()
{
}

void clean_archetypes()
{
	if (!archetypes) {
		return;
	}

	archetypes -= ({ nil });

	if (!sizeof(archetypes)) {
		archetypes = nil;
	}
}

int is_archetype_of(object test)
{
	int index;
	int sz;
	object *arch;

	arch = test->query_archetypes();
	sz = sizeof(arch);

	if (sizeof(arch & ({ this_object() }))) {
		return 1;
	}

	for(index = 0; index < sz; index++) {
		if (is_archetype_of(arch[index])) {
			return 1;
		}
	}

	return 0;
}

object *query_archetypes()
{
	clean_archetypes();

	if (archetypes) {
		return archetypes[..];
	} else {
		return ({ });
	}
}

void set_archetypes(object *new_archs)
{
	int i;
	int sz;

	object *old_archs;
	object *check;

	check = new_archs - ({ archetypes, nil });

	sz = sizeof(check);

	for (i = 0; i < sz; i++) {
		CHECKARG(check[i] <- "archetype", 1, "add_archetype");

		if (is_archetype_of(check[i])) {
			error("Circular reference");
		}
	}

	archetypes = new_archs;

	clean_archetypes();
}

void clear_archetypes()
{
	archetypes = nil;
}

void add_archetype(object new_arch)
{
	CHECKARG(new_arch, 1, "add_archetype");
	CHECKARG(new_arch <- "archetype", 1, "add_archetype");

	if (is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	clean_archetypes();

	if (!archetypes) {
		archetypes = ({ new_arch });
	} else {
		archetypes += ({ new_arch });
	}
}

void add_archetype_at(object new_arch, int position)
{
	CHECKARG(new_arch, 1, "add_archetype_at");
	CHECKARG(new_arch <- "archetype", 1, "add_archetype_at");

	CHECKARG(position >= -1, 2, "add_archetype_at");
	CHECKARG(position <= sizeof(archetypes), 2, "add_archetype_at");

	if (is_archetype_of(new_arch)) {
		error("Circular reference");
	}

	clean_archetypes();

	if (position == -1) {
		archetypes += ({ new_arch });
	} else {
		archetypes = archetypes[0 .. position - 1]
			+ ({ new_arch }) + archetypes[position ..];
	}
}

void del_archetype(object old_arch)
{
	archetypes -= ({ old_arch });

	clean_archetypes();
}
