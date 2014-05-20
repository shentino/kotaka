/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013, 2014  Raymond Jennings
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
#include <kotaka/paths/string.h>
#include <kotaka/paths/text.h>
#include <type.h>

private string from_object(object obj, mapping seen, object viewer)
{
	if (seen[obj]) {
		if (obj == viewer) {
			return "yourself";
		} else {
			return "itself";
		}
	} else {
		if (obj == viewer) {
			return "you";
		} else {
			return TEXT_SUBD->generate_brief_definite(obj);
		}
	}
}

void emit_to(object actor, object viewer, string *verbs, mixed chain ...)
{
	string *buffer;
	mapping seen;
	string phrase;
	object *mobiles;

	int i, sz;

	buffer = ({ });

	seen = ([ ]);

	if (actor == viewer) {
		if (verbs[0]) {
			buffer += ({ "you", verbs[0] });
		}
	} else {
		if (verbs[1]) {
			buffer = ({ TEXT_SUBD->generate_brief_definite(actor), verbs[1] });
		}
	}

	seen[actor] = 1;

	sz = sizeof(chain);

	for (i = 0; i < sz; i++) {
		mixed item;

		item = chain[i];

		switch(typeof(item)) {
		case T_STRING:
			buffer += ({ item });
			break;

		case T_OBJECT:
			buffer += ({ from_object(item, seen, viewer) });
			break;

		case T_ARRAY:
			{
				int j, sz2;
				string *list;

				sz2 = sizeof(item);
				list = ({ });

				for (j = 0; j < sz2; j++) {
					list += ({ from_object(item[j], seen, viewer) });
				}

				list[sz2 - 1] = "and " + list[sz2 - 1];

				buffer += ({ implode(list, ", ") });
			}
			break;
		}
	}

	phrase = implode(buffer, " ") + ".";
	phrase = STRINGD->to_upper(phrase[0 .. 0]) + phrase[1 ..];

	mobiles = viewer->query_property("mobiles");

	sz = sizeof(mobiles -= ({ nil }) );

	for (i = 0; i < sz; i++) {
		mobiles[i]->message(phrase + "\n");
	}
}

void emit_from(object actor, string *verbs, mixed chain ...)
{
	object env;
	object *cand;
	int i, sz;

	cand = ({ actor });

	env = actor->query_environment();

	if (env) {
		cand |= env->query_inventory();
		cand |= ({ env });
	}

	cand |= actor->query_inventory();

	sz = sizeof(cand);

	for (i = 0; i < sz; i++) {
		emit_to(actor, cand[i], verbs, chain ...);
	}
}
