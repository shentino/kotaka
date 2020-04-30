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
#include <kotaka/assert.h>
#include <kotaka/checkarg.h>
#include <kotaka/privilege.h>

private object environment;
private object *inventory;

void reset_id_number();

/* private helpers */

private atomic void move_core(object new_env)
{
	if (environment) {
		environment->thing_del_inventory(this_object());
		environment->bulk_invalidate();
	}

	environment = new_env;
	reset_id_number();

	if (environment) {
		environment->thing_add_inventory(this_object());
		environment->bulk_invalidate();
	}
}

/* creator */

static void create()
{
	inventory = ({ });
}

/* internal */

void thing_add_inventory(object arriving)
{
	ACCESS_CHECK(THING());

	ASSERT(arriving);
	ASSERT(!sizeof(inventory & ({ arriving })));

	inventory = ({ arriving }) + (inventory - ({ nil }));
}

void thing_del_inventory(object departing)
{
	ACCESS_CHECK(THING());

	ASSERT(departing);

	inventory -= ({ departing, nil });
}

/* hooks */

static int forbid_move(object new_env)
{
	return 0;
}

int forbid_insert(object obj)
{
	return 0;
}

int forbid_remove(object obj)
{
	return 0;
}

void move_notify(object old_env)
{
}

void insert_notify(object obj)
{
}

void remove_notify(object obj)
{
}

/* public */

int is_container_of(object test)
{
	object env;
	object this;
	mapping seen;

	this = this_object();
	env = test->query_environment();

	seen = ([ ]);

	while (env) {
		if (seen[env]) {
			error("Cyclic containment detected");
		}

		if (env == this) {
			return 1;
		}

		seen[env] = 1;

		env = env->query_environment();
	}

	return 0;
}

object query_environment()
{
	return environment;
}

object *query_inventory()
{
	return (inventory -= ({ nil }))[..];
}

void move(object new_env, varargs int force)
{
	object old_env;
	object this;

	old_env = environment;

	this = this_object();

	PERMISSION_CHECK(!old_env || !old_env->forbid_remove(this));
	PERMISSION_CHECK(!forbid_move(new_env));
	PERMISSION_CHECK(!new_env || !new_env->forbid_insert(this));

	if (new_env == environment) {
		return;
	}

	if (new_env) {
		object test;
		int levels;

		if (new_env == this) {
			error("Self containment attempted");
		}

		if (sizeof(new_env->query_inventory()) >= 100) {
			error("Destination object too cluttered");
		}

		test = new_env->query_environment();

		while (test) {
			if (test == this) {
				error("Cyclic containment attempted");
			}

			levels++;

			if (levels >= 32) {
				error("Object nesting overflow");
			}

			test = test->query_environment();
		}
	}

	move_core(new_env);

	if (old_env) {
		old_env->remove_notify(this);
	}

	move_notify(old_env);

	if (new_env) {
		new_env->insert_notify(this);
	}
}

void rearrange_inventory(object *new_inventory)
{
	/* nil */
	CHECKARG(sizeof(new_inventory & ({ nil })) == 0, 1, "rearrange_inventory");

	/* duplicates */
	CHECKARG(sizeof( ({ }) | new_inventory) == sizeof(new_inventory), 1, "rearrange_inventory");

	inventory -= ({ nil });

	/* new or removed */
	CHECKARG(sizeof(inventory ^ new_inventory) == 0, 1, "rearrange_inventory");

	inventory = new_inventory[..];
}
