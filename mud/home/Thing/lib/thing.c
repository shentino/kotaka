/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2006, 2010, 2011, 2012, 2013, 2014, 2015  Raymond Jennings
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
inherit arch "thing/archetype";
inherit id "thing/id";
inherit inv "thing/inventory";
inherit prop "thing/property";
inherit bulk "thing/bulk";

int destructing;

/*****************/
/* General stuff */
/*****************/

static nomask void thing_create()
{
	arch::create();
	id::create();
	inv::create();
	prop::create();
	bulk::create();
}

static void create()
{
}

int forbid_insert(object obj)
{
	return destructing;
}

static nomask void thing_destruct()
{
	int sz;
	int index;
	object env;
	object *children;

	destructing = 1;

	children = query_inventory();
	env = query_environment();
	sz = sizeof(children);

	for (index = 0; index < sz; index++) {
		children[index]->move(env);
	}

	if (env) {
		env->bulk_invalidate();
	}

	clear_archetypes();
}

void self_destruct()
{
	destruct_object(this_object());
}