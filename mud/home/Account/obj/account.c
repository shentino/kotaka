/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2018, 2019, 2020  Raymond Jennings
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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <kotaka/assert.h>

inherit "/lib/string/sprint";

string name;
string password; /* hashed */

mapping properties;

static void create(int clone)
{
}

void save()
{
	string buf;

	ACCESS_CHECK(ACCOUNT());

	buf = hybrid_sprint(
		([
			"properties" : properties,
			"password" : password
		])
	);

	SECRETD->make_dir(".");
	SECRETD->make_dir("accounts");

	SECRETD->remove_file("accounts/" + name + ".tmp");
	SECRETD->write_file("accounts/" + name + ".tmp", buf + "\n");

	if (SECRETD->file_info("accounts/" + name)) {
		if (SECRETD->file_info("accounts/" + name + ".old")) {
			SECRETD->remove_file("accounts/" + name + ".old");
		}

		SECRETD->rename_file("accounts/" + name, "accounts/" + name + ".old");
	}

	SECRETD->rename_file("accounts/" + name + ".tmp", "accounts/" + name);
}

void restore()
{
	mapping map;
	string buf;

	ACCESS_CHECK(ACCOUNT());

	buf = SECRETD->read_file("accounts/" + name);
	ASSERT(buf);

	map = PARSER_VALUE->parse(buf);

	properties = map["properties"];
	password = map["password"];
}

void load()
{
	ACCESS_CHECK(ACCOUNT());

	restore();
}

void set_name(string new_name)
{
	ACCESS_CHECK(ACCOUNT());

	ASSERT(!name);

	name = new_name;
}

string query_name()
{
	ACCESS_CHECK(ACCOUNT());

	return name;
}

void set_password(string new_password)
{
	ACCESS_CHECK(ACCOUNT());

	password = hash_string("crypt", new_password, name);

	save();
}

void set_hashed_password(string new_password)
{
	ACCESS_CHECK(ACCOUNT());

	password = new_password;

	save();
}

void clear_password()
{
	ACCESS_CHECK(ACCOUNT());

	password = nil;

	save();
}

int authenticate(string trial_password)
{
	ACCESS_CHECK(ACCOUNT());

	if (!password) {
		return !trial_password;
	}

	if (password == hash_string("crypt", trial_password, password)) {
		return TRUE;
	}

	if (password == hash_string("SHA1", trial_password)) {
		set_password(trial_password);
		return TRUE;
	}

	if (password == hash_string("MD5", trial_password)) {
		set_password(trial_password);
		return TRUE;
	}

	if (password == hash_string("crypt", trial_password, name)) {
		set_password(trial_password);
		return TRUE;
	}
}

string *list_properties()
{
	ACCESS_CHECK(ACCOUNT());

	return properties ? map_indices(properties) : ({ });
}

void set_property(string property, mixed value)
{
	ACCESS_CHECK(ACCOUNT());

	if (!property) {
		error("Invalid property name");
	}

	if (!properties && value != nil) {
		properties = ([ ]);
	}

	properties[property] = value;

	if (!map_sizeof(properties)) {
		properties = nil;
	}

	save();
}

mixed query_property(string property)
{
	ACCESS_CHECK(ACCOUNT());

	if (!property) {
		error("Invalid property name");
	}

	if (properties) {
		return properties[property];
	} else {
		return nil;
	}
}
