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
	return ({
		({ "dob", ({ nil }), 0 }),
		({ "iob", ({ "to" }), 0 })
	});
}

private string validate_give(object actor, object obj, object iob)
{
	if (obj == actor) {
		return "Giving yourself can't be done literally.";
	}

	if (obj->query_environment() != actor) {
		return "It has to be in your possession first.";
	}

	if (obj->is_container_of(iob)) {
		return "Turn it inside out first.";
	}

	if (obj == iob) {
		return "Just turn it inside out.";
	}

	if (obj->query_property("is_immobile")) {
		return "It is stuck like glue and cannot be given away.";
	}
}

private void do_give(object actor, object obj, object iob)
{
	obj->move(iob);
	obj->set_x_position(0);
	obj->set_y_position(0);
	obj->set_z_position(0);
}

string query_help_title()
{
	return "Give";
}

string *query_help_contents()
{
	return ({
		"Gives an object to a sentient creature.",
		"The object must be in your possession and the recipient must consent."
	});
}

void main(object actor, mapping roles)
{
	mixed dob;
	mixed iob;
	string look;

	if (!actor) {
		send_out("You must be in character to use this command.\n");
		return;
	}

	dob = roles["dob"];
	iob = roles["iob"];

	if (!dob) {
		send_out("Give what?\n");
		return;
	}

	if (!iob) {
		send_out("Give it to who?\n");
		return;
	}

	if (dob[0]) {
		send_out("Bad grammar.\n");
		return;
	}

	if (iob[0] != "to") {
		send_out("Bad grammar.\n");
		return;
	}

	dob = dob[1];
	iob = iob[1];

	switch(typeof(dob)) {
	case T_STRING:
		send_out(dob + "\n");
		return;

	case T_ARRAY:
	case T_OBJECT:
		switch(typeof(iob)) {
		case T_STRING:
			send_out(iob + "\n");
			return;

		case T_ARRAY:
			send_out("Please be more specific on who or what you are giving things to.\n");
			return;

		case T_OBJECT:
			switch(typeof(dob)) {
			case T_ARRAY:
				{
					int i, sz;
					sz = sizeof(dob);

					for (i = 0; i < sz; i++) {
						string err;

						err = validate_give(actor, dob[i], iob);

						if (err) {
							send_out(err + "\n");
							return;
						}
					}

					for (i = 0; i < sz; i++) {
						do_give(actor, dob[i], iob);
					}
				}
				break;

			case T_OBJECT:
				{
					string err;

					err = validate_give(actor, dob, iob);

					if (err) {
						send_out(err + "\n");
						return;
					}

					do_give(actor, dob, iob);
				}
				break;
			}
		}
	}

	emit_from(actor, actor, " ", ({ "give", "gives" }), dob, " to ", iob, ".");
}
