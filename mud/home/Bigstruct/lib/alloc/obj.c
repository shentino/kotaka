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
mapping query_grants();

static object new_node()
{
	return clone_object("node");
}

static void discard_node(object node)
{
	destruct_object(node);
}

void garbage_check()
{
	mapping grants;

	if (!sscanf(object_name(this_object()), "%*s#%*d")) {
		return;
	}

	grants = query_grants();

	if (!grants || !map_sizeof(grants)) {
		destruct_object(this_object());
	}
}
