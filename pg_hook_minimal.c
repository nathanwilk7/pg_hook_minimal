#include "postgres.h"
#include "fmgr.h"

// Include the relevant file[s] for the hook[s] you're overriding
// #include "optimizer/planner.h"

// https://www.postgresql.org/docs/current/xfunc-c.html
// Write PG_MODULE_MAGIC in one (and only one) of the module source files, after having included the header fmgr.h
PG_MODULE_MAGIC;

// This will store the previous planner hook, so that we can restore it on unload
// or forward calls to it if it is set.
static planner_hook_type prev_planner_hook = NULL;


/*
 * Planner hook: if planner_hook has already been overridden, forward to the
 * overriding planner (which we saved to prev_planner_hook in _PG_init).
 * If planner_hook has not been overriden (e.g.: is 0), then forward to the regular planner.
 */
static PlannedStmt *
pg_minimal_planner(Query *parse,
				   const char *query_string,
				   int cursorOptions,
				   ParamListInfo boundParams)
{
	PlannedStmt *result;

	// Example of sending a notice to the client
	elog(NOTICE, "Running hook pg_minimal_planner");

	// WARNING this simple example doesn't handle all special cases (e.g.: nesting),
	// See contrib/pg_stat_statements in the postgresql codebase for an example of how to handle
	// some special cases.
	if (prev_planner_hook)
	{
		result = prev_planner_hook(parse, query_string, cursorOptions, boundParams);
	}
	else
	{
		result = standard_planner(parse, query_string, cursorOptions, boundParams);
	}

	return result;
}

/*
 * Module load callback.
 */
void _PG_init(void)
{
	// Save the current planner_hook so that we can restore it on unload
	// and forward to it if it is set.
	prev_planner_hook = planner_hook;
	planner_hook = pg_minimal_planner;

	// Hacky example of printing in the logfile that this module has been loaded (not using official logging functions)
	printf("Loaded hook pg_minimal_planner");
}

/*
 * Module unload callback.
 */
void _PG_fini(void)
{
	// Restore the previous planner hook (if any).
	planner_hook = prev_planner_hook;
}
