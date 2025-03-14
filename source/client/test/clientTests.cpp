/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <iostream>
#include "clientInt.h"
#include "taoserror.h"
#include "tglobal.h"
#include "thash.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"

#include "executor.h"
#include "taos.h"

namespace {
void showDB(TAOS* pConn) {
  TAOS_RES* pRes = taos_query(pConn, "show databases");
  TAOS_ROW  pRow = NULL;

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%s\n", str);
  }
}

void printResult(TAOS_RES* pRes) {
  TAOS_ROW    pRow = NULL;
  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  int32_t n = 0;
  char    str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
//    int32_t* length = taos_fetch_lengths(pRes);
//    for(int32_t i = 0; i < numOfFields; ++i) {
//      printf("(%d):%d " , i, length[i]);
//    }
//    printf("\n");
//
//    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
//    printf("%s\n", str);
//    memset(str, 0, sizeof(str));
  }
}

void fetchCallback(void* param, void* res, int32_t numOfRow) {
#if 0
  printf("numOfRow = %d \n", numOfRow);
  int         numFields = taos_num_fields(res);
  TAOS_FIELD *fields = taos_fetch_fields(res);
  TAOS       *_taos = (TAOS *)param;
  if (numOfRow > 0) {
    for (int i = 0; i < numOfRow; ++i) {
      TAOS_ROW row = taos_fetch_row(res);

      char temp[256] = {0};
      taos_print_row(temp, row, fields, numFields);
      printf("%s\n", temp);
    }
    taos_fetch_rows_a(res, fetchCallback, _taos);
  } else {
    printf("no more data, close the connection.\n");
//    taos_free_result(res);
//    taos_close(_taos);
//    taos_cleanup();
  }
#endif
  if (numOfRow == 0) {
    printf("completed\n");
    return;
  }

  taos_fetch_raw_block_a(res, fetchCallback, param);
}

void queryCallback(void* param, void* res, int32_t code) {
  if (code != TSDB_CODE_SUCCESS) {
    printf("failed to execute, reason:%s\n", taos_errstr(res));
  }
  printf("start to fetch data\n");
  taos_fetch_raw_block_a(res, fetchCallback, param);
}

void createNewTable(TAOS* pConn, int32_t index) {
  char str[1024] = {0};
  sprintf(str, "create table tu%d using st2 tags(%d)", index, index);

  TAOS_RES* pRes = taos_query(pConn, str);
  if (taos_errno(pRes) != 0) {
    printf("failed to create table tu, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  for(int32_t i = 0; i < 2000; i += 20) {
    char sql[1024] = {0};
    sprintf(sql,
            "insert into tu%d values(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)"
            "(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)"
            "(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)"
            "(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)(now+%da, %d)",
            index, i, i, i + 1, i + 1, i + 2, i + 2, i + 3, i + 3, i + 4, i + 4, i + 5, i + 5, i + 6, i + 6, i + 7,
            i + 7, i + 8, i + 8, i + 9, i + 9, i + 10, i + 10, i + 11, i + 11, i + 12, i + 12, i + 13, i + 13, i + 14,
            i + 14, i + 15, i + 15, i + 16, i + 16, i + 17, i + 17, i + 18, i + 18, i + 19, i + 19);
    TAOS_RES* p = taos_query(pConn, sql);
    if (taos_errno(p) != 0) {
      printf("failed to insert data, reason:%s\n", taos_errstr(p));
    }

    taos_free_result(p);
  }
}

void *queryThread(void *arg) {
  TAOS* pConn = taos_connect("192.168.0.209", "root", "taosdata", NULL, 0);
  if (pConn == NULL) {
    printf("failed to connect to db, reason:%s", taos_errstr(pConn));
    return NULL;
  }

  int64_t el = 0;

  for (int32_t i = 0; i < 5000000; ++i) {
    int64_t st = taosGetTimestampUs();
    TAOS_RES* pRes = taos_query(pConn,
                      "SELECT _wstart as ts,max(usage_user) FROM benchmarkcpu.host_49 WHERE  ts >= 1451618560000 AND ts < 1451622160000 INTERVAL(1m) ;");
    if (taos_errno(pRes) != 0) {
      printf("failed, reason:%s\n", taos_errstr(pRes));
    } else {
      printResult(pRes);
    }

    taos_free_result(pRes);
    el += (taosGetTimestampUs() - st);
    if (i % 1000 == 0 && i != 0) {
      printf("total:%d, avg time:%.2fms\n", i, el/(double)(i*1000));
    }
  }

  taos_close(pConn);
  return NULL;
}

static int32_t numOfThreads = 1;

void tmq_commit_cb_print(tmq_t *pTmq, int32_t code, void *param) {
  printf("success, code:%d\n", code);
}

}  // namespace

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  if (argc > 1) {
    numOfThreads = atoi(argv[1]);
  }

  numOfThreads = TMAX(numOfThreads, 1);
  printf("the runing threads is:%d", numOfThreads);

  return RUN_ALL_TESTS();
}

