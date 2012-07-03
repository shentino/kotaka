#include <status.h>

#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

int callout;
int frag_level;

void freeze();
void thaw();

static void create()
{
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || LOCAL());

	if (callout) {
		error("Watchdog already enabled");
	}

	rlimits (0; -1) {
		callout = call_out("check", 1);
	}
}

void disable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN());

	if (!callout) {
		error("Watchdog not enabled");
	}

	rlimits (0; -1) {
		remove_call_out(callout);
		callout = 0;
	}
}

int enabled()
{
	return !!callout;
}

void reboot()
{
	ACCESS_CHECK(SYSTEM());
}

static void check()
{
	float mem_size;
	float mem_used;
	float mem_free;

	int obj_size;
	int obj_used;
	int obj_free;

	int swap_size;
	int swap_used;
	int swap_free;

	int freeze;

	rlimits(0; -1) {
		callout = call_out("check", 1);
	}

	mem_used = (float)status(ST_SMEMUSED) + (float)status(ST_DMEMUSED);
	mem_size = (float)status(ST_SMEMSIZE) + (float)status(ST_DMEMSIZE);
	mem_free = mem_size - mem_used;

	obj_used = status(ST_NOBJECTS);
	obj_size = status(ST_OTABSIZE);
	obj_free = obj_size - obj_used;

	swap_used = status(ST_SWAPUSED);
	swap_size = status(ST_SWAPSIZE);
	swap_free = swap_size - swap_used;

	if (mem_used > (float)(1 << 30)) {
		LOGD->post_message("watchdog", LOG_NOTICE, "Memory full, swapping out");
		swapout();
		return;
	}

	if ((mem_free - (float)(16 << 20)) / mem_size > 0.25) {
		frag_level++;
	} else if (frag_level) {
		frag_level--;
	}

	if (frag_level > 60) {
		LOGD->post_message("watchdog", LOG_NOTICE, "Memory fragmented, swapping out");
		swapout();
		frag_level = 0;
	}
}
