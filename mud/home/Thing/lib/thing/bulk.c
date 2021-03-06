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
#include <kotaka/paths/thing.h>

object query_archetype();
object query_environment();
object *query_inventory();

/* bulk */
private float mass;		/* kg */
private float density;		/* kg/l */

private string mass_derivation;
/* absolute: mass is fixed */
/* inherit: mass is local mass times mass of archetype */

private int flexible;		/* flexible container */
private int virtual;		/* virtual container */

private float capacity;		/* m^3 */
private float max_mass;		/* kg */

/* caching */
private int bulk_dirty;			/* if cache is invalid */

private float cached_content_mass;	/* cached mass of our contents */
private float cached_content_volume;	/* cached volume of our contents */

float query_mass();
void set_mass(float new_mass);
void bulk_invalidate();			/* invalidate */
void bulk_sync(varargs int force);	/* synchronize bulk cache */

static void create()
{
	mass = 0.0;
	density = 1.0;
}

/* mass */

void set_local_mass(float new_mass)
{
	object env;

	if (virtual) {
		error("Cannot set mass of virtual object");
	}

	if (new_mass == mass) {
		return;
	}

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	mass = new_mass;
}

float query_local_mass()
{
	return mass;
}

atomic void set_mass_derivation(string derivation)
{
	object env;

	switch (derivation) {
	case nil: /* default: inherit */
	case "inherit":
	case "absolute":
		if (mass_derivation != derivation) {
			/* changed derivation, preserve value */
			float old_mass;

			old_mass = query_mass();
			mass_derivation = derivation;
			set_mass(old_mass); /* will invalidate */
		}
		break;

	default:
		error("Invalid mass derivation");
	}
}

string query_mass_derivation()
{
	return mass_derivation;
}

void set_mass(float new_mass)
{
	object arch;
	float factor;

	switch(mass_derivation) {
	case nil:
	case "inherit":
		arch = query_archetype();

		if (!arch || arch->query_virtual()) {
			set_local_mass(new_mass);
		} else {
			set_local_mass(new_mass / arch->query_mass());
		}
		break;

	case "absolute":
		set_local_mass(new_mass);
	}
}

float query_mass()
{
	object arch;
	float factor;

	switch(mass_derivation) {
	case nil:
	case "inherit":
		arch = query_archetype();

		if (!arch || arch->query_virtual()) {
			return mass;
		} else {
			return mass * arch->query_mass();
		}
		break;

	case "absolute":
		return mass;
	}
}

float query_total_mass()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return query_mass() + cached_content_mass;
}

float query_contained_mass()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return cached_content_mass;
}

void figure_mass(float volume)
{
	set_mass(volume / density * 1000.0);
}

/* density */

void set_density(float new_density)
{
	object env;

	if (virtual) {
		error("Cannot set density of virtual object");
	}

	if (new_density == density) {
		return;
	}

	if (env = query_environment()) {
		env->bulk_invalidate();
	}

	density = new_density;
}

float query_density()
{
	return density;
}

void figure_density(float volume)
{
	set_density(mass / volume / 1000.0);
}

/* volume */

float query_volume()
{
	if (virtual) {
		return 0.0;
	}

	return mass / density / 1000.0;
}

float query_total_volume()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	if (flexible || virtual) {
		return query_volume() + cached_content_volume;
	} else {
		return query_volume() + capacity;
	}
}

float query_contained_volume()
{
	if (bulk_dirty) {
		bulk_sync();
	}

	return cached_content_volume;
}

/* capacity */

void set_capacity(float new_capacity)
{
	object env;

	if (virtual) {
		error("Cannot set capacity of virtual object");
	}

	if (capacity == new_capacity) {
		return;
	}

	capacity = new_capacity;

	/* rigid objects change size when they change capacity */
	if (!flexible && (env = query_environment())) {
		env->bulk_invalidate();
	}
}

float query_capacity()
{
	return capacity;
}

void set_max_mass(float new_max_mass)
{
	if (virtual) {
		error("Cannot set max mass of virtual object");
	}

	max_mass = new_max_mass;
}

float query_max_mass()
{
	return max_mass;
}

/* flexible */

void set_flexible(int new_flexible)
{
	object env;

	new_flexible = !!new_flexible;

	if (flexible == new_flexible) {
		return;
	}

	flexible = new_flexible;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}
}

int query_flexible()
{
	return flexible;
}

void set_virtual(int new_virtual)
{
	object env;

	mass = 0.0;
	density = 0.0;
	capacity = 0.0;
	max_mass = 0.0;

	virtual = new_virtual;
}

int query_virtual()
{
	return virtual;
}

/***********/
/* Caching */
/***********/

int query_bulk_dirty()
{
	return bulk_dirty;
}

void bulk_sync(varargs int force)
{
	int i;
	int sz;
	object *inv;
	float mass_sum;
	float volume_sum;

	if (!bulk_dirty && !force) {
		return;
	}

	inv = query_inventory();
	sz = sizeof(inv);

	for (i = 0; i < sz; i++) {
		mass_sum += inv[i]->query_total_mass();
		volume_sum += inv[i]->query_total_volume();
	}

	cached_content_mass = mass_sum;
	cached_content_volume = volume_sum;

	bulk_dirty = 0;
}

void bulk_invalidate()
{
	object env;

	bulk_dirty = 1;

	if (env = query_environment()) {
		env->bulk_invalidate();
	}
}
