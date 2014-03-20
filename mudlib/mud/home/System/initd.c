/*
 * This file is part of Kotaka, a mud library for DGD
 * http://github.com/shentino/kotaka
 *
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014  Raymond Jennings
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
#include <kernel/access.h>
#include <kernel/kernel.h>
#include <kernel/version.h>
#include <kotaka/log.h>
#include <kotaka/paths/bigstruct.h>
#include <kotaka/paths/kotaka.h>
#include <kotaka/paths/system.h>
#include <kotaka/privilege.h>
#include <kotaka/version.h>
#include <status.h>
#include <trace.h>
#include <type.h>

#define CONSOLE_LOGLEVEL 6

inherit SECOND_AUTO;
inherit LIB_INITD;
inherit UTILITY_COMPILE;

int dumped;

mapping subsystems;

void console_post(string str, int level);
void message(string str);
void boot_subsystem(string subsystem);

private void configure_klib();
void configure_logging();
void configure_rsrc();
private void check_config();
private void check_versions();

private void initialize()
{
	subsystems = ([ ]);

	load_object(ERRORD);
	ERRORD->enable();

	load_object(LOGD);

	remove_file("/log/session.log");
	configure_logging();

	LOGD->post_message("boot", LOG_INFO, "Logging initialized");
	LOGD->post_message("boot", LOG_INFO, "Welcome to Kotaka " + KOTAKA_VERSION);

	LOGD->post_message("boot", LOG_INFO, "Loading kernel manager");
	load_object(KERNELD);
	configure_klib();
	configure_rsrc();

	KERNELD->set_global_access("System", 1);

	boot_subsystem("String");
	boot_subsystem("Algorithm");

	LOGD->post_message("boot", LOG_INFO, "Loading program manager");
	load_object(PROGRAM_INFO);
	load_object(PROGRAMD);

	LOGD->post_message("boot", LOG_INFO, "Loading object manager");
	load_object(OBJECTD);

	LOGD->post_message("boot", LOG_INFO, "Initializing program database");
	OBJECTD->discover_objects();

	boot_subsystem("Bigstruct");
	LOGD->post_message("boot", LOG_INFO, "Switching program database to bigstruct");
	PROGRAMD->convert_database();

	LOGD->post_message("boot", LOG_INFO, "Loading clone manager");
	load_object(CLONE_INFO);
	load_object(CLONED);

	LOGD->post_message("boot", LOG_INFO, "Loading system");
	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("sys", 1);

	LOGD->post_message("boot", LOG_INFO, "Configuring user manager");
	SYSTEM_USERD->set_reserve(2);
	SYSTEM_USERD->enable();

	LOGD->post_message("boot", LOG_INFO, "Starting status manager");
	STATUSD->enable();

	LOGD->post_message("boot", LOG_INFO, "Starting watchdog");
	WATCHDOGD->enable();

	/* Booted up */

	LOGD->post_message("boot", LOG_INFO, "System ready");

	boot_subsystem("Kotaka");
	boot_subsystem("Game");
	boot_subsystem("Test");
}

static void create()
{
	check_config();
	check_versions();

	rlimits (0; -1) {
		catch {
			initialize();
			call_out("audit_filequota", 0);
		} : {
			LOGD->flush();
			shutdown();
			error("System setup failed");
		}
	}
}

int booting()
{
	mixed *frame;

	frame = call_trace()[0];

	return frame[TRACE_PROGNAME] == DRIVER
		&& frame[TRACE_FUNCTION] == "initialize";
}

int restoring()
{
	mixed *frame;

	frame = call_trace()[0];

	return frame[TRACE_PROGNAME] == DRIVER
		&& frame[TRACE_FUNCTION] == "restored";
}

void prepare_reboot()
{
	int sz;
	int index;
	string *ind;

	ACCESS_CHECK(KERNEL());

	ACCESSD->save();

	LOGD->post_message("system", LOG_NOTICE, "InitD: Preparing for reboot");

	ind = map_indices(subsystems - ({ "System" }));
	sz = sizeof(ind);

	for (index = 0; index < sz; index++) {
		catch {
			LOGD->post_message("system", LOG_NOTICE, "sending prepare_reboot to " + ind[index]);
			call_other(USR_DIR + "/" + ind[index] + "/initd", "prepare_reboot");
		}
	}
}

