#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"


#define ASSERT_EQUALS_RECORDS(_l,_r, _schema, _message)					\
	do {																\
		Record *_lR = _l;												\
		Record *_rR = _r;												\
		ASSERT_TRUE(memcmp(_lR->data,_rR->data,getRecordSize(_schema)) == 0, _message); \
		int _i_attr;													\
		for(_i_attr = 0; _i_attr < _schema->numAttr; _i_attr++)			\
		{																\
			Value *_lVal, *_rVal;										\
			char *_lSer, *_rSer;										\
			getAttr(_lR, _schema, _i_attr, &_lVal);						\
			getAttr(_rR, _schema, _i_attr, &_rVal);						\
			_lSer = serializeValue(_lVal);								\
			_rSer = serializeValue(_rVal);								\
			ASSERT_EQUALS_STRING(_lSer, _rSer, "attr same");			\
			free(_lVal);												\
			free(_rVal);												\
			free(_lSer);												\
			free(_rSer);												\
		}																\
	} while(0)

#define ASSERT_EQUALS_RECORD_IN(_l,_r, _rSize, _schema, _message)		\
	do {																\
		int _i_attr;													\
		boolean _found = false;											\
		for(_i_attr = 0; _i_attr < _rSize; _i_attr++)					\
		{																\
			if (memcmp(_l->data,_r[_i_attr]->data,getRecordSize(_schema)) == 0)	\
			{															\
				_found = true;											\
			}															\
		}																\
		ASSERT_TRUE(0, _message);										\
	} while(0)

#define OP_TRUE(_left, _right, _op, _message)					\
	do {														\
		Value *_op_result = (Value *) malloc(sizeof(Value));	\
		_op(_left, _right, _op_result);							\
		bool _bool_op_result = _op_result->v.boolV;				\
		free(_op_result);										\
		ASSERT_TRUE(_bool_op_result,_message);					\
	} while (0)

// test methods
static void testRecords (void);
static void testCreateTableAndInsert (void);
static void testUpdateTable (void);
static void testScans (void);
static void testScansTwo (void);
static void testInsertManyRecords(void);
static void testMultipleScans(void);

// struct for test records
typedef struct TestRecord {
	int a;
	char *b;
	int c;
} TestRecord;

// helper methods
Record *testRecord(Schema *schema, int a, char *b, int c);
Schema *testSchema (void);
Record *fromTestRecord (Schema *schema, TestRecord in);

// test name
char *testName;

// main method
int
main (void)
{
	testName = "";

	testInsertManyRecords();
	testRecords();
	testCreateTableAndInsert();
	testUpdateTable();
	testScans();
	testScansTwo();
	testMultipleScans();

	return 0;
}

// ************************************************************
void
testRecords (void)
{
	TestRecord expected[] = {
			{1, "aaaa", 3},
	};
	Schema *schema;
	Record *r;
	Value *value;
    Value *ovalue;
	testName = "test creating records and manipulating attributes";

	// check attributes of created record
	schema = testSchema();
	r = fromTestRecord(schema, expected[0]);

	getAttr(r, schema, 0, &value);
    ovalue = stringToValue("i1");
	OP_TRUE(ovalue, value, valueEquals, "first attr");
    freeVal(ovalue);
	freeVal(value);

	getAttr(r, schema, 1, &value);
    ovalue = stringToValue("saaaa");
	OP_TRUE(ovalue, value, valueEquals, "second attr");
    freeVal(ovalue);
	freeVal(value);

	getAttr(r, schema, 2, &value);
    ovalue = stringToValue("i3");
	OP_TRUE(ovalue, value, valueEquals, "third attr");
    freeVal(ovalue);
	freeVal(value);

	//modify attrs
    ovalue = stringToValue("i4");
	setAttr(r, schema, 2, ovalue);
	getAttr(r, schema, 2, &value);
	OP_TRUE(ovalue, value, valueEquals, "third attr after setting");
    freeVal(ovalue);
	freeVal(value);

	freeRecord(r);
	freeSchema(schema);
	TEST_DONE();
}

