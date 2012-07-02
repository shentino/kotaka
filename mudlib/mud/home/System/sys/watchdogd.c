#include <status.h>

#include <kernel/kernel.h>

#include <kotaka/paths.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>

inherit SECOND_AUTO;

int callout;
float frag_angst;

void freeze();
void thaw();

int swapover_actions;

static void create()
{
}

void enable()
{
	ACCESS_CHECK(SYSTEM() || KADMIN() || LOCAL());

	if (callout) {
		error("Watchdog already enabled");
	}

	LOGD->post_message("system", LOG_NOTICE, "Watchdog enabled");

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
		LOGD->post_message("system", LOG_NOTICE, "Watchdog disabled");
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
	int mem_size;
	int mem_used;
	int mem_free;

	int obj_size;
	int obj_used;
	int obj_free;

	int swap_size;
	int swap_used;
	int swap_free;

	int freeze;
	float frag_factor;

	rlimits(0; -1) {
		callout = call_out("check", 1);
	}

	mem_used = status(ST_SMEMUSED) + status(ST_DMEMUSED);
	mem_size = status(ST_SMEMSIZE) + status(ST_DMEMSIZE);
	mem_free = mem_size - mem_used;

	obj_used = status(ST_NOBJECTS);
	obj_size = status(ST_OTABSIZE);
	obj_free = obj_size - obj_used;

	swap_used = status(ST_SWAPUSED);
	swap_size = status(ST_SWAPSIZE);
	swap_free = swap_size - swap_used;

	if (mem_used >> 20 > 512) {
		LOGD->post_message("watchdog", LOG_NOTICE, "Memory full, swapping out");
		frag_angst = 0.0;
		return;
	}

	frag_factor = ((float)mem_free / (float)(mem_size + (16 << 20))) - 0.25;
	frag_angst += frag_factor;

	if (frag_angst <= 0.0) {
		frag_angst = 0.0;
	}

	LOGD->post_message("watchdog", LOG_NOTICE, "Free memory at " + mem_free);
	LOGD->post_message("watchdog", LOG_NOTICE, "Fragmentation factor at " + frag_factor);
	LOGD->post_message("watchdog", LOG_NOTICE, "Fragmentation angst at " + frag_angst);

	if (frag_angst > 1.0) {
		LOGD->post_message("watchdog", LOG_NOTICE, "Memory fragmented, swapping out");
		frag_angst = 0.0;
		swapout();
	}
}
