/*-------------------------------------------------------------------------
 *
 * pg_extension_template.c: Short Description
 *
 * Longer Description
 *
 * This program is open source, licensed under the PostgreSQL license.
 * For license terms, see the LICENSE file.
 *
 * Copyright (c) 2023, PostgreSQL Global Development Group
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "executor/executor.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "optimizer/planner.h"
#include "parser/analyze.h"
#include "storage/ipc.h"
#include "storage/lwlock.h"
#include "storage/shmem.h"
#include "tcop/utility.h"
#include "utils/builtins.h"

/*--- Local includes ---*/

#include "include/pg_compat.h"

PG_MODULE_MAGIC;

/*--- Structs & macro---*/

#define PGET_SRF_COLS	2

/* Global shared state */
typedef struct pgetSharedState
{
	LWLock	   *lock;		/* protects FIXME */
} pgetSharedState;

/*--- Variables exported ---*/

bool		my_bool_guc;
int			my_int_guc;
char	   *my_string_guc;

/*--- Private variables ---*/

/* Links to shared memory state */
static pgetSharedState *pget = NULL;

/*--- Functions --- */

PGDLLEXPORT void _PG_init(void);

static void pget_ExecutorStart_hook(EXECUTOR_START_HOOK_ARGS);
static ExecutorStart_hook_type prev_ExecutorStart = NULL;

static void pget_ExecutorRun_hook(EXECUTOR_RUN_HOOK_ARGS);
static ExecutorRun_hook_type prev_ExecutorRun = NULL;

static void pget_ExecutorFinish_hook(EXECUTOR_FINISH_HOOK_ARGS);
static ExecutorFinish_hook_type prev_ExecutorFinish = NULL;

static void pget_ExecutorEnd_hook(EXECUTOR_END_HOOK_ARGS);
static ExecutorEnd_hook_type prev_ExecutorEnd = NULL;

static PlannedStmt *pget_planner_hook(PLANNER_HOOK_ARGS);
static planner_hook_type prev_planner = NULL;

static void pget_post_parse_analyze_hook(POST_PARSE_ANALYZE_HOOK_ARGS);
static post_parse_analyze_hook_type prev_post_parse_analyze;

static void pget_ProcessUtility_hook(UTILITY_HOOK_ARGS);
static ProcessUtility_hook_type prev_ProcessUtility = NULL;

#if PG_VERSION_NUM >= 150000
static void pget_shmem_request_hook(SHMEM_REQUEST_HOOK_ARGS);
static shmem_request_hook_type prev_shmem_request = NULL;
#endif

static void pget_shmem_startup_hook(SHMEM_STARTUP_HOOK_ARGS);
static shmem_startup_hook_type prev_shmem_startup = NULL;

static void pget_shmem_shutdown(int code, Datum arg);

static void pget_request_resources(void);
static Size pget_memsize(void);

