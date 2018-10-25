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
#include <kernel/kernel.h>
#include <kotaka/privilege.h>
#include <status.h>

private int enough_free_objects(int clone)
{
	int used;
	int free;
	int total;
	int quota;

	used = status(ST_NOBJECTS);
	total = status(ST_OTABSIZE);
	free = total - used;
	quota = total / 50;

	if (quota < 100) {
		quota = 100;
	}

	if (clone) {
		quota /= 2;
	}

	return free >= quota;
}

static object compile_object(string path, string source...)
{
	object obj;

	obj = find_object(path);

	if (!SYSTEM() &&
		DRIVER->creator(path) != "System" &&
		!obj && !enough_free_objects(0)) {
		error("Too many objects");
	}

	return ::compile_object(path, source...);
}

static object load_object(string path)
{
	object obj;

	obj = find_object(path);

	if (obj) {
		return obj;
	}

	if (!SYSTEM() &&
		DRIVER->creator(path) != "System" &&
		!obj && !enough_free_objects(0)) {
		error("Too many objects");
	}

	return ::compile_object(path);
}

static object clone_object(string path, varargs string uid)
{
	if (!SYSTEM() &&
		query_owner() != "System" &&
		!enough_free_objects(1)) {
		error("Too many objects");
	}

	return ::clone_object(path, uid);
}
