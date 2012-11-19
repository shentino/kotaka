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
#include <kotaka/paths.h>

#define CONFIGD "foo"

string grammar;

static void create()
{
	::create();

	grammar = read_file("~/data/parse/value.dpd");
}

static void upgraded()
{
	::create();

	grammar = read_file("~/data/parse/value.dpd");
}

mixed parse(string input)
{
	mixed *ret;

	ret = parse_string(grammar, input);

	if (!ret)
		error("Parse failure");

	return ret[0];
}

static string *parse_str(string *input)
{
	string str;

	str = input[0];
	str = str[1 .. strlen(str) - 2];

	str = STRINGD->replace(str, "\\t", "\t");
	str = STRINGD->replace(str, "\\n", "\n");
	str = STRINGD->replace(str, "\\\"", "\"");
	str = STRINGD->replace(str, "\\\\", "\\");

	return ({ str });
}

static int *parse_int(string *input)
{
	return ({ (int)input[0] });
}

static float *parse_flt(string *input)
{
	return ({ (float)input[0] });
}

static object *parse_obj(string *input)
{
	string oname, osubname;
	string *parts;
	object root, obj;

	oname = input[0];
	oname = oname[1 .. strlen(oname) - 2];	/* strip off angle brackets */

	if (function_object("parse_object", previous_object(1))) {
		return ({ previous_object(1)->parse_object(oname) });
	}

	parts = explode(oname, ":") - ({ "" });

	if (sizeof(parts) == 0) {
		error("Badly formatted object designation: <" + oname + ">");
	}

	if (parts[0][0] == '/') {
		string osubname;

		root = find_object(parts[0]);

		if (!root)
			error(parts[0] + ": no such object");
	} else if (parts[0] != "ROOT") {
		error("Unspecified base for object");
	} else {
		root = CONFIGD->query_object_root();
	}

	if (sizeof(parts) > 1) {
		osubname = implode(parts[1 ..], ":");
		obj = root->locate_object(osubname);
		return ({ obj });
	} else {
		return ({ root });
	}
}

static mixed *parse_nil(string *input)
{
	return ({ nil });
}

static mixed *pick_middle(mixed *input)
{
	if (sizeof(input) % 2 != 1)
		error("Illegal input");
	return input[sizeof(input) / 2 .. sizeof(input) / 2];
}

static mixed **make_arr(mixed *input)
{
	return ({ input });
}

static mixed **make_empty_arr(string *input)
{
	return ({ ({ }) });
}

static mixed *build_arr(mixed *input)
{
	return ({ input[0] + ({ input[2] }) });
}

static mapping *make_map(mixed *input)
{
	return ({ ([ input[0] : input[1] ]) });
}

static mapping *make_empty_map(string *input)
{
	return ({ ([ ]) });
}

static mapping *build_map(mixed *input)
{
	return ({ input[0] + ([ input[2] : input[3] ]) });
}

static mixed *strike_middle(mixed *input)
{
	return ({ input[0], input[sizeof(input) - 1] });
}
