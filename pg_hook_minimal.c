#include "postgres.h"
#include "fmgr.h"
#include "optimizer/planner.h"
#include "nodes/makefuncs.h"
#include <string.h>
#include <time.h>

// https://github.com/eatonphil/pgtam/tree/main
// https://github.com/taminomara/psql-hooks/blob/master/Detailed.md
// https://stackoverflow.com/questions/23392218/hooks-not-working-in-postgresql-9-3-when-building-from-source
// https://wiki.postgresql.org/images/e/e3/Hooks_in_postgresql.pdf
// https://www.endpointdev.com/blog/2010/05/using-postgresql-hooks/
// https://archive.fosdem.org/2021/schedule/event/postgresql_extensibility/attachments/slides/4348/export/events/attachments/postgresql_extensibility/slides/4348/fosdem21_postgres_extensibility.pdf

// https://www.postgresql.org/docs/current/xfunc-c.html
PG_MODULE_MAGIC;

static planner_hook_type original_planner_hook = NULL;

/*
 * Note, copied from contrib/pg_stat_statements/pg_stat_statements.c
 */
static PlannedStmt *
pgplannerhook_planner(Query *parse,
                         const char *query_string,
                         int cursorOptions,
                         ParamListInfo boundParams)
{
        // Hardcoded cross join
        // TODO try removing random crap to see what happens?
        SeqScan* seqScanLeft;
        TargetEntry* targetEntry0;
        Var* var0;
        TargetEntry* targetEntry1;
        Var* var1;
        SeqScan* seqScanRight;
        Material* material;
        NestLoop* nestLoop;
        PlannedStmt *result;

        targetEntry0 = makeNode(TargetEntry);
        targetEntry0->resno = 1;
        var0 = makeNode(Var);
        var0->varno = 1;
        var0->varattno = 1;
        var0->vartype = 23;
        targetEntry0->expr = (Expr*) var0;

        targetEntry1 = makeNode(TargetEntry);
        targetEntry1->resno = 2;
        var1 = makeNode(Var);
        var1->varno = 1;
        var1->varattno = 2;
        var1->vartype = 23;
        targetEntry1->expr = (Expr*) var1;

        seqScanLeft = makeNode(SeqScan);
        seqScanLeft->scan.plan.targetlist = list_make2(targetEntry0, targetEntry1);
        seqScanLeft->scan.scanrelid = ((RangeTblRef*) parse->jointree->fromlist->elements[0].ptr_value)->rtindex;

        seqScanRight = makeNode(SeqScan);
        seqScanRight->scan.scanrelid = ((RangeTblRef*) parse->jointree->fromlist->elements[1].ptr_value)->rtindex;

        material = makeNode(Material);
        material->plan.lefttree = (Plan*) seqScanRight;

        nestLoop = makeNode(NestLoop);
        nestLoop->join.jointype = JOIN_INNER;
        // OUTER_VAR explained here postgres/src/include/nodes/primnodes.h:172, presumably set here? postgres/src/backend/optimizer/plan/setrefs.c:2255
        nestLoop->join.plan.targetlist = parse->targetList;
        ((Var*) ((TargetEntry*) nestLoop->join.plan.targetlist->elements[0].ptr_value)->expr)->varno = OUTER_VAR;
        nestLoop->join.plan.lefttree = (Plan*) seqScanLeft;
        nestLoop->join.plan.righttree = (Plan*) material;

        result = makeNode(PlannedStmt);
        result->commandType = parse->commandType;
        result->planTree = (Plan*) nestLoop;
        result->rtable = parse->rtable;
        result->permInfos = parse->rteperminfos;
        result->relationOids = list_make2_oid(((RangeTblEntry*) linitial(parse->rtable))->relid, ((RangeTblEntry*) lsecond(parse->rtable))->relid);

        return result;

         // Hardcoded Select
        //PlannedStmt *result;
        //SeqScan *node;

        //// TODO clean up code, ptrs, etc
        //// TODO check for each condition that makes it ok to use my hardcoded single SeqScan plan
        //node = makeNode(SeqScan);
        //node->scan.plan.targetlist = parse->targetList; //
        //node->scan.scanrelid = 1; // is this always 1 for SeqScan? What is it?

        //result = makeNode(PlannedStmt);
        //result->commandType = parse->commandType; //
        //result->planTree = (Plan*) node;
        //result->rtable = parse->rtable; //
        //result->permInfos = parse->rteperminfos; //
        //result->relationOids = list_make1_oid(((RangeTblEntry*) linitial(parse->rtable))->relid); //

        //return result;

         // TODO here's a real planner timer: contrib/pg_stat_statements/pg_stat_statements.c:880
        // Plan time notice for user
        //clock_t begin;
        //clock_t end;
        //double time_spent;
        //PlannedStmt *result;
        //begin = clock();

        //// TODO is there a bug in this check if this gets modified before/during/after init by other extensions?
        //if (original_planner_hook)
        //{
        //      //elog(NOTICE, "4NATEE: using original_planner_hook for query: %s", query_string);
        //      result = original_planner_hook(parse, query_string, cursorOptions, boundParams);
        //}
        //else
        //{
        //      //elog(NOTICE, "5NATEE: using standard_planner for query: %s", query_string);
        //      result = standard_planner(parse, query_string, cursorOptions, boundParams);
        //}

        //end = clock();
        //time_spent = (double) (end - begin) / CLOCKS_PER_SEC;
        //if (strstr(query_string, "display_optimization_time"))
        //      elog(NOTICE, "6NATEE: time spent optimizing query: %f", time_spent);

        //printf("%ld", (long) result->queryId); // TODO get rid of query2, didin't work
        //return result;
}

// Called upon extension load.
void _PG_init(void)
{
    original_planner_hook = planner_hook;
    planner_hook = pgplannerhook_planner;
    printf("0NATEE: overwrote planner with pgplannerhook");
}

void _PG_fini(void)
{
    planner_hook = original_planner_hook;
    printf("1NATEE: set planner back to original");
}
