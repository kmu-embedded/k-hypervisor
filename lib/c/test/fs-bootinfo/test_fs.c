#include <check.h>
#include <stdio.h>
#include "test_libs_c.h"
#include <string.h>

START_TEST(open_close_test)
{
	FILE *foo = fopen("foo", "r");
	fail_unless(foo != NULL, "Should be able to open foo");
	fail_unless(fclose(foo) == 0, "Should be able to close foo");
}
END_TEST

START_TEST(read_test)
{
	FILE *foo = fopen("foo", "r");
	char data[4];
	int r;
	fail_unless(foo != NULL, "Should be able to open foo");
	r = fread(data, 1, 4, foo);
	fail_unless(r == 4, "Should read 4 bytes from foo");
	fail_unless(strncmp(data, "Test", 4) == 0, "Data read should be 'Test'");
	fail_unless(fclose(foo) == 0, "Should be able to close foo");
}
END_TEST

START_TEST(partial_read_test)
{
	FILE *foo = fopen("foo", "r");
	char data[4];
	int r;

	fail_unless(foo != NULL, "Should be able to open foo");

	r = fread(data, 1, 2, foo);
	fail_unless(r == 2, "Should read 2 bytes from foo");
	fail_unless(strncmp(data, "Te", 2) == 0, "Data read should be 'Te'");

	r = fread(data, 1, 2, foo);
	fail_unless(r == 2, "Should read 2 bytes from foo");
	fail_unless(strncmp(data, "st", 2) == 0, "Data read should be 'st'");

	r = fread(data, 1, 2, foo);
	fail_unless(r == 0, "Should now return 0");
	fail_unless(feof(foo) == 1, "Should now return 0");

	fail_unless(fclose(foo) == 0, "Should be able to close foo");
}
END_TEST

START_TEST(rewind_test)
{
	FILE *foo = fopen("foo", "r");
	char data[4];
	int r;
	fail_unless(foo != NULL, "Should be able to open foo");
	r = fread(data, 1, 4, foo);
	fail_unless(r == 4, "Should read 2 bytes from foo");
	fail_unless(strncmp(data, "Test", 4) == 0, "Data read should be 'Test'");

	rewind(foo);

	r = fread(data, 1, 4, foo);
	fail_unless(r == 4, "Should read 4 bytes from foo");
	fail_unless(strncmp(data, "Test", 4) == 0, "Data read should be 'Test'");

	r = fread(data, 1, 2, foo);
	fail_unless(r == 0, "Should now return 0");
	fail_unless(feof(foo) == 1, "Should now return 0");

	fail_unless(fclose(foo) == 0, "Should be able to close foo");
}
END_TEST


void
fs_test_init(Suite *suite)
{
	TCase *tc;
	tc = tcase_create("fopen"); 
	tcase_add_test(tc, open_close_test);
	tcase_add_test(tc, read_test);
	tcase_add_test(tc, partial_read_test);
	tcase_add_test(tc, rewind_test);
	suite_add_tcase(suite, tc);
}