PGDLLEXPORT Datum pg_extension_template(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(pg_extension_template);


/*
 * Module load callback
 */
void
_PG_init(void)
{
	if (!process_shared_preload_libraries_in_progress)
	{
		elog(ERROR, "This module can only be loaded via shared_preload_libraries");
		return;
	}

	/* Install hooks */
	prev_ExecutorStart = ExecutorStart_hook;
	ExecutorStart_hook = pget_ExecutorStart_hook;

	prev_ExecutorRun = ExecutorRun_hook;
	ExecutorRun_hook = pget_ExecutorRun_hook;

	prev_ExecutorFinish = ExecutorFinish_hook;
	ExecutorFinish_hook = pget_ExecutorFinish_hook;

	prev_ExecutorEnd = ExecutorEnd_hook;
	ExecutorEnd_hook = pget_ExecutorEnd_hook;

	prev_planner = planner_hook;
	planner_hook = pget_planner_hook;

	prev_post_parse_analyze = post_parse_analyze_hook;
	post_parse_analyze_hook = pget_post_parse_analyze_hook;

	prev_ProcessUtility = ProcessUtility_hook;
	ProcessUtility_hook = pget_ProcessUtility_hook;

#if PG_VERSION_NUM >= 150000
	prev_shmem_request = shmem_request_hook;
	shmem_request_hook = pget_shmem_request_hook;
#endif

	prev_shmem_startup = shmem_startup_hook;
	shmem_startup_hook = pget_shmem_startup_hook;

	/* Define (or redefine) custom GUC variables. */
	DefineCustomBoolVariable("pg_extension_template.my_bool_guc",
							 "Short Description",
							 NULL,
							 &my_bool_guc,
							 true,			/* init value */
							 PGC_USERSET,	/* GucContext */
							 0,				/* flags, see utils/guc.h */
							 NULL,			/* check_hook */
							 NULL,			/* assign_hook */
							 NULL			/* show_hook */
							 );

	DefineCustomIntVariable("pg_extension_template.my_int_guc",
							"Short Description",
							NULL,
							&my_int_guc,
							0,				/* init value */
							0,				/* min value */
							INT_MAX,		/* max_value */
							PGC_USERSET,	/* GucContext */
							0,				/* flags, see utils/guc.h */
							NULL,			/* check_hook */
							NULL,			/* assign_hook */
							NULL			/* show_hook */
							);

	DefineCustomStringVariable("pg_extension_template.my_string_guc",
							   "Short Description",
							   NULL,
							   &my_string_guc,
							   "FIXME",		/* init value */
							   PGC_USERSET,	/* GucContext */
							   0,			/* flags, see utils/guc.h */
							   NULL,		/* check_hook */
							   NULL,		/* assign_hook */
							   NULL			/* show_hook */
							   );

	MarkGUCPrefixReserved("pg_extension_template");

#if PG_VERSION_NUM < 150000
	/*
	 * For pg14- we request additional resources here.  For pg15+ we rely on
	 * shmem_request_hook.
	 */
	pget_request_resources();
#endif
}

/*
 * ExecutorStart hook
 */
static void
pget_ExecutorStart_hook(EXECUTOR_START_HOOK_ARGS)
{
	if (prev_ExecutorStart)
		prev_ExecutorStart(EXECUTOR_START_HOOK_ARG_NAMES);
	else
		standard_ExecutorStart(EXECUTOR_START_HOOK_ARG_NAMES);
}

/*
 * ExecutorRun hook
 */
static void
pget_ExecutorRun_hook(EXECUTOR_RUN_HOOK_ARGS)
{
	if (prev_ExecutorRun)
		prev_ExecutorRun(EXECUTOR_RUN_HOOK_ARG_NAMES);
	else
		standard_ExecutorRun(EXECUTOR_RUN_HOOK_ARG_NAMES);
}

/*
 * ExecutorFinish hook
 */
static void
pget_ExecutorFinish_hook(EXECUTOR_FINISH_HOOK_ARGS)
{
	if (prev_ExecutorFinish)
		prev_ExecutorFinish(EXECUTOR_FINISH_HOOK_ARG_NAMES);
	else
		standard_ExecutorFinish(EXECUTOR_FINISH_HOOK_ARG_NAMES);
}

/*
 * ExecutorEnd hook
 */
static void
pget_ExecutorEnd_hook(EXECUTOR_END_HOOK_ARGS)
{
	if (prev_ExecutorEnd)
		prev_ExecutorEnd(EXECUTOR_END_HOOK_ARG_NAMES);
	else
		standard_ExecutorEnd(EXECUTOR_END_HOOK_ARG_NAMES);
}

/*
 * planner hook
 */
static PlannedStmt *
pget_planner_hook(PLANNER_HOOK_ARGS)
{
	if (prev_planner)
		return prev_planner(PLANNER_HOOK_ARG_NAMES);
	else
		return standard_planner(PLANNER_HOOK_ARG_NAMES);
}

/*
 * post_parse_analyze hook
 */
static void
pget_post_parse_analyze_hook(POST_PARSE_ANALYZE_HOOK_ARGS)
{
	if (prev_post_parse_analyze)
		prev_post_parse_analyze(POST_PARSE_ANALYZE_HOOK_ARG_NAMES);
}

/*
 * ProcessUtility hook
*/
static void
pget_ProcessUtility_hook(UTILITY_HOOK_ARGS)
{
	if (prev_ProcessUtility)
		prev_ProcessUtility(UTILITY_HOOK_ARG_NAMES);
	else
		standard_ProcessUtility(UTILITY_HOOK_ARG_NAMES);
}

#if PG_VERSION_NUM >= 150000
/*
 * shmem_request hook: request additional shared resources.  We will allocate
 * or attach to the shared resources in pget_shmem_startup_hook().
*/
static void
pget_shmem_request_hook(SHMEM_REQUEST_HOOK_ARGS)
{
	if (prev_shmem_request)
		prev_shmem_request(SHMEM_REQUEST_HOOK_ARG_NAMES);

	pget_request_resources();
}
#endif

/*
 * shmem_startup hook: allocate or attach to shared memory.
*/
static void
pget_shmem_startup_hook(SHMEM_STARTUP_HOOK_ARGS)
{
	bool		found;

	if (prev_shmem_startup)
		prev_shmem_startup(SHMEM_STARTUP_HOOK_ARG_NAMES);

	/* reset in case this is a restart within the postmaster */
	pget = NULL;

	/* Create or attach to the shared memory state */
	LWLockAcquire(AddinShmemInitLock, LW_EXCLUSIVE);

	pget = ShmemInitStruct("pg_extension_template",
						   sizeof(pgetSharedState),
						   &found);

	if (!found)
	{
		/* First time through... */
		pget->lock = &(GetNamedLWLockTranche("pg_extension_template"))->lock;
	}

	LWLockRelease(AddinShmemInitLock);


	/*
	 * If we're in the postmaster (or a standalone backend...), set up a shmem
	 * exit hook to dump the statistics to disk.
	 */
	if (!IsUnderPostmaster)
		on_shmem_exit(pget_shmem_shutdown, (Datum) 0);

}

/*
 * shmem_shutdown hook: FIXME
 */
static void
pget_shmem_shutdown(int code, Datum arg)
{
	/* FIXME */
}

/*
 * pget_request_resources: Request additional shared resources.
 *
 * This function is either called in shmem_request_hook (for pg15+) or
 * _PG_init() (pg14-).
 */
static void
pget_request_resources(void)
{
	/*
	 * Request additional shared resources.  (These are no-ops if we're not in
	 * the postmaster process.)  We'll allocate or attach to the shared
	 * resources in pget_shmem_startup().
	 */
	RequestAddinShmemSpace(pget_memsize());
	RequestNamedLWLockTranche("pg_extension_template", 1);
}

/*
 * Estimate shared memory space needed.
 */
static Size
pget_memsize(void)
{
	Size		size;

	/* Size of the main shared state area, maxaligned */
	size = MAXALIGN(sizeof(pgetSharedState));

	/* Add any extraneous shared memory usage here */
	size = add_size(size, 0);

	return size;
}

/*
 * Some SRF example.
 */
Datum
pg_extension_template(PG_FUNCTION_ARGS)
{
	ReturnSetInfo *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;

	InitMaterializedSRF(fcinfo, 0);

	LWLockAcquire(pget->lock, LW_SHARED);

	/* Main function should loop here */
	{
		Datum		values[PGET_SRF_COLS];
		bool		nulls[PGET_SRF_COLS];
		int			i = 0;

		memset(values, 0, sizeof(values));
		memset(nulls, 0, sizeof(nulls));

		values[i++] = BoolGetDatum(true);
		values[i++] = CStringGetTextDatum("some text");

		tuplestore_putvalues(rsinfo->setResult, rsinfo->setDesc, values, nulls);
	}

	LWLockRelease(pget->lock);

	return (Datum) 0;
}