void clear_admin()
{
	string *resources;
	int index;

	resources = KERNELD->query_resources();

	for (index = 0; index < sizeof(resources); index++) {
		KERNELD->rsrc_set_limit("admin", resources[index],
			-1);
	}
}

void hotboot()
{
	int index;
	int sz;
	string *ind;

	ACCESS_CHECK(KERNEL());

	check_config();
	check_versions();

	LOGD->post_message("system", LOG_NOTICE, "hotbooted");

	clear_admin();
	configure_rsrc();
	remove_call_out(dumped);
	dumped = 0;

	ind = map_indices(subsystems - ({ "System" }));
	sz = sizeof(ind);

	for (index = 0; index < sz; index++) {
		catch {
			LOGD->post_message("system", LOG_NOTICE, "Sending hotboot to " + ind[index]);
			call_other(USR_DIR + "/" + ind[index] + "/initd", "hotboot");
		}
	}

	call_out("audit_filequota", 0);
}

void reboot()
{
	int index;
	int sz;
	string *ind;

	ACCESS_CHECK(KERNEL());

	check_config();
	check_versions();

	LOGD->post_message("system", LOG_NOTICE, "rebooted");

	clear_admin();
	configure_rsrc();
	remove_call_out(dumped);
	dumped = 0;

	ACCESSD->restore();
	OBJECTD->reboot();
	WATCHDOGD->reboot();
	SYSTEM_USERD->reboot();

	ind = map_indices(subsystems - ({ "System" }));
	sz = sizeof(ind);

	for (index = 0; index < sz; index++) {
		catch {
			LOGD->post_message("system", LOG_NOTICE, "Sending reboot to " + ind[index]);
			call_other(USR_DIR + "/" + ind[index] + "/initd", "reboot");
		}
	}

	call_out("audit_filequota", 0);
}

/********************/
/* Helper functions */
/********************/

/* Miscellaneous */

private void configure_klib()
{
	string *wizards;
	int index;

	KERNELD->add_owner("Secret");

	wizards = KERNELD->query_users();

	for (index = 0; index < sizeof(wizards); index++) {
		KERNELD->add_owner(wizards[index]);
	}
}

void configure_rsrc()
{
	KERNELD->set_rsrc("ticks", 1000000, 0, 0);
}

void configure_logging()
{
	/* wipe the slate clean */
	LOGD->clear_targets();

	/* log to console by default */
	LOGD->set_target("*", 127, "driver");

	/* don't log these to the console */
	LOGD->set_target("debug", 0, "driver");
	LOGD->set_target("compile", 63, "driver");
	/* LOGD->set_target("trace", 0, "driver"); */

	/* prevent default logging */
	LOGD->set_target("debug", 255, "null");
	LOGD->set_target("compile", 255, "null");
	LOGD->set_target("trace", 255, "null");

	/* general log gets everything */
	LOGD->set_target("*", 255, "file:/log/general.log");
	LOGD->set_target("debug", 255, "file:/log/general.log");
	LOGD->set_target("compile", 255, "file:/log/general.log");
	LOGD->set_target("trace", 63, "file:/log/general.log");

	/* error log gets errors and traces */
	LOGD->set_target("error", 255, "file:/log/error.log");
	LOGD->set_target("trace", 255, "file:/log/error.log");
	LOGD->set_target("compile", 63, "file:/log/error.log");

	/* session log gets only non debug */
	LOGD->set_target("*", 127, "file:/log/session.log");
	LOGD->set_target("debug", 0, "file:/log/session.log");

	/* debug log gets only debug */
	LOGD->set_target("*", 128, "file:/log/debug.log");
	LOGD->set_target("debug", 255, "file:/log/debug.log");

	/* general system log goes to general and logged in staff */
	LOGD->set_target("system", 255, "file:/log/general.log");
	LOGD->set_target("system", 255, "file:/log/session.log");
}

int forbid_inherit(string from, string path, int priv)
{
	string creator;
	int firstchar;

	ACCESS_CHECK(previous_program() == OBJECTD);

	if (sscanf(path, USR_DIR + "/System/lib/auto/%*s")) {
		return 0;
	}

	creator = DRIVER->creator(from);

	if (!creator) {
		return 1;
	}

	if (creator == "System") {
		return 0;
	}

	if (sscanf(path, USR_DIR + "/System/closed/%*s")) {
		return 1;
	}

	firstchar = creator[0];

	if (firstchar < 'A' || firstchar > 'Z') {
		return 1;
	}

	return 0;
}

