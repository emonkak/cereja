/* Test: Test framework
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */




/* BUGS: assumes sizeof(long) == sizeof(data*) == sizeof(func*) */
#define PTR(p) ((void*)(unsigned long)(p))




static void bool_success(test_case* _test_tc) {TEST_ASSERT(!0);}
static void bool_failure(test_case* _test_tc) {TEST_ASSERT(0);}

static void int_success(test_case* _test_tc) {TEST_ASSERT_INT(123, 123);}
static void int_failure(test_case* _test_tc) {TEST_ASSERT_INT(123, 789);}

static void str_success(test_case* _test_tc) {TEST_ASSERT_STR("AbC", "AbC");}
static void str_failure(test_case* _test_tc) {TEST_ASSERT_STR("AbC", "aBc");}

static void ptr_success(test_case* _test_tc) {TEST_ASSERT_PTR(NULL, NULL);}
static void ptr_failure(test_case* _test_tc) {TEST_ASSERT_PTR(NULL, "ptr");}

static void ptr_null_success(test_case* _test_tc) {TEST_ASSERT_PTR_NULL(NULL);}
static void ptr_null_failure(test_case* _test_tc) {TEST_ASSERT_PTR_NULL("xx");}

static void ptr_not_null_success(test_case* _test_tc)
	{TEST_ASSERT_PTR_NOT_NULL("xx");}
static void ptr_not_null_failure(test_case* _test_tc)
	{TEST_ASSERT_PTR_NOT_NULL(NULL);}








/* test_case */

TEST_PROC(test_case_new_and_test_case_delete)
{
	test_case* tc;

	tc = test_case_new(bool_success);

	TEST_ASSERT_PTR_NOT_NULL(tc);
	TEST_ASSERT_STR("bool_success", tc->name);
	TEST_ASSERT_PTR(PTR(bool_success), PTR(tc->proc));
	TEST_ASSERT_PTR_NULL(tc->message);

	test_case_delete(tc);
}


TEST_PROC(test_case_run_with_success)
{
	test_case* tc;

	tc = test_case_new(bool_success);

	TEST_ASSERT_PTR_NOT_NULL(tc);
	TEST_ASSERT_STR("bool_success", tc->name);
	TEST_ASSERT_PTR(PTR(bool_success), PTR(tc->proc));
	TEST_ASSERT_PTR_NULL(tc->message);

	test_case_run(tc);
	TEST_ASSERT(!(tc->failedp));

	test_case_delete(tc);
}


TEST_PROC(test_case_run_with_failure)
{
	test_case* tc;

	tc = test_case_new(bool_failure);

	TEST_ASSERT_PTR_NOT_NULL(tc);
	TEST_ASSERT_STR("bool_failure", tc->name);
	TEST_ASSERT_PTR(PTR(bool_failure), PTR(tc->proc));
	TEST_ASSERT_PTR_NULL(tc->message);

	test_case_run(tc);
	TEST_ASSERT(tc->failedp);

	test_case_delete(tc);
}








/* test_suite */

TEST_PROC(test_suite_success)
{
	test_suite* ts;
	test_case* tc0;
	test_case* tc1;
	test_case** tcs;
	int i;

	ts = test_suite_new();

	TEST_ASSERT_INT(0, ts->all_count);
	TEST_ASSERT_PTR_NULL(ts->cases);

	tc0 = test_case_new(bool_success);
	test_suite_add_test_case(ts, tc0);
	TEST_ASSERT_INT(1, ts->all_count);
	TEST_ASSERT_PTR_NOT_NULL(ts->cases);
	TEST_ASSERT_PTR(tc0, ts->cases[0]);

	tc1 = test_case_new(int_success);
	test_suite_add_test_case(ts, tc1);
	TEST_ASSERT_INT(2, ts->all_count);
	TEST_ASSERT_PTR_NOT_NULL(ts->cases);
	TEST_ASSERT_PTR(tc1, ts->cases[1]);

	test_suite_add_test_case(ts, test_case_new(str_success));
	test_suite_add_test_case(ts, test_case_new(ptr_success));
	test_suite_add_test_case(ts, test_case_new(ptr_null_success));
	test_suite_add_test_case(ts, test_case_new(ptr_not_null_success));
	TEST_ASSERT_INT(6, ts->all_count);
	TEST_ASSERT_PTR_NOT_NULL(ts->cases);

	tcs = ts->cases;
	test_suite_run(ts);
	TEST_ASSERT_INT(6, ts->all_count);
	TEST_ASSERT_PTR(tcs, ts->cases);
	for (i = 0; i < ts->all_count; i++)
		TEST_ASSERT(!(ts->cases[i]->failedp));

	test_suite_delete(ts);
}


TEST_PROC(test_suite_failure)
{
	test_suite* ts;
	int i;

	ts = test_suite_new();

	test_suite_add_test_case(ts, test_case_new(bool_failure));
	test_suite_add_test_case(ts, test_case_new(int_failure));
	test_suite_add_test_case(ts, test_case_new(str_failure));
	test_suite_add_test_case(ts, test_case_new(ptr_failure));
	test_suite_add_test_case(ts, test_case_new(ptr_null_failure));
	test_suite_add_test_case(ts, test_case_new(ptr_not_null_failure));
	TEST_ASSERT_INT(6, ts->all_count);
	TEST_ASSERT_PTR_NOT_NULL(ts->cases);

	test_suite_run(ts);
	for (i = 0; i < ts->all_count; i++)
		TEST_ASSERT(ts->cases[i]->failedp);

	test_suite_delete(ts);
}


TEST_PROC(test_suite_success_and_failure_mix)
{
	test_suite* ts;

	ts = test_suite_new();

	test_suite_add_test_case(ts, test_case_new(bool_failure));
	test_suite_add_test_case(ts, test_case_new(int_success));
	test_suite_add_test_case(ts, test_case_new(str_success));
	test_suite_add_test_case(ts, test_case_new(ptr_success));
	test_suite_add_test_case(ts, test_case_new(ptr_null_success));
	test_suite_add_test_case(ts, test_case_new(ptr_not_null_failure));
	TEST_ASSERT_INT(6, ts->all_count);
	TEST_ASSERT_PTR_NOT_NULL(ts->cases);

	test_suite_run(ts);
	TEST_ASSERT(ts->cases[0]->failedp);
	TEST_ASSERT(!(ts->cases[1]->failedp));
	TEST_ASSERT(!(ts->cases[2]->failedp));
	TEST_ASSERT(!(ts->cases[3]->failedp));
	TEST_ASSERT(!(ts->cases[4]->failedp));
	TEST_ASSERT(ts->cases[5]->failedp);

	test_suite_delete(ts);
}




/* __END__ */