// ************************************************************
void
testCreateTableAndInsert (void)
{
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = {
			{1, "aaaa", 3},
			{2, "bbbb", 2},
			{3, "cccc", 1},
			{4, "dddd", 3},
			{5, "eeee", 5},
			{6, "ffff", 1},
			{7, "gggg", 3},
			{8, "hhhh", 3},
			{9, "iiii", 2}
	};
	int numInserts = 9, i;
	Record *r;
	RID *rids;
	Schema *schema;
	testName = "test creating a new table and inserting tuples";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_r",schema));
	TEST_CHECK(openTable(table, "test_table_r"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
	{
		r = fromTestRecord(schema, inserts[i]);
		TEST_CHECK(insertRecord(table,r));
		rids[i] = r->id;
        freeRecord(r);
	}

	TEST_CHECK(closeTable(table));
	TEST_CHECK(openTable(table, "test_table_r"));

	createRecord(&r, schema);

	// randomly retrieve records from the table and compare to inserted ones
	for(i = 0; i < 1000; i++)
	{
        Record *rexp;
		int pos = rand() % numInserts;
		RID rid = rids[pos];
		TEST_CHECK(getRecord(table, rid, r));
        rexp = fromTestRecord(schema, inserts[pos]);
		ASSERT_EQUALS_RECORDS(rexp, r, schema, "compare records");
        freeRecord(rexp);
	}
	freeRecord(r);

	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	free(rids);
	free(table);
	freeSchema(schema);
	TEST_DONE();
}

void
testMultipleScans(void)
{
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = {
			{1, "aaaa", 3},
			{2, "bbbb", 2},
			{3, "cccc", 1},
			{4, "dddd", 3},
			{5, "eeee", 5},
			{6, "ffff", 1},
			{7, "gggg", 3},
			{8, "hhhh", 3},
			{9, "iiii", 2},
			{10, "jjjj", 5},
	};
	int numInserts = 10, i, scanOne=0, scanTwo=0;
	Record *r;
	RID *rids;
	Schema *schema;
	testName = "test running muliple scans ";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);
	RM_ScanHandle *sc1 = (RM_ScanHandle *) malloc(sizeof(RM_ScanHandle));
	RM_ScanHandle *sc2 = (RM_ScanHandle *) malloc(sizeof(RM_ScanHandle));
	Expr *se1, *left, *right;
	int rc,rc2;

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_r",schema));
	TEST_CHECK(openTable(table, "test_table_r"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
	{
		r = fromTestRecord(schema, inserts[i]);
		TEST_CHECK(insertRecord(table,r));
		rids[i] = r->id;
        freeRecord(r);
	}

	// Mix 2 scans with c=3 as condition
	MAKE_CONS(left, stringToValue("i3"));
	MAKE_ATTRREF(right, 2);
	MAKE_BINOP_EXPR(se1, left, right, OP_COMP_EQUAL);
	createRecord(&r, schema);
	TEST_CHECK(startScan(table, sc1, se1));
	TEST_CHECK(startScan(table, sc2, se1));
	if ((rc2 = next(sc2, r)) == RC_OK)
		scanTwo++;
	i = 0;
	while((rc = next(sc1, r)) == RC_OK)
	{
		scanOne++;
		i++;
		if (i % 3 == 0)
			if ((rc2 = next(sc2, r)) == RC_OK)
				scanTwo++;
	}
	while((rc2 = next(sc2, r)) == RC_OK)
		scanTwo++;

	ASSERT_TRUE(scanOne == scanTwo, "scans returned same number of tuples");
	if (rc != RC_RM_NO_MORE_TUPLES)
		TEST_CHECK(rc);
	TEST_CHECK(closeScan(sc1));
	TEST_CHECK(closeScan(sc2));

	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	// free stuff
	freeSchema(schema);
	free(sc1);
	free(sc2);
	freeExpr(se1);
    freeRecord(r);
	free(rids);
	free(table);
	TEST_DONE();
}

void
testUpdateTable (void)
{
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = {
			{1, "aaaa", 3},
			{2, "bbbb", 2},
			{3, "cccc", 1},
			{4, "dddd", 3},
			{5, "eeee", 5},
			{6, "ffff", 1},
			{7, "gggg", 3},
			{8, "hhhh", 3},
			{9, "iiii", 2},
			{10, "jjjj", 5},
	};
	TestRecord updates[] = {
			{1, "iiii", 6},
			{2, "iiii", 6},
			{3, "iiii", 6}
	};
	int deletes[] = {
			9,
			6,
			7,
			8,
			5
	};
	TestRecord finalR[] = {
			{1, "iiii", 6},
			{2, "iiii", 6},
			{3, "iiii", 6},
			{4, "dddd", 3},
			{5, "eeee", 5},
	};
	int numInserts = 10, numUpdates = 3, numDeletes = 5, numFinal = 5, i;
	Record *r;
	RID *rids;
	Schema *schema;
	testName = "test creating a new table and insert,update,delete tuples";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_r",schema));
	TEST_CHECK(openTable(table, "test_table_r"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
	{
		r = fromTestRecord(schema, inserts[i]);
		TEST_CHECK(insertRecord(table,r));
		rids[i] = r->id;
        freeRecord(r);
	}

	// delete rows from table
	for(i = 0; i < numDeletes; i++)
	{
		TEST_CHECK(deleteRecord(table,rids[deletes[i]]));
	}

	// update rows into table
	for(i = 0; i < numUpdates; i++)
	{
		r = fromTestRecord(schema, updates[i]);
		r->id = rids[i];
		TEST_CHECK(updateRecord(table,r));
        freeRecord(r);
	}

	TEST_CHECK(closeTable(table));
	TEST_CHECK(openTable(table, "test_table_r"));

	// retrieve records from the table and compare to expected final stage
	createRecord(&r,schema);

	for(i = 0; i < numFinal; i++)
	{
        Record *rexp;
		RID rid = rids[i];
		TEST_CHECK(getRecord(table, rid, r));
        rexp = fromTestRecord(schema, finalR[i]);
		ASSERT_EQUALS_RECORDS(rexp,r, schema, "compare records");
        freeRecord(rexp);
	}

	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	// free stuff
	freeRecord(r);
	freeSchema(schema);
	free(rids);
	free(table);
	TEST_DONE();
}

void
testInsertManyRecords(void)
{
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = {
			{1, "aaaa", 3},
			{2, "bbbb", 2},
			{3, "cccc", 1},
			{4, "dddd", 3},
			{5, "eeee", 5},
			{6, "ffff", 1},
			{7, "gggg", 3},
			{8, "hhhh", 3},
			{9, "iiii", 2},
			{10, "jjjj", 5},
	};
	TestRecord realInserts[10000];
	TestRecord updates[] = {
			{3333, "iiii", 6}
	};
	int numInserts = 10000, i;
	int randomRec = 3333;
	Record *r, *rexp;
	RID *rids;
	Schema *schema;
	testName = "test creating a new table and inserting 10000 records then updating record from rids[3333]";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_t",schema));
	TEST_CHECK(openTable(table, "test_table_t"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
	{
		realInserts[i] = inserts[i%10];
		realInserts[i].a = i;
		r = fromTestRecord(schema, realInserts[i]);
		TEST_CHECK(insertRecord(table,r));
		rids[i] = r->id;
        freeRecord(r);
	}
	TEST_CHECK(closeTable(table));
	TEST_CHECK(openTable(table, "test_table_t"));

	// retrieve records from the table and compare to expected final stage
	createRecord(&r, schema);
	for(i = 0; i < numInserts; i++)
	{
        Record *rexp;
		RID rid = rids[i];

		TEST_CHECK(getRecord(table, rid, r));
        rexp = fromTestRecord(schema, realInserts[i]);
		ASSERT_EQUALS_RECORDS(rexp, r, schema, "compare records");

        freeRecord(rexp);
	}
	freeRecord(r);

	r = fromTestRecord(schema, updates[0]);
	r->id = rids[randomRec];
	TEST_CHECK(updateRecord(table,r));
    freeRecord(r);

	createRecord(&r, schema);
	TEST_CHECK(getRecord(table, rids[randomRec], r));
    rexp = fromTestRecord(schema, updates[0]);
	ASSERT_EQUALS_RECORDS(rexp, r, schema, "compare records");
    freeRecord(r);
    freeRecord(rexp);

	// free stuff
	freeSchema(schema);
	free(rids);
	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_t"));
	TEST_CHECK(shutdownRecordManager());

	free(table);
	TEST_DONE();
}

void testScans (void)
{
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = {
			{1, "aaaa", 3},
			{2, "bbbb", 2},
			{3, "cccc", 1},
			{4, "dddd", 3},
			{5, "eeee", 5},
			{6, "ffff", 1},
			{7, "gggg", 3},
			{8, "hhhh", 3},
			{9, "iiii", 2},
			{10, "jjjj", 5},
	};
	TestRecord scanOneResult[] = {
			{3, "cccc", 1},
			{6, "ffff", 1},
	};
	bool foundScan[] = {
			FALSE,
			FALSE
	};
	int numInserts = 10, scanSizeOne = 2, i;
	Record *r;
	RID *rids;
	Schema *schema;
	RM_ScanHandle *sc = (RM_ScanHandle *) malloc(sizeof(RM_ScanHandle));
	Expr *sel, *left, *right;
	int rc;

	testName = "test creating a new table and inserting tuples";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_r",schema));
	TEST_CHECK(openTable(table, "test_table_r"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
	{
		r = fromTestRecord(schema, inserts[i]);
		TEST_CHECK(insertRecord(table,r));
		rids[i] = r->id;
        freeRecord(r);
	}

	TEST_CHECK(closeTable(table));
	TEST_CHECK(openTable(table, "test_table_r"));

	// run some scans
	MAKE_CONS(left, stringToValue("i1"));
	MAKE_ATTRREF(right, 2);
	MAKE_BINOP_EXPR(sel, left, right, OP_COMP_EQUAL);

	createRecord(&r, schema);
	TEST_CHECK(startScan(table, sc, sel));
	while((rc = next(sc, r)) == RC_OK)
	{
		for(i = 0; i < scanSizeOne; i++)
		{
            Record *rexp = fromTestRecord(schema, scanOneResult[i]);
			if (memcmp(rexp->data,r->data,getRecordSize(schema)) == 0)
				foundScan[i] = TRUE;
            freeRecord(rexp);
		}
	}
	if (rc != RC_RM_NO_MORE_TUPLES)
		TEST_CHECK(rc);
	TEST_CHECK(closeScan(sc));
	for(i = 0; i < scanSizeOne; i++)
		ASSERT_TRUE(foundScan[i], "check for scan result");

	// clean up
	freeRecord(r);
	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	freeSchema(schema);
	free(rids);
	free(table);
	free(sc);
	freeExpr(sel);
	TEST_DONE();
}


void testScansTwo (void)
{
	RM_TableData *table = (RM_TableData *) malloc(sizeof(RM_TableData));
	TestRecord inserts[] = {
			{1, "aaaa", 3},
			{2, "bbbb", 2},
			{3, "cccc", 1},
			{4, "dddd", 3},
			{5, "eeee", 5},
			{6, "ffff", 1},
			{7, "gggg", 3},
			{8, "hhhh", 3},
			{9, "iiii", 2},
			{10, "jjjj", 5},
	};
	bool foundScan[] = {
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE,
			FALSE
	};
	int numInserts = 10, i;
	Record *r;
	RID *rids;
	Schema *schema;
	RM_ScanHandle *sc = (RM_ScanHandle *) malloc(sizeof(RM_ScanHandle));
	Expr *sel, *left, *right, *first, *se;
	int rc;

	testName = "test creating a new table and inserting tuples";
	schema = testSchema();
	rids = (RID *) malloc(sizeof(RID) * numInserts);

	TEST_CHECK(initRecordManager(NULL));
	TEST_CHECK(createTable("test_table_r",schema));
	TEST_CHECK(openTable(table, "test_table_r"));

	// insert rows into table
	for(i = 0; i < numInserts; i++)
	{
		r = fromTestRecord(schema, inserts[i]);
		TEST_CHECK(insertRecord(table,r));
		rids[i] = r->id;
        freeRecord(r);
	}

	TEST_CHECK(closeTable(table));
	TEST_CHECK(openTable(table, "test_table_r"));

	// Select 1 record with INT in condition a=2.
	MAKE_CONS(left, stringToValue("i2"));
	MAKE_ATTRREF(right, 0);
	MAKE_BINOP_EXPR(sel, left, right, OP_COMP_EQUAL);
	createRecord(&r, schema);
	TEST_CHECK(startScan(table, sc, sel));
	while((rc = next(sc, r)) == RC_OK)
	{
        Record *rexp = fromTestRecord(schema, inserts[1]);
		ASSERT_EQUALS_RECORDS(rexp, r, schema, "compare records");
        freeRecord(rexp);
	}
	if (rc != RC_RM_NO_MORE_TUPLES)
		TEST_CHECK(rc);
	TEST_CHECK(closeScan(sc));
	freeExpr(sel);

	// Select 1 record with STRING in condition b='ffff'.
	MAKE_CONS(left, stringToValue("sffff"));
	MAKE_ATTRREF(right, 1);
	MAKE_BINOP_EXPR(sel, left, right, OP_COMP_EQUAL);
	TEST_CHECK(startScan(table, sc, sel));
	while((rc = next(sc, r)) == RC_OK)
	{
        Record *rexp = fromTestRecord(schema, inserts[5]);
		ASSERT_EQUALS_RECORDS(rexp, r, schema, "compare records");
		serializeRecord(r, schema);
        freeRecord(rexp);
	}
	if (rc != RC_RM_NO_MORE_TUPLES)
		TEST_CHECK(rc);
	TEST_CHECK(closeScan(sc));
	freeExpr(sel);

	// Select all records, with condition being false
	MAKE_CONS(left, stringToValue("i4"));
	MAKE_ATTRREF(right, 2);
	MAKE_BINOP_EXPR(first, right, left, OP_COMP_SMALLER);
	MAKE_UNOP_EXPR(se, first, OP_BOOL_NOT);
	TEST_CHECK(startScan(table, sc, se));
	while((rc = next(sc, r)) == RC_OK)
	{
		serializeRecord(r, schema);
		for(i = 0; i < numInserts; i++)
		{
            Record *rexp = fromTestRecord(schema, inserts[i]);
			if (memcmp(rexp->data,r->data,getRecordSize(schema)) == 0)
				foundScan[i] = TRUE;
            freeRecord(rexp);
		}
	}
	if (rc != RC_RM_NO_MORE_TUPLES)
		TEST_CHECK(rc);
	TEST_CHECK(closeScan(sc));
	freeExpr(se);

	ASSERT_TRUE(!foundScan[0], "not greater than four");
	ASSERT_TRUE(foundScan[4], "greater than four");
	ASSERT_TRUE(foundScan[9], "greater than four");

	// clean up
	TEST_CHECK(closeTable(table));
	TEST_CHECK(deleteTable("test_table_r"));
	TEST_CHECK(shutdownRecordManager());

	freeRecord(r);
	free(table);
	free(sc);
	freeExpr(sel);
	TEST_DONE();
}


Schema *
testSchema (void)
{
	Schema *result;
	char *names[] = { "a", "b", "c" };
	DataType dt[] = { DT_INT, DT_STRING, DT_INT };
	int sizes[] = { 0, 4, 0 };
	int keys[] = {0};
	int i;
	char **cpNames = (char **) malloc(sizeof(char*) * 3);
	DataType *cpDt = (DataType *) malloc(sizeof(DataType) * 3);
	int *cpSizes = (int *) malloc(sizeof(int) * 3);
	int *cpKeys = (int *) malloc(sizeof(int));

	for(i = 0; i < 3; i++)
	{
		cpNames[i] = (char *) malloc(2);
		strcpy(cpNames[i], names[i]);
	}
	memcpy(cpDt, dt, sizeof(DataType) * 3);
	memcpy(cpSizes, sizes, sizeof(int) * 3);
	memcpy(cpKeys, keys, sizeof(int));

	result = createSchema(3, cpNames, cpDt, cpSizes, 1, cpKeys);

	return result;
}

Record *
fromTestRecord (Schema *schema, TestRecord in)
{
	return testRecord(schema, in.a, in.b, in.c);
}

Record *
testRecord(Schema *schema, int a, char *b, int c)
{
	Record *result;
	Value *value;

	TEST_CHECK(createRecord(&result, schema));

	MAKE_VALUE(value, DT_INT, a);
	TEST_CHECK(setAttr(result, schema, 0, value));
	freeVal(value);

	MAKE_STRING_VALUE(value, b);
	TEST_CHECK(setAttr(result, schema, 1, value));
	freeVal(value);

	MAKE_VALUE(value, DT_INT, c);
	TEST_CHECK(setAttr(result, schema, 2, value));
	freeVal(value);

	return result;
}
