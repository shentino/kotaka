/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012, 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <type.h>

inherit LIB_EMIT;
inherit LIB_VERB;

string *query_parse_methods()
{
	return ({ "english" });
}

/* ({ role, prepositions, raw }) */
mixed **query_roles()
{
	return ({ });
}

void main(object actor, mapping roles)
{
	object configure;
	mixed dob;

	if (query_user()->query_class() < 3) {
		send_out("You must be an administrator to use this command.\n");
		return;
	}

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];

	if (!dob) {
		send_out("Purge what?\n");
		return;
	}

	if (typeof(dob) == T_STRING) {
		send_out(dob + "\n");
		return;
	}

	if (dob[0]) {
		send_out("Your grammar stinks.\n");
		return;
	}

	dob = dob[1];

	emit_from(actor, ({ "purge", "purges" }), dob);

	{
		int sz, i;
		object *inv;

		inv = dob->query_inventory();
		sz = sizeof(inv);

		for (i = 0; i < sz; i++) {
			object obj;

			obj = inv[i];

			if (obj->query_first_instance()) {
				continue;
			}

			if (sizeof(obj->query_inventory())) {
				continue;
			}

			obj->self_destruct();
		}
	}
}