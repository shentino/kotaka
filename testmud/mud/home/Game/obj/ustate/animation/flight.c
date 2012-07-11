#include <kotaka/paths.h>
#include <kotaka/privilege.h>

#include <game/paths.h>
#include <status.h>

inherit "~/lib/animate";

float **particles;
float speed;

#define NPARTICLES 100

static void create(int clone)
{
	::create();
}

static void destruct(int clone)
{
	::destruct();
}

void begin()
{
	int i;

	::begin();

	speed = 5.0;
	send_out("\033[1;1H\033[2J");

	particles = allocate(NPARTICLES);

	for (i = 0; i < NPARTICLES; i++) {
		particles[i] = allocate_float(3);
		particles[i][0] = SUBD->rnd() * 400.0 - 200.0;
		particles[i][1] = SUBD->rnd() * 100.0 - 50.0;
		particles[i][2] = SUBD->rnd() * 10.0 + 1.0;
	}
}

int zcomp(mixed *a, mixed *b)
{
	if (a[2] > b[2]) {
		return 1;
	}
}

private void do_particles(object paint, float diff)
{
	int x, y, i, sortflag;

	for (i = 0; i < NPARTICLES; i++) {
		mixed *particle;

		particle = particles[i];
		particle[2] -= diff * 5.0;

		if (particle[2] < 1.0) {
			particle[0] = SUBD->rnd() * 400.0 - 200.0;
			particle[1] = SUBD->rnd() * 100.0 - 50.0;
			particle[2] = SUBD->rnd() * 1.0 + 10.0;
			sortflag = 1;
		}
	}

	if (sortflag) {
		SUBD->qsort(particles, 0, NPARTICLES, "zcomp");
	}

	for (i = NPARTICLES - 1; i >= 0; i--) {
		mixed *particle;
		float depth;

		particle = particles[i];
		depth = particle[2];

		x = (int)floor(particle[0] / depth + 40.0);
		y = (int)floor(particle[1] / depth + 12.5);

		paint->move_pen(x, y);

		if (depth < 5.0) {
			paint->set_color(0xF);
			paint->draw("@");
		} else if (depth < 7.5) {
			paint->set_color(0x7);
			paint->draw("*");
		} else {
			paint->set_color(0x8);
			paint->draw("+");
		}

	}
}

static void do_frame(float diff)
{
	object paint;

	paint = new_object(LWO_PAINTER);
	paint->start(80, 25);
	paint->set_color(0xF);

	do_particles(paint, diff);

	send_out("\033[1;1H");
	send_out(paint->render_color());
}
