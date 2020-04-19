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
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/log.h>
#include <status.h>

#define K (1 << 10)
#define M (1 << 20)
#define G (1 << 30)

inherit SECOND_AUTO;

static void create()
{
}

static void check()
{
	float dmem_size;
	float dmem_used;

	wipe_callouts();

	call_out("check", 1);

	dmem_size = (float)status(ST_DMEMSIZE);
	dmem_used = (float)status(ST_DMEMUSED);

	if (dmem_used / dmem_size < 0.75) {
		frag = 1;
	}

	if (dmem_size - dmem_used > (float)M * 64.0) {
		slack = 1;
	}

	if (frag && slack) {
		LOGD->post_message("system", LOG_NOTICE, "SwapD: Swapping out");
	}
}
