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

#define RED	1
#define GREEN	2
#define BLUE	4
#define BOLD	8

inherit "/lib/string/char";
inherit "~Geometry/lib/diff";

void on_paint_text(object gc, object obj, object viewer)
{
	int dx, dy, dz;
	int lx, ly, hx, hy;
	int i;
	mixed sx, sy;

	({ dx, dy, dz }) = query_position_difference(viewer, obj);

	gc->set_layer("view");

	lx = dx;
	ly = dy;
	sx = obj->query_property("size_x");
	sy = obj->query_property("size_y");

	if (!sx) {
		sx = 0;
	}
	if (!sy) {
		sy = 0;
	}

	hx = dx + sx - 1;
	hy = dy + sy - 1;

	if (sx > (hx - lx + 1)) {
		sx = hx - lx + 1;
	}

	gc->set_color(0x3B);

	for (i = ly; i <= hy; i++) {
		gc->move_pen(lx, i);
		gc->draw(chars('.', sx));
	}
}
