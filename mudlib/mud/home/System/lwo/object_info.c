#include <kernel/kernel.h>

#include <kotaka/privilege.h>
#include <kotaka/paths.h>
#include <kotaka/assert.h>

#include <status.h>

inherit SECOND_AUTO;

/*************/
/* Variables */
/*************/

string path;			/* the canonical path for this object */

int *inherits;			/* program numbers of inherited objects */
string *includes;		/* canonical include files */

int destructed;			/* destructed */

string constructor;		/* constructor */
string destructor;		/* destructor */

string *inherited_constructors;	/* constructors */
string *inherited_destructors;	/* destructors */

int nclones;			/* number of clones */
int overflow;			/* if we overflowed the array */
mapping clones;			/* clones */

/****************/
/* Declarations */
/****************/

static void create(int clone);
void set_path(string new_path);
void set_inherits(int *new_inherits);
void set_includes(string *new_includes);
void set_destructed();

void set_constructor(string constructor);
void set_destructor(string destructor);

void set_inherited_constructors(string *constructors);
void set_inherited_destructors(string *destructors);

void add_clone(object clone);
void remove_clone(object clone);

string query_path();
int *query_inherits();
string *query_includes();
int query_destructed();

string query_constructor();
string query_destructor();

string *query_inherited_constructors();
string *query_inherited_destructors();

object *query_clones();
int query_clone_count();

/***************/
/* definitions */
/***************/

static void create(int clone)
{
}

void set_path(string new_path)
{
	ACCESS_CHECK(SYSTEM());

	path = new_path;
}

void set_inherits(int *new_inherits)
{
	ACCESS_CHECK(SYSTEM());

	inherits = new_inherits;
}

void set_includes(string *new_includes)
{
	ACCESS_CHECK(SYSTEM());

	includes = new_includes;
}

void set_destructed()
{
	ACCESS_CHECK(SYSTEM());

	destructed = 1;
}

void set_constructor(string new_constructor)
{
	ACCESS_CHECK(SYSTEM());

	constructor = new_constructor;
}

void set_destructor(string new_destructor)
{
	ACCESS_CHECK(SYSTEM());

	destructor = new_destructor;
}

void set_inherited_constructors(string *constructors)
{
	ACCESS_CHECK(SYSTEM());

	inherited_constructors = constructors;
}

void set_inherited_destructors(string *destructors)
{
	ACCESS_CHECK(SYSTEM());

	inherited_destructors = destructors;
}

void add_clone(object clone)
{
	ACCESS_CHECK(SYSTEM());

	nclones++;

	if (overflow) {
		return;
	}

	if (nclones > status(ST_ARRAYSIZE)) {
		overflow = 1;
		clones = nil;
		return;
	}

	if (!clones) {
		clones = ([ ]);
	}

	clones[clone] = 1;
}

void remove_clone(object clone)
{
	ACCESS_CHECK(SYSTEM());
	ASSERT(nclones > 0);

	nclones--;

	if (overflow) {
		return;
	}

	clones[clone] = nil;
}

string query_path()
{
	return path;
}

int *query_inherits()
{
	return inherits[..];
}

string *query_includes()
{
	return includes[..];
}

int query_destructed()
{
	return destructed;
}

string query_constructor()
{
	return constructor;
}

string query_destructor()
{
	return destructor;
}

string *query_inherited_constructors()
{
	return inherited_constructors;
}

string *query_inherited_destructors()
{
	return inherited_destructors;
}

object *query_clones()
{
	if (overflow) {
		return nil;
	}

	return clones ? map_indices(clones) : ({ });
}

int query_clone_count()
{
	return nclones;
}
