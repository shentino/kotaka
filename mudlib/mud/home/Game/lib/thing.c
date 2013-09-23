/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2013  Raymond Jennings
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
#include <kotaka/paths/thing.h>

inherit thing LIB_THING;
private inherit oldposition "thing/position";
private inherit oldgeometry "thing/geometry";

inherit position "~Geometry/lib/thing/position";
inherit geometry "~Geometry/lib/thing/geometry";

int migrated;

static void create()
{
	migrated = 1;

	thing::create();

	geometry::create();
}

nomask void thing_migrate()
{
	if (migrated) {
		return;
	}

	position::load_position_state(oldposition::save_position_state());
	geometry::load_geometry_state(oldgeometry::save_geometry_state());

	migrated = 1;
}

static void move_notify(object old_env)
{
	thing::move_notify(old_env);
	position::move_notify(old_env);
	geometry::move_notify(old_env);
}
