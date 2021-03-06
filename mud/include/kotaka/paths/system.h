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
#include <config.h>

/* initd */

#define INITD		(USR_DIR + "/System/initd")

/* daemons */

#define CATALOGD	(USR_DIR + "/System/sys/catalogd")
#define DUMPD		(USR_DIR + "/System/sys/dumpd")
#define ERRORD		(USR_DIR + "/System/sys/errord")
#define ETCD		(USR_DIR + "/System/sys/etcd")
#define FILTERD		(USR_DIR + "/System/sys/filterd")
#define IDD		(USR_DIR + "/System/sys/idd")
#define KERNELD		(USR_DIR + "/System/sys/kerneld")
#define LOGD		(USR_DIR + "/System/sys/logd")
#define MODULED		(USR_DIR + "/System/sys/moduled")
#define OBJECTD		(USR_DIR + "/System/sys/objectd")
#define PATCHD		(USR_DIR + "/System/sys/patchd")
#define PROXYD		(USR_DIR + "/System/sys/proxyd")
#define SECRETD		(USR_DIR + "/System/sys/secretd")
#define STATUSD		(USR_DIR + "/System/sys/statusd")
#define SWAPD		(USR_DIR + "/System/sys/swapd")
#define SYSTEM_USERD	(USR_DIR + "/System/sys/userd")
#define TLSD		(USR_DIR + "/System/sys/tlsd")
#define TOUCHD		(USR_DIR + "/System/sys/touchd")
#define VARD		(USR_DIR + "/System/sys/vard")
#define WATCHDOGD	(USR_DIR + "/System/sys/watchdogd")

/* libs */

#define CALL_GUARD		(USR_DIR + "/System/lib/auto/call_guard")
#define SECOND_AUTO		(USR_DIR + "/System/lib/auto/second_auto")
#define LIB_FILTER		(USR_DIR + "/System/lib/filter")
#define LIB_SYSTEM_USER		(USR_DIR + "/System/lib/user")
#define LIB_SYSTEM_WIZTOOL	(USR_DIR + "/System/lib/wiztool")
#define LIB_USERD		(USR_DIR + "/System/lib/userd")
#define LIB_INITD		(USR_DIR + "/System/lib/initd")
#define LIB_SYSTEM		(USR_DIR + "/System/lib/system")
#define UTILITY_STRING		(USR_DIR + "/System/lib/utility/string")
#define UTILITY_COMPILE		(USR_DIR + "/System/lib/utility/compile")
#define UTILITY_ALGORITHM	(USR_DIR + "/System/lib/utility/algorithm")

/* lwos */

#define PROGRAM_INFO		(USR_DIR + "/System/lwo/program_info")
#define SPARSE_ARRAY		(USR_DIR + "/System/lwo/struct/sparse_array")

/* objs */

#define SYSTEM_CONN		(USR_DIR + "/System/obj/connection")
