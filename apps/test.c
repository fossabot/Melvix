// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <cpu.h>
#include <math.h>
#include <mem.h>
#include <print.h>
#include <serial.h>
#include <str.h>
#include <sys.h>

#define a_mag 0x55
#define b_mag 0x42

#define TEST(name) static void test_##name(void)
#define CHECK(exp) pass_or_fail(__FILE__, __LINE__, __func__, #exp, "1", exp);
#define EQUALS(first, second)                                                                      \
	pass_or_fail(__FILE__, __LINE__, __func__, #first, #second, (first) == (second));
#define EQUALS_STR(first, second)                                                                  \
	pass_or_fail(__FILE__, __LINE__, __func__, #first, #second, strcmp((first), (second)) == 0);

static u32 failed;

static void pass_or_fail(const char *file_name, int line_num, const char *func, const char *first,
			 const char *second, int success)
{
	failed += success ? 0 : 1;
	log("\x1B[%s\x1B[0m %s:%d: %s: %s == %s\n", success ? "32m[PASS]" : "31m[FAIL]", file_name,
	    line_num, func, first, second);
}

TEST(math)
{
	EQUALS(pow(2, 3), 8);
	EQUALS(pow(0, 3), 0);
	EQUALS(pow(0, 0), 1);
}

TEST(conv)
{
	char buf1[1] = { 0 };
	char buf2[7] = { 0 };
	char buf3[5] = { 0 };
	char buf4[3] = { 0 };
	EQUALS(atoi("42"), 42);
	EQUALS_STR(htoa(0x42), "42");
	EQUALS(htoi("42"), 0x42);
	EQUALS_STR(itoa(42), "42");
	EQUALS_STR(conv_base(42, buf1, 0, 0), "");
	EQUALS_STR(conv_base(42, buf2, 2, 0), "101010");
	EQUALS_STR(conv_base(424242, buf3, 36, 0), "93ci");
	EQUALS_STR(conv_base(0xffffffff, buf4, 10, 1), "-1");
}

TEST(mem)
{
	const char *str0 = "";
	const char *str1 = "";
	const char *str2 = "12345";
	const char *str3 = "12345";
	const char *str4 = "12354";
	EQUALS(memcmp(str4, str2, strlen(str2)), 1);
	EQUALS(memcmp(str2, str4, strlen(str2)), -1);
	EQUALS(memcmp(str2, str3, strlen(str2)), 0);
	EQUALS(memcmp(str0, str1, strlen(str0)), 0);
	EQUALS(memcmp(NULL, NULL, 0), 0);

	char buf[6] = { 0 };
	EQUALS_STR(memcpy(buf, "hallo", 6), "hallo");

	char buf2[6] = { 0 };
	EQUALS_STR(memset(buf2, 'x', 5), "xxxxx");
}

int main(void)
{
	// Serial connection
	serial_install();
	serial_print("\nConnected testing.\n");

	test_math();
	test_conv();
	test_mem();

	if (failed)
		log("%d tests failed\n", failed);
	else
		log("All tests passed\n");

	// Try emulator shutdown
	outw(0xB004, 0x2000);
	outw(0x604, 0x2000);
	outw(0x4004, 0x3400);

	loop();
	return 0;
}
