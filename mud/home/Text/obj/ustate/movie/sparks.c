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
#include <kotaka/paths/text.h>
#include <kotaka/privilege.h>

inherit "~/lib/animate";
inherit "/lib/math/random";

int nparticles;
mixed **particles;
int color;

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

private void set_nparticles()
{
	nparticles = screen_width * screen_height / 100;
}

private void reset_particle(mixed *particle)
{
	particle[0] = (float)(screen_width) / 2.0;
	particle[1] = (float)(screen_height);

	particle[2] = (bell_rnd(2) - 0.5) * (float)(screen_width);
	particle[3] = bell_rnd(2) * (float)(screen_height) * 2.0 - (float)(screen_height) * 2.0;
	particle[4] = color;

	color++;
	color %= 7;
}

void begin()
{
	int i;

	ACCESS_CHECK(previous_object() == query_user());

	::begin();

	send_out("\033[2J");

	check_screen();
	set_nparticles();

	particles = allocate(nparticles);

	for (i = 0; i < nparticles; i++) {
		particles[i] = allocate(5);
		reset_particle(particles[i]);
	}
}

private void do_particles(object paint, float diff)
{
	int x, y, i;

	for (i = 0; i < nparticles; i++) {
		float ovy, nvy;
		mixed *particle;

		particle = particles[i];

		ovy = particle[3];
		nvy = ovy + diff * (float)(screen_width / 2);

		particle[0] += particle[2] * diff;
		particle[1] += (ovy + nvy) * 0.5 * diff;
		particle[3] = nvy;

		if (particle[1] >= (float)(screen_height)) {
			reset_particle(particle);
		}

		x = (int)floor(particle[0]);
		y = (int)floor(particle[1]);
		paint->move_pen(x, y);

		switch(particle[4]) {
		case 0: paint->set_color(0x8f); break;
		case 1: paint->set_color(0x87); break;
		case 2: paint->set_color(0x88); break;
		case 3: paint->set_color(0x81); break;
		case 4: paint->set_color(0x89); break;
		case 5: paint->set_color(0x83); break;
		case 6: paint->set_color(0x8b); break;
		}

		paint->draw("+");
	}
}

static void do_frame(float diff)
{
	object paint;
	object gc;

	send_out("\033[0m\033[1;1H");

	paint = new_object(LWO_PAINTER);
	paint->set_size(screen_width, screen_height);
	paint->add_layer("default");
	paint->set_layer_size("default", screen_width, screen_height);

	gc = paint->create_gc();
	gc->set_layer("default");
	gc->set_clip(0, 0, screen_width - 1, screen_height - 1);

	do_particles(gc, diff);

	send_out(implode(paint->render_color(), "\n"));
}

void end()
{
	destruct_object(this_object());
}
