/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2012  Raymond Jennings
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
#include <kotaka/property.h>

#include <type.h>

inherit "declaration";

private mapping properties;
private string *removed_properties;

/*********************/
/* Property handling */
/*********************/

mixed query_property(string name);

/* low */

static void initialize_property()
{
	ACCESS_CHECK(previous_program() == LIB_OBJECT);

	if (!properties) {
		properties = ([ ]);
		removed_properties = ({ });
	}
}

nomask mapping _F_query_local_properties()
{
	ACCESS_CHECK(KOTAKA());

	return SUBD->deep_copy(properties);
}

nomask void _F_set_local_properties(mapping prop)
{
	ACCESS_CHECK(KOTAKA());

	properties = SUBD->deep_copy(prop);
}

nomask void _F_clear_local_properties()
{
	mapping old_props;

	ACCESS_CHECK(KOTAKA());

	properties = ([ ]);
}

nomask void _F_set_removed_properties(string *remove)
{
	ACCESS_CHECK(KOTAKA());

	removed_properties = remove - ({ nil });
}

nomask string *_F_query_removed_properties()
{
	ACCESS_CHECK(KOTAKA());

	return removed_properties[..];
}

nomask void _F_clear_removed_properties()
{
	ACCESS_CHECK(KOTAKA());

	removed_properties = ({ });
}

nomask void _F_set_local_property(string name, mixed value, varargs int nosignal)
{
	properties[name] = SUBD->deep_copy(value);
}

nomask void _F_set_property(string name, mixed value)
{
	mixed *info;
	mixed old;
	string basename;

	ACCESS_CHECK(KOTAKA());

	info = PROPERTYD->query_property(name);

	if (!info) {
		if (value != nil) {
			error("Undefined property \"" + name + "\"");
		}
	} else {
		if (info[0] != -1 && value != nil && info[0] !=
			typeof(value)) {
			error("Type mismatch, " + typeof(value) +
				" versus " + info[0] + " on " + name);
		}
	}

	if (info) {
		switch(info[1]) {
		case PROP_SIMPLE:
			break;

		case PROP_COMBO:
		case PROP_MIXDOWN:
			error("Composite properties are read only");
		case PROP_MAGIC:
			{
				string func;
				string lib;
				string creator;

				func = info[2][1];	/* writer */
				lib = function_object(func, this_object());

				if (!lib) {
					error("No such function");
				}

				creator = DRIVER->creator(lib);

				ACCESS_CHECK(creator == "Game");

				call_other(this_object(), func, value);
			}
			return;
		}
	}

	_F_set_local_property(name, value);
}

nomask mixed _F_query_property(string name)
{
	mixed *info;
	object propkey;
	string basename;

	int flags;

	ACCESS_CHECK(KOTAKA());

	info = PROPERTYD->query_property(name);

	if (!info) {
		error(name + ": Undefined property");
	}

	switch (info[1]) {
	case PROP_SIMPLE:
		{
			mixed value;

			value = properties[name];

			if (value != nil) {
				return SUBD->deep_copy(value);
			}

			switch(info[0]) {
			case T_INT:
				return 0;

			case T_FLOAT:
				return 0.0;

			case T_ARRAY:
				return ({ });

			case T_MAPPING:
				return ([ ]);

			case T_STRING:
				return "";

			default:
				return nil;
			}
		}

	case PROP_INHERIT:
	case PROP_DROPDOWN:
		if (properties[name] != nil)  {
			return SUBD->deep_copy(properties[name]);
		}
		if (sizeof(removed_properties & ({ name }) )) {
			return nil;
		}
		{
			int index;
			mixed value;
			object *arch;

			if (info[1] == PROP_DROPDOWN) {
				arch = ({ _F_query_environment() }) - ({ nil });
			} else {
				arch = _F_query_archetypes();
			}

			for (index = 0; index < sizeof(arch); index++) {
				value = arch[index]->
					query_property(name);

				if (value != nil) {
					return value;
				}
			}

			switch(info[0]) {
			case T_INT:
				return 0;

			case T_FLOAT:
				return 0.0;

			default:
				return nil;
			}
		}

	case PROP_COMBO:
	case PROP_MIXDOWN:
		{
			int index;
			mixed local;
			mixed gather;

			mixed remove;

			string lname;
			string rname;

			string *extra;
			object *arch;

			extra = info[2];

			lname = extra[0];
			rname = extra[1];

			if (info[1] == PROP_MIXDOWN) {
				arch = ({ _F_query_environment() }) - ({ nil });
			} else {
				arch = _F_query_archetypes();
			}

			local = SUBD->deep_copy(properties[lname]);

			switch(info[0]) {
			case T_ARRAY:
				gather = ({ });

				if (!local) {
					local = ({ });
				}

				break;

			case T_MAPPING:
				gather = ([ ]);

				if (!local) {
					local = ([ ]);
				}

				break;

			default:
				error("Illegal combo type");
			}

			for (index = 0; index < sizeof(arch);
				index++) {
				gather |=
					arch[index]->
					query_property(name);
			}

			if (rname) {
				remove = query_property(rname);

				if (!remove) {
					remove = ({ });
				}
				gather -= remove;
			}

			if (typeof(gather) == T_MAPPING) {
				gather += local;
			} else {
				gather |= local;
			}

			return gather;
		}
		break;

	case PROP_MAGIC:
		{
			string func;
			string lib;
			string creator;

			func = info[2][0];	/* reader */
			lib = function_object(func, this_object());

			if (!lib) {
				error("No such function");
			}

			creator = DRIVER->creator(lib);

			ACCESS_CHECK(creator == "Game");

			return call_other(this_object(), func);
		}
	default:
		error("Invalid property code of " + info[1]);
	}
}

nomask void _F_setprop_pre(string propname, mixed old, mixed new)
{
/*	_F_emit_signal("setprop:" + propname, old, new);*/
}

nomask void _F_setprop_post(string propname, mixed old, mixed new)
{
/*	_F_emit_signal("setprop-post:" + propname, old, new);*/
}

/* high */

void set_local_property(string name, mixed value)
{
	_F_set_local_property(name, value);
}

void set_property(string name, mixed value)
{
	_F_set_property(name, value);
}

mixed query_property(string name)
{
	return _F_query_property(name);
}

mixed query_local_property(string name)
{
	return SUBD->deep_copy(properties[name]);
}

string *list_local_properties()
{
	return map_indices(properties);
}

mapping query_local_properties()
{
	return SUBD->deep_copy(properties);
}

void set_local_properties(mapping prop)
{
	CHECKARG(prop, 1, "set_local_properties");

	_F_set_local_properties(prop);
}

void clear_local_properties()
{
	_F_clear_local_properties();
}

void set_removed_properties(string *remove)
{
	CHECKARG(remove, 1, "set_removed_properties");

	_F_set_removed_properties(remove);
}

string *query_removed_properties()
{
	return removed_properties[..];
}

void clear_removed_properties()
{
	_F_clear_removed_properties();
}
