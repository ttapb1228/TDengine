import datetime

from dataclasses import dataclass
from typing import List
from util.log import *
from util.sql import *
from util.cases import *
from util.dnodes import *

PRIMARY_COL = "ts"

INT_COL     = "c_int"
BINT_COL    = "c_bint"
SINT_COL    = "c_sint"
TINT_COL    = "c_tint"
FLOAT_COL   = "c_float"
DOUBLE_COL  = "c_double"
BOOL_COL    = "c_bool"
TINT_UN_COL = "c_tint_un"
SINT_UN_COL = "c_sint_un"
BINT_UN_COL = "c_bint_un"
INT_UN_COL  = "c_int_un"

BINARY_COL  = "c8"
NCHAR_COL   = "c9"
TS_COL      = "c10"

NUM_COL     = [ INT_COL, BINT_COL, SINT_COL, TINT_COL, FLOAT_COL, DOUBLE_COL, ]
CHAR_COL    = [ BINARY_COL, NCHAR_COL, ]
BOOLEAN_COL = [ BOOL_COL, ]
TS_TYPE_COL = [ TS_COL, ]

@dataclass
class DataSet:
    ts_data         : List[int]     = None
    int_data        : List[int]     = None
    bint_data       : List[int]     = None
    sint_data       : List[int]     = None
    tint_data       : List[int]     = None
    int_un_data     : List[int]     = None
    bint_un_data    : List[int]     = None
    sint_un_data    : List[int]     = None
    tint_un_data    : List[int]     = None
    float_data      : List[float]   = None
    double_data     : List[float]   = None
    bool_data       : List[int]     = None
    binary_data     : List[str]     = None
    nchar_data      : List[str]     = None


