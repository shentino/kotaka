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
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <status.h>
#include <type.h>

inherit SECOND_AUTO;

object progdb;	/* program database, index -> program_info */
object pathdb;	/* defunct */
object inhdb;	/* inherit database, filename -> inheriting objects */
object incdb;	/* include database, filename -> including objects */

atomic void create_database();
atomic void destruct_database();

/* create/destruct */

static void create()
{
}

static void destruct()
{
	destruct_database();
}

void upgrade()
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	if (pathdb) {
		destruct_object(pathdb);
	}
}

/* helpers */

atomic void create_database()
{
	ACCESS_CHECK(SYSTEM());
	ASSERT(find_object(USR_DIR + "/Bigstruct/initd"));
	ASSERT(find_object(BIGSTRUCT_MAP_OBJ));

	rlimits (0; -1) {
		progdb = clone_object(BIGSTRUCT_MAP_OBJ);
		progdb->claim();
		progdb->set_type(T_INT);

		inhdb = clone_object(BIGSTRUCT_MAP_OBJ);
		inhdb->claim();
		inhdb->set_type(T_INT);

		incdb = clone_object(BIGSTRUCT_MAP_OBJ);
		incdb->claim();
		incdb->set_type(T_STRING);
	}
}

atomic void destruct_database()
{
	ACCESS_CHECK(SYSTEM());

	rlimits (0; -1) {
		int i, sz;
		object *turkeys;

		turkeys = ({ });

		if (progdb) {
			turkeys += ({ progdb });
		}

		if (inhdb) {
			turkeys += ({ inhdb });
		}

		if (incdb) {
			turkeys += ({ incdb });
		}

		if (pathdb) {
			turkeys += ({ pathdb });
		}

		progdb = nil;
		inhdb = nil;
		incdb = nil;
		pathdb = nil;

		sz = sizeof(turkeys);

		for (i = 0; i < sz; i++) {
			destruct_object(turkeys[i]);
		}
	}
}

/* objectd hooks */

atomic object register_program(string path, string *inherits, string *includes)
{
	int oindex;
	object pinfo;

	int *oinherits;
	string *oincludes;

	ACCESS_CHECK(SYSTEM());

	if (!progdb) {
		LOGD->post_message("system", LOG_WARNING, "Skipping registration of " + path + ", no progdb");

		return nil;
	}

	oindex = status(path, O_INDEX);

	pinfo = progdb->query_element(oindex);

	if (!pinfo) {
		pinfo = new_object(PROGRAM_INFO);
		pinfo->set_path(path);

		progdb->set_element(oindex, pinfo);
	}

	if (inherits) {
		string *ctors;
		string *dtors;
		int sz;
		int i;

		int *oindices;

		ASSERT(includes);

		sz = sizeof(inherits);
		oindices = allocate(sz);
		ctors = ({ });
		dtors = ({ });

		for (i = 0; i < sz; i++) {
			object subpinfo;
			int suboindex;

			suboindex = status(inherits[i], O_INDEX);
			oindices[i] = suboindex;

			subpinfo = progdb->query_element(suboindex);

			if (subpinfo) {
				mixed inh;

				inh = subpinfo->query_inherited_constructors();

				if (inh) {
					ctors |= inh;
				}

				ctors |= ({ subpinfo->query_constructor() });

				inh = subpinfo->query_inherited_destructors();

				if (inh) {
					dtors |= inh;
				}

				dtors |= ({ subpinfo->query_destructor() });
			} else {
				LOGD->post_message("debug", LOG_WARNING, "No program info for inherited program " + inherits[i]);
			}
		}

		ctors -= ({ nil });
		dtors -= ({ nil });

		pinfo->set_inherits(oindices);
		pinfo->set_includes(includes);
		pinfo->set_inherited_constructors(ctors);
		pinfo->set_inherited_destructors(dtors);
	}

	return pinfo;
}

atomic object register_destructed_program(string path, int index)
{
	object pinfo;

	ACCESS_CHECK(SYSTEM());

	if (!progdb) {
		LOGD->post_message("system", LOG_WARNING, "Skipping registration of destructed " + path + ", no progdb");

		return nil;
	}

	pinfo = new_object(PROGRAM_INFO);
	pinfo->set_path(path);

	progdb->set_element(index, pinfo);
	pinfo->set_destructed(path);

	return pinfo;
}

atomic void remove_program(int index)
{
	object pinfo;
	string path;

	int *oinherits;
	string *oincludes;

	ACCESS_CHECK(SYSTEM());

	if (!progdb) {
		return;
	}

	progdb->set_element(index, nil);
}

atomic void reset_program_database()
{
	ACCESS_CHECK(SYSTEM());

	destruct_database();
	create_database();
}

/* public functions */

object query_program_indices()
{
	object indices;

	if (!progdb) {
		return nil;
	}

	indices = progdb->query_indices();

	indices->grant_access(previous_object(), FULL_ACCESS);
	indices->grant_access(this_object(), 0);

	return indices;
}

object query_program_info(int oindex)
{
	object pinfo;

	if (progdb) {
		return progdb->query_element(oindex);
	}
}
