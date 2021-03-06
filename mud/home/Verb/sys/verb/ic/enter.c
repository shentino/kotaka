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
#include <kotaka/paths/text.h>
#include <kotaka/paths/verb.h>
#include <type.h>

inherit LIB_EMIT;
inherit LIB_VERB;
inherit "~/lib/ic";

string *query_parse_methods()
{
	return ({ "english" });
}

/* ({ role, prepositions, raw }) */
mixed **query_roles()
{
	return ({ });
}

string query_help_title()
{
	return "Enter";
}

string *query_help_contents()
{
	return ({ "Enters a building or other object.",
		"Usually an enterable object will have a default exit that will be" +
		" used as the entrance, so this is just shorthand for going through" +
		" that exit."
	});
}

void main(object actor, mapping roles)
{
	object user;
	mixed dob;
	object def;
	string look;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	/* todo:  if object has a default entrance */
	/* then use it as an exit */

	dob = roles["dob"];

	if (!dob) {
		send_out("Enter what?\n");
		return;
	}

	if (dob[0]) {
		send_out("Your grammar stinks.\n");
		return;
	}
	dob = dob[1];

	switch(typeof(dob)) {
	case T_STRING:
		send_out(dob + "\n");
		return;

	case T_OBJECT:
		def = dob->query_property("default_entrance");

		if (def) {
			"~Action/sys/action/exit"->action(
				([
					"actor": actor,
					"dob": def
				])
			);
		} else {
			user = query_user();

			if (user->query_class() < 2) {
				send_out("Strange, it doesn't appear to have a main entrance.\n");
				return;
			}

			if (dob == actor || actor->is_container_of(dob)) {
				send_out("Are you trying to create a singularity?\n");
				return;
			}

			"~Action/sys/action/teleport"->action(
				([
					"actor": actor,
					"dob": dob
				])
			);
		}
		break;

	case T_ARRAY:
		send_out("You have to be more specific, there is more than one.\n");
		return;
	}
}