private void check_config()
{
	if (status(ST_ARRAYSIZE) < 256) {
		error("Array size setting is too small");
	}
}

/* shamelessly stolen from phantasmal, kudos to Noah Gibbs */
private void check_versions()
{
	string dgdstatus;
	int major, minor, patch;

	dgdstatus = status(ST_VERSION);

	if (sscanf(dgdstatus, "DGD %d.%d.%d", major, minor, patch) != 3) {
		patch = 0;
		if (sscanf(dgdstatus, "DGD %d.%d", major, minor) != 2) {
			minor = 0;
			if(sscanf(dgdstatus, "DGD %d", major) != 1) {
				error("Cannot parse DGD driver version");
			}
		}
	}

	if (major < 1) {
		error("DGD major version " + major + " too low for this version of kotaka");
	}

	if (major >= 2) {
		error("DGD major version " + major + " too high for this version of kotaka");
	}

	if (minor < 4) {
		error("DGD minor version " + major + "." + minor + " too low for this version of kotaka");
	}

	if (sscanf(KERNEL_LIB_VERSION, "%d.%d.%d", major, minor, patch) != 3) {
		patch = 0;
		if (sscanf(KERNEL_LIB_VERSION, "%d.%d", major, minor) != 2) {
			minor = 0;
			if(sscanf(KERNEL_LIB_VERSION, "%d", major) != 1) {
				error("Cannot parse kernel library version");
			}
		}
	}

	if (major < 1) {
		error("Kernel library major version " + major + " too low for this version of kotaka");
	}

	if (major >= 2) {
		error("Kernel library major version " + major + " too high for this version of kotaka");
	}

	if (minor < 3) {
		error("Kernel library minor version " + major + "." + minor + " too low for this version of kotaka");
	}
}

void add_subsystem(object initd)
{
	ACCESS_CHECK(previous_program() == OBJECTD);

	subsystems[initd->query_owner()] = initd;
}

void boot_subsystem(string subsystem)
{
	if (find_object(USR_DIR + "/" + subsystem + "/initd")) {
		return;
	}

	KERNELD->add_user(subsystem);
	KERNELD->add_owner(subsystem);

	rlimits(100; -1) {
		load_object(USR_DIR + "/" + subsystem + "/initd");
	}

	if (!sizeof(KERNELD->query_global_access() & ({ subsystem }))) {
		error("Failure to grant global access by " + subsystem);
	}

	LOGD->post_message("boot", LOG_NOTICE, "Booted subsystem: " + subsystem);
}

void reboot_subsystem(string subsystem)
{
	object cursor;

	ACCESS_CHECK(INTERFACE() || KADMIN() || subsystem == DRIVER->creator(object_name(previous_object())));

	switch(subsystem) {
	case "Bigstruct":
	case "System":
		error("Cannot reboot " + subsystem);
	}

	while (cursor = KERNELD->first_link(subsystem)) {
		destruct_object(cursor);
	};

	compile_object(USR_DIR + "/" + subsystem + "/initd");
}

void shutdown_subsystem(string subsystem)
{
	object cursor;

	ACCESS_CHECK(INTERFACE() || KADMIN() || subsystem == DRIVER->creator(object_name(previous_object())));

	switch(subsystem) {
	case "Algorithm":
	case "Bigstruct":
	case "String":
	case "System":
		error("Cannot shutdown " + subsystem);
	}

	while (cursor = KERNELD->first_link(subsystem)) {
		destruct_object(cursor);
	};

	KERNELD->remove_user(subsystem);
}

void upgrading()
{
	if (!subsystems) {
		subsystems = ([ ]);
	}
}

string *query_subsystems()
{
	return map_indices(subsystems);
}

static void audit_filequota()
{
	DRIVER->fix_filequota();
}

void do_upgrade()
{
	ACCESS_CHECK(SYSTEM());

	load_dir("lwo", 1);
	load_dir("obj", 1);
	load_dir("sys", 1);
}