TEST(clientCase, driverInit_Test) {
  // taosInitGlobalCfg();
  //  taos_init();
}

TEST(clientCase, connect_Test) {
  taos_options(TSDB_OPTION_CONFIGDIR, "~/first/cfg");

  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  if (pConn == NULL) {
    printf("failed to connect to server, reason:%s\n", taos_errstr(NULL));
  }
  taos_close(pConn);
}

TEST(clientCase, create_user_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "create user abc pass 'abc'");
  if (taos_errno(pRes) != TSDB_CODE_SUCCESS) {
    printf("failed to create user, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, create_account_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "create account aabc pass 'abc'");
  if (taos_errno(pRes) != TSDB_CODE_SUCCESS) {
    printf("failed to create user, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, drop_account_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "drop account aabc");
  if (taos_errno(pRes) != TSDB_CODE_SUCCESS) {
    printf("failed to create user, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, show_user_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "show users");
  TAOS_ROW  pRow = NULL;

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%s\n", str);
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, drop_user_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "drop user abc");
  if (taos_errno(pRes) != TSDB_CODE_SUCCESS) {
    printf("failed to create user, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, show_db_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "show databases");
  TAOS_ROW  pRow = NULL;

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%s\n", str);
  }

  taos_close(pConn);
}

TEST(clientCase, create_db_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "create database abc1 vgroups 2");
  if (taos_errno(pRes) != 0) {
    printf("error in create db, reason:%s\n", taos_errstr(pRes));
  }

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  ASSERT_TRUE(pFields == NULL);

  int32_t numOfFields = taos_num_fields(pRes);
  ASSERT_EQ(numOfFields, 0);

  taos_free_result(pRes);

  pRes = taos_query(pConn, "create database abc1 vgroups 4");
  if (taos_errno(pRes) != 0) {
    printf("error in create db, reason:%s\n", taos_errstr(pRes));
  }
  taos_close(pConn);
}

TEST(clientCase, create_dnode_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "create dnode abc1 port 7000");
  if (taos_errno(pRes) != 0) {
    printf("error in create dnode, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "create dnode 1.1.1.1 port 9000");
  if (taos_errno(pRes) != 0) {
    printf("failed to create dnode, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  taos_close(pConn);
}

TEST(clientCase, drop_dnode_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "drop dnode 3");
  if (taos_errno(pRes) != 0) {
    printf("error in drop dnode, reason:%s\n", taos_errstr(pRes));
  }

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  ASSERT_TRUE(pFields == NULL);

  int32_t numOfFields = taos_num_fields(pRes);
  ASSERT_EQ(numOfFields, 0);

  pRes = taos_query(pConn, "drop dnode 4");
  if (taos_errno(pRes) != 0) {
    printf("error in drop dnode, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, use_db_test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "use abc1");
  if (taos_errno(pRes) != 0) {
    printf("error in use db, reason:%s\n", taos_errstr(pRes));
  }

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  ASSERT_TRUE(pFields == NULL);

  int32_t numOfFields = taos_num_fields(pRes);
  ASSERT_EQ(numOfFields, 0);

  taos_close(pConn);
}

// TEST(clientCase, drop_db_test) {
//  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
//  assert(pConn != NULL);
//
//  showDB(pConn);
//
//  TAOS_RES* pRes = taos_query(pConn, "drop database abc1");
//  if (taos_errno(pRes) != 0) {
//    printf("failed to drop db, reason:%s\n", taos_errstr(pRes));
//  }
//  taos_free_result(pRes);
//
//  showDB(pConn);
//
//  pRes = taos_query(pConn, "create database abc1");
//  if (taos_errno(pRes) != 0) {
//    printf("create to drop db, reason:%s\n", taos_errstr(pRes));
//  }
//  taos_free_result(pRes);
//  taos_close(pConn);
//}

TEST(clientCase, create_stable_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "create database if not exists abc1 vgroups 2");
  if (taos_errno(pRes) != 0) {
    printf("error in create db, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "use abc1");

  pRes = taos_query(pConn, "create table if not exists abc1.st1(ts timestamp, k int) tags(a int)");
  if (taos_errno(pRes) != 0) {
    printf("error in create stable, reason:%s\n", taos_errstr(pRes));
  }

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  ASSERT_TRUE(pFields == NULL);

  int32_t numOfFields = taos_num_fields(pRes);
  ASSERT_EQ(numOfFields, 0);
  taos_free_result(pRes);

//  pRes = taos_query(pConn, "create stable if not exists abc1.`123_$^)` (ts timestamp, `abc` int) tags(a int)");
//  if (taos_errno(pRes) != 0) {
//    printf("failed to create super table 123_$^), reason:%s\n", taos_errstr(pRes));
//  }
//
//  pRes = taos_query(pConn, "use abc1");
//  taos_free_result(pRes);
//  pRes = taos_query(pConn, "drop stable `123_$^)`");
//  if (taos_errno(pRes) != 0) {
//    printf("failed to drop super table 123_$^), reason:%s\n", taos_errstr(pRes));
//  }

  taos_close(pConn);
}

TEST(clientCase, create_table_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "use abc1");
  taos_free_result(pRes);

  pRes = taos_query(pConn, "create table if not exists tm0(ts timestamp, k int)");
  ASSERT_EQ(taos_errno(pRes), 0);

  taos_free_result(pRes);

  pRes = taos_query(pConn, "create table if not exists tm0(ts timestamp, k blob)");
  ASSERT_NE(taos_errno(pRes), 0);

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, create_ctable_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "use abc1");
  if (taos_errno(pRes) != 0) {
    printf("failed to use db, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "create stable if not exists st1 (ts timestamp, k int ) tags(a int)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create stable, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "create table tu using st1 tags('2021-10-10 1:1:1');");
  if (taos_errno(pRes) != 0) {
    printf("failed to create child table tm0, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, show_stable_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != nullptr);

  TAOS_RES* pRes = taos_query(pConn, "show abc1.stables");
  if (taos_errno(pRes) != 0) {
    printf("failed to show stables, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }

  TAOS_ROW    pRow = NULL;
  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%s\n", str);
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, show_vgroup_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "use abc1");
  if (taos_errno(pRes) != 0) {
    printf("failed to use db, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "show vgroups");
  if (taos_errno(pRes) != 0) {
    printf("failed to show vgroups, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }

  TAOS_ROW pRow = NULL;

  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%s\n", str);
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, create_multiple_tables) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  TAOS_RES* pRes = taos_query(pConn, "create database if not exists abc1");
  if (taos_errno(pRes) != 0) {
    printf("failed to create db, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    taos_close(pConn);
    return;
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "use abc1");
  if (taos_errno(pRes) != 0) {
    printf("failed to use db, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    taos_close(pConn);
    return;
  }

  taos_free_result(pRes);

  pRes = taos_query(pConn, "create stable if not exists st1 (ts timestamp, k int) tags(a int)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create stable tables, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);

  pRes = taos_query(pConn, "create table if not exists t_2 using st1 tags(1)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create multiple tables, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }

  taos_free_result(pRes);
  pRes = taos_query(pConn, "create table if not exists t_3 using st1 tags(2)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create multiple tables, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }

  TAOS_ROW    pRow = NULL;
  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%s\n", str);
  }

  taos_free_result(pRes);

  for (int32_t i = 0; i < 500; i += 2) {
    char sql[512] = {0};
    snprintf(sql, tListLen(sql),
             "create table t_x_%d using st1 tags(2) t_x_%d using st1 tags(5)", i, i + 1);
    TAOS_RES* pres = taos_query(pConn, sql);
    if (taos_errno(pres) != 0) {
      printf("failed to create table %d\n, reason:%s", i, taos_errstr(pres));
    }
    taos_free_result(pres);
  }

  taos_close(pConn);
}

TEST(clientCase, show_table_Test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  assert(pConn != NULL);

  TAOS_RES* pRes = taos_query(pConn, "show tables");
  if (taos_errno(pRes) != 0) {
    printf("failed to show tables, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  taos_query(pConn, "use abc1");

  pRes = taos_query(pConn, "show tables");
  if (taos_errno(pRes) != 0) {
    printf("failed to show tables, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
  }

  TAOS_ROW    pRow = NULL;
  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  int32_t count = 0;
  char str[512] = {0};

  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%d: %s\n", ++count, str);
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

//TEST(clientCase, drop_stable_Test) {
//  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
//  assert(pConn != nullptr);
//
//  TAOS_RES* pRes = taos_query(pConn, "create database if not exists abc1");
//  if (taos_errno(pRes) != 0) {
//    printf("error in creating db, reason:%s\n", taos_errstr(pRes));
//  }
//  taos_free_result(pRes);
//
//  pRes = taos_query(pConn, "use abc1");
//  if (taos_errno(pRes) != 0) {
//    printf("error in using db, reason:%s\n", taos_errstr(pRes));
//  }
//  taos_free_result(pRes);
//
//  pRes = taos_query(pConn, "drop stable st1");
//  if (taos_errno(pRes) != 0) {
//    printf("failed to drop stable, reason:%s\n", taos_errstr(pRes));
//  }
//
//  taos_free_result(pRes);
//  taos_close(pConn);
//}

TEST(clientCase, generated_request_id_test) {
  SHashObj* phash = taosHashInit(10000, taosGetDefaultHashFunction(TSDB_DATA_TYPE_BIGINT), false, HASH_ENTRY_LOCK);

  for (int32_t i = 0; i < 50000; ++i) {
    uint64_t v = generateRequestId();
    void*    result = taosHashGet(phash, &v, sizeof(v));
    if (result != nullptr) {
//      printf("0x%llx, index:%d\n", v, i);
    }
    assert(result == nullptr);
    taosHashPut(phash, &v, sizeof(v), NULL, 0);
  }

  taosHashCleanup(phash);
}

TEST(clientCase, insert_test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  TAOS_RES* pRes = taos_query(pConn, "use abc1");
  taos_free_result(pRes);

  pRes = taos_query(pConn, "insert into t_2 values(now, 1)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create into table t_2, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, projection_query_tables) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  //  TAOS_RES* pRes = taos_query(pConn, "create database if not exists abc1 vgroups 1");
  //  if (taos_errno(pRes) != 0) {
  //    printf("error in create db, reason:%s\n", taos_errstr(pRes));
  //  }
  //  taos_free_result(pRes);

  TAOS_RES* pRes = taos_query(pConn, "use abc2");
  taos_free_result(pRes);

  pRes = taos_query(pConn, "create stable st1 (ts timestamp, k int) tags(a int)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create table tu, reason:%s\n", taos_errstr(pRes));
  }

  taos_free_result(pRes);

  pRes = taos_query(pConn, "create stable st2 (ts timestamp, k int) tags(a int)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create table tu, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "create table tu using st1 tags(1)");
  if (taos_errno(pRes) != 0) {
    printf("failed to create table tu, reason:%s\n", taos_errstr(pRes));
  }
  taos_free_result(pRes);

  for (int32_t i = 0; i < 2; ++i) {
    printf("create table :%d\n", i);
    createNewTable(pConn, i);
  }
  //
  //  pRes = taos_query(pConn, "select * from tu");
  //  if (taos_errno(pRes) != 0) {
  //    printf("failed to select from table, reason:%s\n", taos_errstr(pRes));
  //    taos_free_result(pRes);
  //    ASSERT_TRUE(false);
  //  }
  //
  //  TAOS_ROW    pRow = NULL;
  //  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  //  int32_t     numOfFields = taos_num_fields(pRes);
  //
  //  char str[512] = {0};
  //  while ((pRow = taos_fetch_row(pRes)) != NULL) {
  //    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
  //    printf("%s\n", str);
  //  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, tsbs_perf_test) {
  TdThread qid[20] = {0};

  for(int32_t i = 0; i < numOfThreads; ++i) {
    taosThreadCreate(&qid[i], NULL, queryThread, NULL);
  }
  getchar();
}

TEST(clientCase, projection_query_stables) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  TAOS_RES* pRes = taos_query(pConn, "use test");
  taos_free_result(pRes);

  pRes = taos_query(pConn, "select * from meters limit 50000000");
  if (taos_errno(pRes) != 0) {
    printf("failed to select from table, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }

  TAOS_ROW    pRow = NULL;
  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
//    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
//    printf("%s\n", str);
  }

  taos_free_result(pRes);
  taos_close(pConn);
}

TEST(clientCase, agg_query_tables) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  TAOS_RES* pRes = taos_query(pConn, "use abc1");
  if (taos_errno(pRes) != 0) {
    printf("failed to use db, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "show table distributed tup");
  if (taos_errno(pRes) != 0) {
    printf("failed to select from table, reason:%s\n", taos_errstr(pRes));
    taos_free_result(pRes);
    ASSERT_TRUE(false);
  }

  printResult(pRes);
  taos_free_result(pRes);
  taos_close(pConn);
}

/*
--- copy the following script in the shell to setup the environment ---

create database test;
use test;
create table m1(ts timestamp, k int) tags(a int);
create table tm0 using m1 tags(1);
create table tm1 using m1 tags(2);
insert into tm0 values('2021-1-1 1:1:1.120', 1) ('2021-1-1 1:1:2.9', 2) tm1 values('2021-1-1 1:1:1.120', 11) ('2021-1-1 1:1:2.99', 22);

 */
TEST(clientCase, async_api_test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  taos_query(pConn, "use abc1");

  TAOS_RES* pRes = taos_query(pConn, "insert into tu(ts) values('2022-02-27 12:12:61')");
  if (taos_errno(pRes) != 0) {
    printf("failed, reason:%s\n", taos_errstr(pRes));
  }

  int32_t n = 0;
  TAOS_ROW    pRow = NULL;
  TAOS_FIELD* pFields = taos_fetch_fields(pRes);
  int32_t     numOfFields = taos_num_fields(pRes);

  char str[512] = {0};
  while ((pRow = taos_fetch_row(pRes)) != NULL) {
    int32_t* length = taos_fetch_lengths(pRes);
    for(int32_t i = 0; i < numOfFields; ++i) {
      printf("(%d):%d " , i, length[i]);
    }
    printf("\n");

    int32_t code = taos_print_row(str, pRow, pFields, numOfFields);
    printf("%s\n", str);
    memset(str, 0, sizeof(str));
  }

  taos_query_a(pConn, "select count(*) from tu", queryCallback, pConn);
  getchar();
  taos_close(pConn);
}

TEST(clientCase, update_test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  TAOS_RES* pRes = taos_query(pConn, "select cast(0 as timestamp)-1y");
  if (taos_errno(pRes) != TSDB_CODE_SUCCESS) {
    printf("failed to create database, code:%s", taos_errstr(pRes));
    taos_free_result(pRes);
    return;
  }

  taos_free_result(pRes);

  pRes = taos_query(pConn, "use abc1");
  if (taos_errno(pRes) != TSDB_CODE_SUCCESS) {
    printf("failed to use db, code:%s", taos_errstr(pRes));
    taos_free_result(pRes);
    return;
  }
  taos_free_result(pRes);

  pRes = taos_query(pConn, "create table tup (ts timestamp, k int);");
  if (taos_errno(pRes) != 0) {
    printf("failed to create table, reason:%s", taos_errstr(pRes));
  }

  taos_free_result(pRes);

  char s[256]  = {0};
  for(int32_t i = 0; i < 17000; ++i) {
    sprintf(s, "insert into tup values(now+%da, %d)", i, i);
    pRes = taos_query(pConn, s);
    taos_free_result(pRes);
  }
}

TEST(clientCase, subscription_test) {
  TAOS* pConn = taos_connect("localhost", "root", "taosdata", NULL, 0);
  ASSERT_NE(pConn, nullptr);

  //  TAOS_RES* pRes = taos_query(pConn, "create topic topic_t1 as select * from t1");
  //  if (taos_errno(pRes) != TSDB_CODE_SUCCESS) {
  //    printf("failed to create topic, code:%s", taos_errstr(pRes));
  //    taos_free_result(pRes);
  //    return;
  //  }

  tmq_conf_t* conf = tmq_conf_new();
  tmq_conf_set(conf, "enable.auto.commit", "true");
  tmq_conf_set(conf, "auto.commit.interval.ms", "1000");
  tmq_conf_set(conf, "group.id", "cgrpName");
  tmq_conf_set(conf, "td.connect.user", "root");
  tmq_conf_set(conf, "td.connect.pass", "taosdata");
  tmq_conf_set(conf, "auto.offset.reset", "earliest");
  tmq_conf_set(conf, "experimental.snapshot.enable", "true");
  tmq_conf_set(conf, "msg.with.table.name", "true");
  tmq_conf_set_auto_commit_cb(conf, tmq_commit_cb_print, NULL);

  tmq_t* tmq = tmq_consumer_new(conf, NULL, 0);
  tmq_conf_destroy(conf);

  // 创建订阅 topics 列表
  tmq_list_t* topicList = tmq_list_new();
  tmq_list_append(topicList, "topic_t1");

  // 启动订阅
  tmq_subscribe(tmq, topicList);
  tmq_list_destroy(topicList);

  TAOS_FIELD* fields = NULL;
  int32_t     numOfFields = 0;
  int32_t     precision = 0;
  int32_t     totalRows = 0;
  int32_t     msgCnt = 0;
  int32_t     timeout = 5000;

  while (1) {
    TAOS_RES* pRes = tmq_consumer_poll(tmq, timeout);
    if (pRes) {
      char    buf[1024];
      int32_t rows = 0;

      const char* topicName = tmq_get_topic_name(pRes);
      const char* dbName = tmq_get_db_name(pRes);
      int32_t     vgroupId = tmq_get_vgroup_id(pRes);

      printf("topic: %s\n", topicName);
      printf("db: %s\n", dbName);
      printf("vgroup id: %d\n", vgroupId);

      while (1) {
        TAOS_ROW row = taos_fetch_row(pRes);
        if (row == NULL) break;

        fields = taos_fetch_fields(pRes);
        numOfFields = taos_field_count(pRes);
        precision = taos_result_precision(pRes);
        rows++;
        taos_print_row(buf, row, fields, numOfFields);
        printf("precision: %d, row content: %s\n", precision, buf);
      }
    }
//      return rows;
  }

  fprintf(stderr, "%d msg consumed, include %d rows\n", msgCnt, totalRows);
}

#pragma GCC diagnostic pop
