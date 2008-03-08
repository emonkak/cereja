/* Test framework
 *
 * Copyright (C) 2006 kana <http://nicht.s8.xrea.com/>
 * $Id$
 */

#include <stdarg.h>
#include <string.h>

#include "test-framework.h"








/* Misc. */

static void
panic(const char* message)
{
	fputs(message, stderr);
	fputs("\n", stderr);
	exit(EXIT_FAILURE);
}




static void*
pmalloc(size_t size)
{
	void* p;

	p = malloc(size);
	if (p == NULL)
		panic("Out of memory");
	return p;
}

static void*
prealloc(void* old, size_t size)
{
	void* p;

	p = realloc(old, size);
	if (p == NULL)
		panic("Out of memory");
	return p;
}




#define MAX_STR_LEN 1024
static char*
mnprintf(const char* format, ...)
{
	va_list va;
	char buf[MAX_STR_LEN];
	int length;
	char* s;

	va_start(va, format);
	vsnprintf(buf, MAX_STR_LEN, format, va);
	buf[MAX_STR_LEN - 1] = '\0';
	va_end(va);

	length = strlen(buf);
	s = pmalloc((length+1) * sizeof(char));
	strcpy(s, buf);
	return s;
}








/* test_case */

test_case*
_test_case_new(const char* name, test_proc* proc)
{
	test_case* tc;

	tc = pmalloc(sizeof(*tc));
	tc->name = name;
	tc->proc = proc;
	/* tc->failedp = ...; */
	/* tc->cont = ...; */
	tc->message = NULL;
	return tc;
}


void
test_case_delete(test_case* tc)
{
	free(tc->message);
	free(tc);
}




#define FALSE 0
#define TRUE (!FALSE)
void
test_case_run(test_case* tc)
{
	if (setjmp(tc->cont) == 0) {
		tc->failedp = FALSE;
		(*(tc->proc))(tc);
	} else {
		tc->failedp = TRUE;
	}
}




void
_test_assert(test_case* tc, const char* file, int line, const char* msg,
             test_bool condition)
{
	if (!condition) {
		tc->message = mnprintf("%s:%d: %s - failed", file, line, msg);
		longjmp(tc->cont, !0);
	}
}


void
_test_assert_int(test_case* tc, const char* file, int line, const char* msg,
                 int expected, int actual)
{
	if (expected != actual) {
		tc->message = mnprintf("%s:%d: %s - expected `%d', got `%d'",
		                       file, line, msg, expected, actual);
		longjmp(tc->cont, !0);
	}
}


void
_test_assert_str(test_case* tc, const char* file, int line, const char* msg,
                 const char* expected, const char* actual)
{
	if (strcmp(expected, actual) != 0) {
		tc->message = mnprintf("%s:%d: %s - expected `%s', got `%s'",
		                       file, line, msg, expected, actual);
		longjmp(tc->cont, !0);
	}
}


void
_test_assert_ptr(test_case* tc, const char* file, int line, const char* msg,
                 const void* expected, const void* actual)
{
	if (expected != actual) {
		tc->message = mnprintf("%s:%d: %s - expected `%p', got `%p'",
		                       file, line, msg, expected, actual);
		longjmp(tc->cont, !0);
	}
}








/* test_suite */

test_suite*
test_suite_new(void)
{
	test_suite* ts;

	ts = pmalloc(sizeof(*ts));
	ts->cases = NULL;
	ts->all_count = 0;
	return ts;
}


void
test_suite_delete(test_suite* ts)
{
	int i;

	for (i = 0; i < ts->all_count; i++)
		test_case_delete(ts->cases[i]);
	free(ts->cases);
	free(ts);
}




void
test_suite_add_test_case(test_suite* ts, test_case* tc)
{
	ts->cases = prealloc(ts->cases,
	                     (ts->all_count + 1) * sizeof(ts->cases[0]));
	ts->cases[ts->all_count] = tc;
	ts->all_count++;
}




void
test_suite_run(test_suite* ts)
{
	int i;

	for (i = 0; i < ts->all_count; i++)
		test_case_run(ts->cases[i]);
}


void
test_suite_result(test_suite* ts)
{
	int i;
	int failed_count;

	for (i = 0; i < ts->all_count; i++) {
		fputc((ts->cases[i]->failedp ? 'F' : '.'), stdout);
		if ((i+1) % 50 == 0)
			fputs("\n", stdout);
		else if ((i+1) % 10 == 0)
			fputs("   ", stdout);
		else if ((i+1) % 5 == 0)
			fputs(" ", stdout);
		else
			;  /* nop */
	}
	if (ts->all_count % 50 != 0)
		fputs("\n", stdout);

	fputs("\n", stdout);
	failed_count = 0;
	for (i = 0; i < ts->all_count; i++) {
		if (ts->cases[i]->failedp) {
			failed_count++;
			fprintf(stdout, "%s\n", ts->cases[i]->message);
		}
	}

	fputs("\n", stdout);
	if (failed_count == 0) {
		fprintf(stdout, "OK (%d tests)\n", ts->all_count);
	} else {
		fprintf(stdout, "FAILED (ran %d, pass %d, failed %d)\n",
		        ts->all_count,
		        ts->all_count - failed_count,
		        failed_count);
	}
}

/* __END__ */
