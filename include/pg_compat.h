/*--------------------------------------------------------------------
 * pg_compat.h
 *
 * Various backward compatibility code to help writing extensions.
 *
 * Copyright (c) 2023, PostgreSQL Global Development Group
 *
 *--------------------------------------------------------------------
 */
#ifndef PG_COMPAT_H
#define PG_COMPAT_H

#if PG_VERSION_NUM <= 100000
#error Major version not supported
#endif

/*--- Variables exported ---*/

extern bool my_bool_guc;
extern int	my_int_guc;
extern char *my_string_guc;

/*--- Some general backwards compatibility macros ---*/

#if PG_VERSION_NUM < 150000
#define MarkGUCPrefixReserved(className) EmitWarningsOnPlaceholders(className)
#endif

/*--- Hooks prototypes and argument names ---*/

/* ExecutorStart hook */
#if PG_VERSION_NUM >= 180000
#define EXEC_START_RET	bool
#else
#define EXEC_START_RET	void
#endif
#if PG_VERSION_NUM >= 100000
#define EXECUTOR_START_HOOK_ARGS QueryDesc *queryDesc, \
							int eflags
#define EXECUTOR_START_HOOK_ARG_NAMES queryDesc, \
							eflags
#endif
/* end of ExecutorStart hook */

/* ExecutorRun hook */
#if PG_VERSION_NUM >= 180000
#define EXECUTOR_RUN_HOOK_ARGS QueryDesc *queryDesc, \
							ScanDirection direction, \
							uint64 count
#define EXECUTOR_RUN_HOOK_ARG_NAMES queryDesc, \
							direction, \
							count
#else
#define EXECUTOR_RUN_HOOK_ARGS QueryDesc *queryDesc, \
							ScanDirection direction, \
							uint64 count, \
							bool execute_once
#define EXECUTOR_RUN_HOOK_ARG_NAMES queryDesc, \
							direction, \
							count, \
							execute_once
#endif
/* end of ExecutorRun hook */

/* ExecutorFinish hook */
#if PG_VERSION_NUM >= 100000
#define EXECUTOR_FINISH_HOOK_ARGS QueryDesc *queryDesc
#define EXECUTOR_FINISH_HOOK_ARG_NAMES queryDesc
#endif
/* end of ExecutorFinish hook */

/* ExecutorEnd hook */
#if PG_VERSION_NUM >= 100000
#define EXECUTOR_END_HOOK_ARGS QueryDesc *queryDesc
#define EXECUTOR_END_HOOK_ARG_NAMES queryDesc
#endif
/* end of ExecutorEnd hook */

/* planner_hook */
#if PG_VERSION_NUM >= 130000
#define PLANNER_HOOK_ARGS Query *parse, \
							const char *query_string, \
							int cursorOptions, \
							ParamListInfo boundParams
#define PLANNER_HOOK_ARG_NAMES parse, \
							query_string, \
							cursorOptions, \
							boundParams
#elif PG_VERSION_NUM >= 100000
#define PLANNER_HOOK_ARGS Query *parse, \
							int cursorOptions, \
							ParamListInfo boundParams
#define PLANNER_HOOK_ARG_NAMES parse, \
							cursorOptions, \
							boundParams
#endif
/* end of planner_hook */

/* post_parse_analyze hook */
#if PG_VERSION_NUM >= 140000
#define POST_PARSE_ANALYZE_HOOK_ARGS ParseState *pstate, \
							Query *query, \
							JumbleState *jstate
#define POST_PARSE_ANALYZE_HOOK_ARG_NAMES pstate, \
							query, \
							jstate
#elif PG_VERSION_NUM >= 100000
#define POST_PARSE_ANALYZE_HOOK_ARGS ParseState *pstate, \
							Query *query
#define POST_PARSE_ANALYZE_HOOK_ARG_NAMES pstate, \
							query
#endif
/* end of post_parse_analyze hook */

/* ProcessUtility_hook */
#if PG_VERSION_NUM >= 140000
#define UTILITY_HOOK_ARGS PlannedStmt *pstmt, const char *queryString, \
							bool readOnlyTree, \
							ProcessUtilityContext context, ParamListInfo params, \
							QueryEnvironment *queryEnv, \
							DestReceiver *dest, QueryCompletion *qc
#define UTILITY_HOOK_ARG_NAMES pstmt, queryString, \
							readOnlyTree, \
							context, params, \
							queryEnv, \
							dest, qc
#elif PG_VERSION_NUM >= 130000
#define UTILITY_HOOK_ARGS PlannedStmt *pstmt, \
							const char *queryString, \
							ProcessUtilityContext context, \
							ParamListInfo params, \
							QueryEnvironment *queryEnv, \
							DestReceiver *dest, QueryCompletion *qc
#define UTILITY_HOOK_ARG_NAMES pstmt, \
							queryString, \
							context, \
							params, \
							queryEnv, \
							dest, qc
#elif PG_VERSION_NUM >= 100000
#define UTILITY_HOOK_ARGS PlannedStmt *pstmt, \
							const char *queryString, \
							ProcessUtilityContext context, \
							ParamListInfo params, \
							QueryEnvironment *queryEnv, \
							DestReceiver *dest, char *completionTag
#define UTILITY_HOOK_ARG_NAMES pstmt, \
							queryString, \
							context, \
							params, \
							queryEnv, \
							dest, completionTag
#endif
/* end of ProcessUtility_hook */

/* shmem_request_hook */
#if PG_VERSION_NUM >= 150000
#define SHMEM_REQUEST_HOOK_ARGS void
#define SHMEM_REQUEST_HOOK_ARG_NAMES
#endif
/* end of shmem_request_hook */

/* shmem_startup_hook */
#if PG_VERSION_NUM >= 100000
#define SHMEM_STARTUP_HOOK_ARGS void
#define SHMEM_STARTUP_HOOK_ARG_NAMES
#endif
/* end of shmem_startup_hook */

#endif		/* PG_COMPAT_H */
