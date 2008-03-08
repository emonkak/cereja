#!/bin/sh

echo '/* This code is automatically generated by $Id$. */
#include "test-framework.h"
#line 1'

cat $1

echo '
int
main(void)
{
	test_suite* ts;

	ts = test_suite_new();
'

sed -e 's@^TEST_PROC(\([^()]*\))$@test_suite_add_test_case(ts, test_case_new(TEST_PROC_NAME(\1)));@;t;d' $1

echo '
	test_suite_run(ts);
	test_suite_result(ts);

	test_suite_delete(ts);
	return EXIT_SUCCESS;
}
'

# __END__
