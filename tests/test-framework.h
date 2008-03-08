/* Test framework
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#ifndef _TEST_TEST_H
#define _TEST_TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>




typedef int test_bool;




struct _test_case;
typedef void test_proc(struct _test_case* tc);
typedef struct _test_case {
	const char* name;  /* assumes the pointed value is literal. */
	test_proc* proc;
	test_bool failedp;
	jmp_buf cont;
	char* message;
} test_case;

test_case* _test_case_new(const char* name, test_proc* proc);
#define test_case_new(proc) _test_case_new(#proc, proc)
void test_case_delete(test_case* tc);

void test_case_run(test_case* tc);


#define TEST_ASSERT(condition) \
        _test_assert(_test_tc, __FILE__, __LINE__, \
                     #condition, (condition))
#define TEST_ASSERT_INT(expected, actual) \
        _test_assert_int(_test_tc, __FILE__, __LINE__, \
                         #actual, (expected), (actual))
#define TEST_ASSERT_STR(expected, actual) \
        _test_assert_str(_test_tc, __FILE__, __LINE__, \
                         #actual, (expected), (actual))
#define TEST_ASSERT_PTR(expected, actual) \
        _test_assert_ptr(_test_tc, __FILE__, __LINE__, \
                         #actual, (expected), (actual))
#define TEST_ASSERT_PTR_NULL(actual) \
        _test_assert(_test_tc, __FILE__, __LINE__, \
                     #actual " == NULL", (actual) == NULL)
#define TEST_ASSERT_PTR_NOT_NULL(actual) \
        _test_assert(_test_tc, __FILE__, __LINE__, \
                     #actual " != NULL", (actual) != NULL)

void _test_assert(test_case* tc, const char* file, int line, const char* msg,
                  test_bool condition);
void _test_assert_int(test_case* tc, const char*file, int line, const char*msg,
                      int expected, int actual);
void _test_assert_str(test_case* tc, const char*file, int line, const char*msg,
                      const char* expected, const char* actual);
void _test_assert_ptr(test_case* tc, const char*file, int line, const char*msg,
                      const void* expected, const void* actual);




typedef struct {
	test_case** cases;
	int all_count;
} test_suite;

test_suite* test_suite_new(void);
void test_suite_delete(test_suite* ts);

void test_suite_add_test_case(test_suite* ts, test_case* tc);

void test_suite_run(test_suite* ts);
void test_suite_result(test_suite* ts);




#define TEST_PROC_NAME(name) _test__##name
#define TEST_PROC(name) \
        static void \
        TEST_PROC_NAME(name)(test_case* _test_tc)

#endif  /* _TEST_TEST_H */
