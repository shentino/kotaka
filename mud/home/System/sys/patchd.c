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
#include <kernel/access.h>
#include <kotaka/assert.h>
#include <kotaka/log.h>
#include <kotaka/paths/system.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/privilege.h>
#include <status.h>

inherit SECOND_AUTO;
inherit "~/lib/struct/maparr";
inherit "~/lib/struct/multimap";

mapping patcherdb;	/* ([ level : ([ index : patchers ]) ]) */
mapping patchabledb;	/* ([ level : ([ index : obj ]) ]) */

mixed *nudgelist;

static void create()
{
	patcherdb = ([ ]);
	patchabledb = ([ ]);
}

/* hooks */

void upgrade()
{
	ACCESS_CHECK(SYSTEM());

	if (!patcherdb) {
		patcherdb = ([ ]);
	}

	if (!patchabledb) {
		patchabledb = ([ ]);
	}
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

void cleanup_patch()
{
	ACCESS_CHECK(SYSTEM());
}

private void enqueue_nudge(object obj)
{
	if (!nudgelist) {
		nudgelist = ({ nil, nil });
		call_out("dequeue_nudge", 0);
	}

	list_push_back(nudgelist, obj);
}

static void dequeue_nudge()
{
	object obj;

	obj = list_front(nudgelist);
	list_pop_front(nudgelist);

	if (list_empty(nudgelist)) {
		nudgelist = nil;
	} else {
		call_out("dequeue_nudge", 0);
	}

	if (obj) {
		obj->_F_dummy();
	}
}

atomic void enqueue_patchers(object master, string *patchers)
{
	int index;
	string path;
	int touchcount;

	ACCESS_CHECK(previous_program() == OBJECTD);

	touchcount = 0;
	path = object_name(master);
	index = status(master, O_INDEX);

	set_multimap(patcherdb, index, patchers);
	set_multimap(patchabledb, index, master);

	call_touch(master);
	enqueue_nudge(master);

	if (sscanf(path, "%*s" + CLONABLE_SUBDIR + "%*s")) {
		rlimits(0; -1) {
			int sz;

			for (sz = status(ST_OTABSIZE); --sz >= 0; ) {
				object obj;

				if (obj = find_object(path + "#" + sz)) {
					set_multimap(patchabledb, sz, obj);
					call_touch(obj);
					touchcount++;
				}
			}
		}

		call_out("sweep", 0, path);
	}
}

string *query_patchers(object obj)
{
	object odbv;
	string path;
	int index;
	int mindex;
	string *patchers;

	ACCESS_CHECK(SYSTEM());

	path = object_name(obj);
	mindex = status(obj, O_INDEX);

	if (!sscanf(path, "%s#%d", path, index)) {
		index = mindex;
	}

	if (!query_multimap(patchabledb, index)) {
		if (!query_multilevel_map_arr(objdb, 3, index)) {
			return nil;
		}
	}

	patchers = query_multimap(patcherdb, mindex);

	if (patchers) {
		return patchers;
	}
}

void clear_patch(object obj)
{
	string path;
	int index;

	ACCESS_CHECK(SYSTEM());

	path = object_name(obj);

	if (!sscanf(path, "%s#%d", path, index)) {
		index = status(obj, O_INDEX);
	}

	set_multimap(patchabledb, index, nil);
}

static void nudge_object(object obj)
{
	obj->_F_dummy();
}

static void sweep(string path, varargs int index)
{
	int max;
	int end;

	max = status(ST_OTABSIZE);
	end = max;

	if (end > index + 1000) {
		end = index + 1000;
	}

	while (index < end) {
		object obj;

		if (obj = find_object(path + "#" + index++)) {
			enqueue_nudge(obj);
			break;
		}
	}

	if (index < max) {
		call_out("sweep", 0, path, index);
	}
}