class TDTestCase:

    def init(self, conn, logSql):
        tdLog.debug(f"start to excute {__file__}")
        tdSql.init(conn.cursor(), True)

    @property
    def create_databases_sql_err(self):
        return [
            "create database db1 retentions 0s:1d",
            "create database db3 retentions 1s:0d",
            "create database db1 retentions 1s:1y",
            "create database db1 retentions 1s:1n",
            "create database db2 retentions 1w:1d ;",
            "create database db5 retentions 1s:1d,3s:3d,2s:2d",
            "create database db1 retentions 1s:1n,2s:2d,3s:3d,4s:4d",
        ]

    @property
    def create_databases_sql_current(self):
        return [
            "create database db1 retentions 1s:1d",
            "create database db2 retentions 1s:1d,2m:2d,3h:3d",
        ]

    @property
    def alter_database_sql(self):
        return [
            "alter database db1 retentions 99h:99d",
            "alter database db2 retentions 97h:97d,98h:98d,99h:99d,",
        ]

    @property
    def create_stable_sql_err(self):
        return [
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(ceil) watermark 1s maxdelay 1m",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(count) watermark  1min",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) maxdelay -1s",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) watermark -1m",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) watermark 1m ",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) maxdelay 1m ",
            f"create stable stb2 ({PRIMARY_COL} timestamp, {INT_COL} int, {BINARY_COL} binary(16)) tags (tag1 int) rollup(avg) watermark 1s",
            f"create stable stb2 ({PRIMARY_COL} timestamp, {INT_COL} int, {BINARY_COL} nchar(16)) tags (tag1 int) rollup(avg) maxdelay 1m",
            # f"create table ntb_1 ({PRIMARY_COL} timestamp, {INT_COL} int, {BINARY_COL} nchar(16)) rollup(avg) watermark 1s maxdelay 1s",
            # f"create stable stb2 ({PRIMARY_COL} timestamp, {INT_COL} int, {BINARY_COL} nchar(16)) tags (tag1 int) " ,
            # f"create stable stb2 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) " ,
            # f"create stable stb2 ({PRIMARY_COL} timestamp, {INT_COL} int) " ,
            # f"create stable stb2 ({PRIMARY_COL} timestamp, {INT_COL} int, {BINARY_COL} nchar(16)) " ,

            # watermark, maxdelay: [0, 900000], [ms, s, m, ?]
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) maxdelay 1u",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) watermark 1b",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) watermark 900001ms",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) maxdelay 16m",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) maxdelay 901s",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) maxdelay 1h",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) maxdelay 0.2h",
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) watermark 0.002d",

        ]

    @property
    def create_stable_sql_current(self):
        return [
            f"create stable stb1 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(avg)",
            f"create stable stb2 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(min) watermark 5s maxdelay 1m",
            f"create stable stb3 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(max) watermark 5s maxdelay 1m",
            f"create stable stb4 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(sum) watermark 5s maxdelay 1m",
            # f"create stable stb5 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(last) watermark 5s maxdelay 1m",
            # f"create stable stb6 ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) rollup(first) watermark 5s maxdelay 1m",
        ]

    def test_create_stb(self):
        tdSql.execute("use db2")
        for err_sql in self.create_stable_sql_err:
            tdSql.error(err_sql)
        for cur_sql in self.create_stable_sql_current:
            tdSql.execute(cur_sql)
        tdSql.query("show stables")
        # assert "rollup" in tdSql.description
        tdSql.checkRows(len(self.create_stable_sql_current))

        # tdSql.execute("use db")  # because db is a noraml database, not a rollup database, should not be able to create a rollup database
        # tdSql.error(f"create stable nor_db_rollup_stb ({PRIMARY_COL} timestamp, {INT_COL} int) tags (tag1 int) file_factor 5.0")


    def test_create_databases(self):
        for err_sql in self.create_databases_sql_err:
            tdSql.error(err_sql)
        for cur_sql in self.create_databases_sql_current:
            tdSql.execute(cur_sql)
            # tdSql.query("show databases")
        for alter_sql in self.alter_database_sql:
            tdSql.error(alter_sql)

    def all_test(self):
        self.test_create_databases()
        self.test_create_stb()

    def __create_tb(self):
        tdLog.printNoPrefix("==========step1:create table")
        create_stb_sql  =  f'''create table stb1(
                ts timestamp, {INT_COL} int, {BINT_COL} bigint, {SINT_COL} smallint, {TINT_COL} tinyint,
                {FLOAT_COL} float, {DOUBLE_COL} double, {BOOL_COL} bool,
                {BINARY_COL} binary(16), {NCHAR_COL} nchar(32), {TS_COL} timestamp,
                {TINT_UN_COL} tinyint unsigned, {SINT_UN_COL} smallint unsigned,
                {INT_UN_COL} int unsigned, {BINT_UN_COL} bigint unsigned
            ) tags (t1 int)
            '''
        create_ntb_sql = f'''create table t1(
                ts timestamp, {INT_COL} int, {BINT_COL} bigint, {SINT_COL} smallint, {TINT_COL} tinyint,
                {FLOAT_COL} float, {DOUBLE_COL} double, {BOOL_COL} bool,
                {BINARY_COL} binary(16), {NCHAR_COL} nchar(32), {TS_COL} timestamp,
                {TINT_UN_COL} tinyint unsigned, {SINT_UN_COL} smallint unsigned,
                {INT_UN_COL} int unsigned, {BINT_UN_COL} bigint unsigned
            )
            '''
        tdSql.execute(create_stb_sql)
        tdSql.execute(create_ntb_sql)

        for i in range(4):
            tdSql.execute(f'create table ct{i+1} using stb1 tags ( {i+1} )')

    def __data_set(self, rows):
        now_time = int(datetime.datetime.timestamp(datetime.datetime.now()) * 1000)
        data_set = DataSet()
        # neg_data_set = DataSet()
        data_set.ts_data = []
        data_set.int_data = []
        data_set.bint_data = []
        data_set.sint_data = []
        data_set.tint_data = []
        data_set.int_un_data = []
        data_set.bint_un_data = []
        data_set.sint_un_data = []
        data_set.tint_un_data = []
        data_set.float_data = []
        data_set.double_data = []
        data_set.bool_data = []
        data_set.binary_data = []
        data_set.nchar_data = []

        for i in range(rows):
            data_set.ts_data.append(now_time + 1 * (rows - i))
            data_set.int_data.append(rows - i)
            data_set.bint_data.append(11111 * (rows - i))
            data_set.sint_data.append(111 * (rows - i) % 32767)
            data_set.tint_data.append(11 * (rows - i) % 127)
            data_set.int_un_data.append(rows - i)
            data_set.bint_un_data.append(11111 * (rows - i))
            data_set.sint_un_data.append(111 * (rows - i) % 32767)
            data_set.tint_un_data.append(11 * (rows - i) % 127)
            data_set.float_data.append(1.11 * (rows - i))
            data_set.double_data.append(1100.0011 * (rows - i))
            data_set.bool_data.append((rows - i) % 2)
            data_set.binary_data.append(f'binary{(rows - i)}')
            data_set.nchar_data.append(f'nchar_测试_{(rows - i)}')

            # neg_data_set.ts_data.append(-1 * i)
            # neg_data_set.int_data.append(-i)
            # neg_data_set.bint_data.append(-11111 * i)
            # neg_data_set.sint_data.append(-111 * i % 32767)
            # neg_data_set.tint_data.append(-11 * i % 127)
            # neg_data_set.int_un_data.append(-i)
            # neg_data_set.bint_un_data.append(-11111 * i)
            # neg_data_set.sint_un_data.append(-111 * i % 32767)
            # neg_data_set.tint_un_data.append(-11 * i % 127)
            # neg_data_set.float_data.append(-1.11 * i)
            # neg_data_set.double_data.append(-1100.0011 * i)
            # neg_data_set.binary_data.append(f'binary{i}')
            # neg_data_set.nchar_data.append(f'nchar_测试_{i}')

        return data_set

    def __insert_data_0(self):
        data = self.__data_set(rows=self.rows)

        now_time = int(datetime.datetime.timestamp(datetime.datetime.now()) * 1000)
        null_data = '''null, null, null, null, null, null, null, null, null, null, null, null, null, null'''
        zero_data = "0, 0, 0, 0, 0, 0, 0, 'binary_0', 'nchar_0', 0, 0, 0, 0, 0"

        for i in range(self.rows):
            row_data = f'''
                {data.int_data[i]}, {data.bint_data[i]}, {data.sint_data[i]}, {data.tint_data[i]}, {data.float_data[i]}, {data.double_data[i]},
                {data.bool_data[i]}, '{data.binary_data[i]}', '{data.nchar_data[i]}', {data.ts_data[i]}, {data.tint_un_data[i]},
                {data.sint_un_data[i]}, {data.int_un_data[i]}, {data.bint_un_data[i]}
            '''
            neg_row_data = f'''
                {-1 * data.int_data[i]}, {-1 * data.bint_data[i]}, {-1 * data.sint_data[i]}, {-1 * data.tint_data[i]}, {-1 * data.float_data[i]}, {-1 * data.double_data[i]},
                {data.bool_data[i]}, '{data.binary_data[i]}', '{data.nchar_data[i]}', {data.ts_data[i]}, {1 * data.tint_un_data[i]},
                {1 * data.sint_un_data[i]}, {1 * data.int_un_data[i]}, {1 * data.bint_un_data[i]}
            '''

            tdSql.execute( f"insert into ct1 values ( {now_time - i * 10000}, {row_data} )" )
            tdSql.execute( f"insert into ct4 values ( {now_time - i * 8000}, {row_data} )" )
            tdSql.execute( f"insert into t1 values ( {now_time - i * 8000}, {row_data} )" )
            tdSql.execute( f"insert into ct2 values ( {now_time - i * 12000}, {neg_row_data} )" )

        tdSql.execute( f"insert into ct2 values ( {now_time + 6000}, {null_data} )" )
        tdSql.execute( f"insert into ct2 values ( {now_time - (self.rows + 1) * 6000}, {null_data} )" )
        tdSql.execute( f"insert into ct2 values ( {now_time - self.rows * 2900}, {null_data} )" )

        tdSql.execute( f"insert into ct4 values ( {now_time + 8000}, {null_data} )" )
        tdSql.execute( f"insert into ct4 values ( {now_time - (self.rows + 1) * 8000}, {null_data} )" )
        tdSql.execute( f"insert into ct4 values ( {now_time - self.rows * 3900}, {null_data} )" )

        tdSql.execute( f"insert into t1 values ( {now_time + 12000}, {null_data} )" )
        tdSql.execute( f"insert into t1 values ( {now_time - (self.rows + 1) * 12000}, {null_data} )" )
        tdSql.execute( f"insert into t1 values ( {now_time - self.rows * 5900}, {null_data} )" )



    def __insert_data(self, rows):
        now_time = int(datetime.datetime.timestamp(datetime.datetime.now()) * 1000)
        data = self.__data_set(rows)
        for i in range(rows):
            tdSql.execute(
                f'''insert into ct1 values (
                { now_time - i * 1000 }, {i}, {11111 * i}, {111 * i % 32767 }, {11 * i % 127}, {1.11*i}, {1100.0011*i},
                {i%2}, 'binary{i}', 'nchar_测试_{i}', { now_time + 1 * i }, {11 * i % 127}, {111 * i % 32767}, {i}, {11111 * i} )'''
            )
            tdSql.execute(
                f'''insert into ct4 values (
                { now_time - i * 7776000000 }, {i}, {11111 * i}, {111 * i % 32767 }, {11 * i % 127}, {1.11*i}, {1100.0011*i},
                {i%2}, 'binary{i}', 'nchar_测试_{i}', { now_time + 1 * i }, {11 * i % 127}, {111 * i % 32767}, {i}, {11111 * i} )'''
            )
            tdSql.execute(
                f'''insert into ct2 values (
                { now_time - i * 7776000000 }, {-i},  {-11111 * i}, {-111 * i % 32767 }, {-11 * i % 127}, {-1.11*i}, {-1100.0011*i},
                {i%2}, 'binary{i}', 'nchar_测试_{i}', { now_time + 1 * i }, {11 * i % 127}, {111 * i % 32767}, {i}, {11111 * i} )'''
            )
        tdSql.execute(
            f'''insert into ct1 values
            ( { now_time - rows * 5 }, 0, 0, 0, 0, 0, 0, 0, 'binary0', 'nchar_测试_0', { now_time + 8 }, 0, 0, 0, 0)
            ( { now_time + 10000 }, { rows }, -99999, -999, -99, -9.99, -99.99, 1, 'binary9', 'nchar_测试_9', { now_time + 9 }, 0, 0, 0, 0 )
            '''
        )

        tdSql.execute(
            f'''insert into ct4 values
            ( { now_time - rows * 7776000000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            ( { now_time - rows * 3888000000 + 10800000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            ( { now_time +  7776000000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            (
                { now_time + 5184000000}, {pow(2,31)-pow(2,15)}, {pow(2,63)-pow(2,30)}, 32767, 127,
                { 3.3 * pow(10,38) }, { 1.3 * pow(10,308) }, { rows % 2 }, "binary_limit-1", "nchar_测试_limit-1", { now_time - 86400000},
                254, 65534, {pow(2,32)-pow(2,16)}, {pow(2,64)-pow(2,31)}
                )
            (
                { now_time + 2592000000 }, {pow(2,31)-pow(2,16)}, {pow(2,63)-pow(2,31)}, 32766, 126,
                { 3.2 * pow(10,38) }, { 1.2 * pow(10,308) }, { (rows-1) % 2 }, "binary_limit-2", "nchar_测试_limit-2", { now_time - 172800000},
                255, 65535, {pow(2,32)-pow(2,15)}, {pow(2,64)-pow(2,30)}
                )
            '''
        )

        tdSql.execute(
            f'''insert into ct2 values
            ( { now_time - rows * 7776000000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            ( { now_time - rows * 3888000000 + 10800000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            ( { now_time + 7776000000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            (
                { now_time + 5184000000 }, { -1 * pow(2,31) + pow(2,15) }, { -1 * pow(2,63) + pow(2,30) }, -32766, -126, { -1 * 3.2 * pow(10,38) },
                { -1.2 * pow(10,308) }, { rows % 2 }, "binary_limit-1", "nchar_测试_limit-1", { now_time - 86400000 }, 1, 1, 1, 1
                )
            (
                { now_time + 2592000000 }, { -1 * pow(2,31) + pow(2,16) }, { -1 * pow(2,63) + pow(2,31) }, -32767, -127, { - 3.3 * pow(10,38) },
                { -1.3 * pow(10,308) }, { (rows-1) % 2 }, "binary_limit-2", "nchar_测试_limit-2", { now_time - 172800000 }, 1, 1, 1, 1
                )
            '''
        )

        for i in range(rows):
            insert_data = f'''insert into t1 values
                ( { now_time - i * 3600000 }, {i}, {i * 11111}, { i % 32767 }, { i % 127}, { i * 1.11111 }, { i * 1000.1111 }, { i % 2},
                "binary_{i}", "nchar_测试_{i}", { now_time - 1000 * i }, {i % 127}, {i % 32767}, {i}, {i * 11111})
                '''
            tdSql.execute(insert_data)
        tdSql.execute(
            f'''insert into t1 values
            ( { now_time + 10800000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            ( { now_time - (( rows // 2 ) * 60 + 30) * 60000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
            ( { now_time - rows * 3600000 }, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )
            (
                { now_time + 7200000 }, { pow(2,31) - pow(2,15) }, { pow(2,63) - pow(2,30) }, 32767, 127, { 3.3 * pow(10,38) },
                { 1.3 * pow(10,308) }, { rows % 2 }, "binary_limit-1", "nchar_测试_limit-1", { now_time - 86400000 },
                254, 65534, {pow(2,32)-pow(2,16)}, {pow(2,64)-pow(2,31)}
                )
            (
                { now_time + 3600000 } , { pow(2,31) - pow(2,16) }, { pow(2,63) - pow(2,31) }, 32766, 126, { 3.2 * pow(10,38) },
                { 1.2 * pow(10,308) }, { (rows-1) % 2 }, "binary_limit-2", "nchar_测试_limit-2", { now_time - 172800000 },
                255, 65535, {pow(2,32)-pow(2,15)}, {pow(2,64)-pow(2,30)}
                )
            '''
        )


    def run(self):
        self.rows = 10

        tdLog.printNoPrefix("==========step0:all check")
        self.all_test()

        tdLog.printNoPrefix("==========step1:create table in normal database")
        tdSql.prepare()
        self.__create_tb()
        # self.__insert_data(self.rows)
        self.__insert_data_0()
        # return

        tdLog.printNoPrefix("==========step2:create table in rollup database")
        tdSql.execute("create database db3 retentions 1s:4m,2s:8m,3s:12m")
        tdSql.execute("use db3")
        # return
        self.__create_tb()
        self.__insert_data_0()

        tdSql.execute("drop database if exists db1 ")
        tdSql.execute("drop database if exists db2 ")

        tdDnodes.stop(1)
        tdDnodes.start(1)

        tdLog.printNoPrefix("==========step4:after wal, all check again ")
        self.all_test()

    def stop(self):
        tdSql.close()
        tdLog.success(f"{__file__} successfully executed")

tdCases.addLinux(__file__, TDTestCase())
tdCases.addWindows(__file__, TDTestCase())
