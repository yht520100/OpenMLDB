/*-------------------------------------------------------------------------
 * Copyright (C) 2020, 4paradigm
 * transform_test.cc
 *
 * Author: chenjing
 * Date: 2020/3/13
 *--------------------------------------------------------------------------
 **/
#include "vm/transform.h"
#include <stack>
#include <string>
#include <utility>
#include "base/status.h"
#include "gtest/gtest.h"
#include "node/node_manager.h"
#include "parser/parser.h"
#include "plan/planner.h"
#include "vm/test_base.h"

namespace fesql {
namespace vm {
class TransformTest : public ::testing::TestWithParam<std::string> {
 public:
    TransformTest() {}
    ~TransformTest() {}
};

void BuildTableDef(::fesql::type::TableDef& table_def) {  // NOLINT
    table_def.set_name("t1");
    table_def.set_catalog("db");
    {
        ::fesql::type::ColumnDef* column = table_def.add_columns();
        column->set_type(::fesql::type::kInt32);
        column->set_name("col1");
    }
    {
        ::fesql::type::ColumnDef* column = table_def.add_columns();
        column->set_type(::fesql::type::kInt16);
        column->set_name("col2");
    }
    {
        ::fesql::type::ColumnDef* column = table_def.add_columns();
        column->set_type(::fesql::type::kFloat);
        column->set_name("col3");
    }

    {
        ::fesql::type::ColumnDef* column = table_def.add_columns();
        column->set_type(::fesql::type::kDouble);
        column->set_name("col4");
    }

    {
        ::fesql::type::ColumnDef* column = table_def.add_columns();
        column->set_type(::fesql::type::kInt64);
        column->set_name("col15");
    }
}

INSTANTIATE_TEST_CASE_P(
    SqlExprPlanner, TransformTest,
    testing::Values(
        "SELECT COL1 FROM t1;", "SELECT COL1 as c1 FROM t1;",
        "SELECT COL1 c1 FROM t1;", "SELECT t1.COL1 FROM t1;",
        "SELECT t1.COL1 as c1 FROM t1;", "SELECT t1.COL1 c1 FROM t1;",
        "SELECT t1.COL1 c1 FROM t1 limit 10;", "SELECT * FROM t1;",
        "SELECT COUNT(*) FROM t1;", "SELECT COUNT(COL1) FROM t1;",
        "SELECT TRIM(COL1) FROM t1;", "SELECT trim(COL1) as trim_col1 FROM t1;",
        "SELECT MIN(COL1) FROM t1;", "SELECT min(COL1) FROM t1;",
        "SELECT MAX(COL1) FROM t1;", "SELECT max(COL1) as max_col1 FROM t1;",
        "SELECT SUM(COL1) FROM t1;", "SELECT sum(COL1) as sum_col1 FROM t1;",
        "SELECT COL1, COL2, `TS`, AVG(COL3) OVER w, SUM(COL3) OVER w FROM t1 \n"
        "WINDOW w AS (PARTITION BY COL2\n"
        "              ORDER BY `TS` ROWS BETWEEN UNBOUNDED PRECEDING AND "
        "UNBOUNDED FOLLOWING);",
        "SELECT COL1, trim(COL2), `TS`, AVG(AMT) OVER w, SUM(AMT) OVER w FROM "
        "t1 \n"
        "WINDOW w AS (PARTITION BY COL2\n"
        "              ORDER BY `TS` ROWS BETWEEN 3 PRECEDING AND 3 "
        "FOLLOWING);",
        "SELECT COL1, SUM(AMT) OVER w as w_amt_sum FROM t1 \n"
        "WINDOW w AS (PARTITION BY COL2\n"
        "              ORDER BY `TS` ROWS BETWEEN 3 PRECEDING AND 3 "
        "FOLLOWING);",
        "SELECT COL1 + COL2 as col12 FROM t1;",
        "SELECT COL1 - COL2 as col12 FROM t1;",
        "SELECT COL1 * COL2 as col12 FROM t1;",
        "SELECT COL1 / COL2 as col12 FROM t1;",
        "SELECT COL1 % COL2 as col12 FROM t1;",
        "SELECT COL1 = COL2 as col12 FROM t1;",
        "SELECT COL1 == COL2 as col12 FROM t1;",
        "SELECT COL1 < COL2 as col12 FROM t1;",
        "SELECT COL1 > COL2 as col12 FROM t1;",
        "SELECT COL1 <= COL2 as col12 FROM t1;",
        "SELECT COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 && COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 and COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 || COL1 != COL2 as col12 FROM t1;",
        "SELECT COL1 >= COL2 or COL1 != COL2 as col12 FROM t1;",
        "SELECT !(COL1 >= COL2 or COL1 != COL2) as col12 FROM t1;",

        "SELECT sum(col1) OVER w1 as w1_col1_sum FROM t1 "
        "WINDOW w1 AS (PARTITION BY col15 ORDER BY `TS` RANGE BETWEEN 3 "
        "PRECEDING AND CURRENT ROW) limit 10;",
        "SELECT COUNT(*) FROM t1;"));

INSTANTIATE_TEST_CASE_P(
    SqlWherePlan, TransformTest,
    testing::Values(
        "SELECT COL1 FROM t1 where COL1+COL2;",
        "SELECT COL1 FROM t1 where COL1;",
        "SELECT COL1 FROM t1 where COL1 > 10 and COL2 = 20 or COL1 =0;",
        "SELECT COL1 FROM t1 where COL1 > 10 and COL2 = 20;",
        "SELECT COL1 FROM t1 where COL1 > 10;"));
INSTANTIATE_TEST_CASE_P(
    SqlLikePlan, TransformTest,
    testing::Values("SELECT COL1 FROM t1 where COL like \"%abc\";",
                    "SELECT COL1 FROM t1 where COL1 like '%123';",
                    "SELECT COL1 FROM t1 where COL not like \"%abc\";",
                    "SELECT COL1 FROM t1 where COL1 not like '%123';",
                    "SELECT COL1 FROM t1 where COL1 not like 10;",
                    "SELECT COL1 FROM t1 where COL1 like 10;"));
INSTANTIATE_TEST_CASE_P(
    SqlInPlan, TransformTest,
    testing::Values(
        "SELECT COL1 FROM t1 where COL in (1, 2, 3, 4, 5);",
        "SELECT COL1 FROM t1 where COL1 in (\"abc\", \"xyz\", \"test\");",
        "SELECT COL1 FROM t1 where COL1 not in (1,2,3,4,5);"));

INSTANTIATE_TEST_CASE_P(
    SqlGroupPlan, TransformTest,
    testing::Values(
        "SELECT distinct sum(COL1) as col1sum, * FROM t1 where col2 > 10 group "
        "by COL1, "
        "COL2 having col1sum > 0 order by COL1+COL2 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2;",
        "SELECT COL1 FROM t1 group by COL1+COL2;",
        "SELECT COL1 FROM t1 group by COL1;",
        "SELECT COL1 FROM t1 group by COL1 > 10 and COL2 = 20 or COL1 =0;",
        "SELECT COL1 FROM t1 group by COL1, COL2;",
        "SELECT COL1 FROM t1 group by COL1;"));

INSTANTIATE_TEST_CASE_P(
    SqlHavingPlan, TransformTest,
    testing::Values(
        "SELECT COL1 FROM t1 having COL1+COL2;",
        "SELECT COL1 FROM t1 having COL1;",
        "SELECT COL1 FROM t1 HAVING COL1 > 10 and COL2 = 20 or COL1 =0;",
        "SELECT COL1 FROM t1 HAVING COL1 > 10 and COL2 = 20;",
        "SELECT COL1 FROM t1 HAVING COL1 > 10;"));

INSTANTIATE_TEST_CASE_P(
    SqlOrderPlan, TransformTest,
    testing::Values("SELECT COL1 FROM t1 order by COL1 + COL2 - COL3;",
                    "SELECT COL1 FROM t1 order by COL1, COL2, COL3;",
                    "SELECT COL1 FROM t1 order by COL1, COL2;",
                    "SELECT COL1 FROM t1 order by COL1;"));

INSTANTIATE_TEST_CASE_P(
    SqlWhereGroupHavingOrderPlan, TransformTest,
    testing::Values(
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 order by COL1+COL2 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 order by COL1 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 limit 10;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0;",
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2 having "
        "sum(COL1) > 0;",
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2 having "
        "col1sum > 0;"));

INSTANTIATE_TEST_CASE_P(
    SqlJoinPlan, TransformTest,
    testing::Values("SELECT * FROM t1 full join t2 on t1.col1 = t2.col2;",
                    "SELECT * FROM t1 left join t2 on t1.col1 = t2.col2;",
                    "SELECT * FROM t1 right join t2 on t1.col1 = t2.col2;",
                    "SELECT * FROM t1 inner join t2 on t1.col1 = t2.col2;"));

INSTANTIATE_TEST_CASE_P(
    SqlUnionPlan, TransformTest,
    testing::Values(
        "SELECT * FROM t1 UNION SELECT * FROM t2;",
        "SELECT * FROM t1 UNION DISTINCT SELECT * FROM t2;",
        "SELECT * FROM t1 UNION ALL SELECT * FROM t2;",
        "SELECT * FROM t1 UNION ALL SELECT * FROM t2 UNION SELECT * FROM t3;",
        "SELECT * FROM t1 left join t2 on t1.col1 = t2.col2 UNION ALL SELECT * "
        "FROM t3 UNION SELECT * FROM t4;",
        "SELECT sum(COL1) as col1sum, * FROM t1 where col2 > 10 group by COL1, "
        "COL2 having col1sum > 0 order by COL1+COL2 limit 10 UNION ALL "
        "SELECT sum(COL1) as col1sum, * FROM t1 group by COL1, COL2 having "
        "sum(COL1) > 0;",
        "SELECT * FROM t1 inner join t2 on t1.col1 = t2.col2 UNION "
        "SELECT * FROM t3 inner join t4 on t3.col1 = t4.col2 UNION "
        "SELECT * FROM t5 inner join t6 on t5.col1 = t6.col2;"));
INSTANTIATE_TEST_CASE_P(
    SqlDistinctPlan, TransformTest,
    testing::Values(
        "SELECT distinct COL1 FROM t1 HAVING COL1 > 10 and COL2 = 20;",
        "SELECT DISTINCT sum(COL1) as col1sum, * FROM t1 group by COL1,COL2;",
        "SELECT DISTINCT sum(col1) OVER w1 as w1_col1_sum FROM t1 "
        "WINDOW w1 AS (PARTITION BY col15 ORDER BY `TS` RANGE BETWEEN 3 "
        "PRECEDING AND CURRENT ROW) limit 10;",
        "SELECT DISTINCT COUNT(*) FROM t1;",
        "SELECT distinct COL1 FROM t1 where COL1+COL2;",
        "SELECT DISTINCT COL1 FROM t1 where COL1 > 10;"));

INSTANTIATE_TEST_CASE_P(
    SqlSubQueryPlan, TransformTest,
    testing::Values(
        "SELECT * FROM t1 WHERE COL1 > (select avg(COL1) from t1) limit 10;",
        "select * from (select * from t1 where col1>0);",
        "select * from \n"
        "    (select * from t1 where col1 = 7) s\n"
        "left join \n"
        "    (select * from t2 where col2 = 2) t\n"
        "on s.col3 = t.col3\n"
        "union\n"
        "select distinct * from \n"
        "    (select distinct * from t1 where col1 = 7) s\n"
        "right join \n"
        "    (select distinct * from t2 where col2 = 2) t\n"
        "on s.col3 = t.col3;",
        "SELECT * FROM t5 inner join t6 on t5.col1 = t6.col2;",
        "select * from \n"
        "    (select * from t1 where col1 = 7) s\n"
        "left join \n"
        "    (select * from t2 where col2 = 2) t\n"
        "on s.col3 = t.col3\n"
        "union\n"
        "select distinct * from \n"
        "    (select  * from t1 where col1 = 7) s\n"
        "right join \n"
        "    (select distinct * from t2 where col2 = 2) t\n"
        "on s.col3 = t.col3;"));

TEST_P(TransformTest, transform_physical_plan) {
    std::string sqlstr = GetParam();
    std::cout << sqlstr << std::endl;

    const fesql::base::Status exp_status(::fesql::common::kOk, "ok");
    std::cout << sqlstr << std::endl;

    fesql::type::TableDef table_def;
    fesql::type::TableDef table_def2;
    fesql::type::TableDef table_def3;
    fesql::type::TableDef table_def4;
    fesql::type::TableDef table_def5;
    fesql::type::TableDef table_def6;
    BuildTableDef(table_def);
    BuildTableDef(table_def2);
    BuildTableDef(table_def3);
    BuildTableDef(table_def4);
    BuildTableDef(table_def5);
    BuildTableDef(table_def6);
    table_def2.set_name("t2");
    table_def3.set_name("t3");
    table_def4.set_name("t4");
    table_def5.set_name("t5");
    table_def6.set_name("t6");
    std::shared_ptr<::fesql::storage::Table> table(
        new ::fesql::storage::Table(1, 1, table_def));
    std::shared_ptr<::fesql::storage::Table> table2(
        new ::fesql::storage::Table(1, 1, table_def2));
    std::shared_ptr<::fesql::storage::Table> table3(
        new ::fesql::storage::Table(1, 1, table_def3));
    std::shared_ptr<::fesql::storage::Table> table4(
        new ::fesql::storage::Table(1, 1, table_def4));
    std::shared_ptr<::fesql::storage::Table> table5(
        new ::fesql::storage::Table(1, 1, table_def5));
    std::shared_ptr<::fesql::storage::Table> table6(
        new ::fesql::storage::Table(1, 1, table_def6));
    auto catalog = BuildCommonCatalog(table_def, table);
    AddTable(catalog, table_def2, table2);
    AddTable(catalog, table_def3, table3);
    AddTable(catalog, table_def4, table4);
    AddTable(catalog, table_def5, table5);
    AddTable(catalog, table_def6, table6);

    ::fesql::node::NodeManager manager;
    ::fesql::node::PlanNodeList plan_trees;
    ::fesql::base::Status base_status;
    {
        ::fesql::plan::SimplePlanner planner(&manager);
        ::fesql::parser::FeSQLParser parser;
        ::fesql::node::NodePointVector parser_trees;
        parser.parse(sqlstr, parser_trees, &manager, base_status);
        ASSERT_EQ(0, base_status.code);
        if (planner.CreatePlanTree(parser_trees, plan_trees, base_status) ==
            0) {
            std::cout << *(plan_trees[0]);
        } else {
            std::cout << base_status.msg;
        }

        ASSERT_EQ(0, base_status.code);
        std::cout.flush();
    }

    Transform transform("db", catalog);
    PhysicalOpNode* physical_plan = nullptr;
    ASSERT_TRUE(transform.TransformPhysicalPlan(
        dynamic_cast<node::PlanNode*>(plan_trees[0]), &physical_plan,
        base_status));
}

}  // namespace vm
}  // namespace fesql
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
