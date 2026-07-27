/*
 * tests/list_test.c
 *
 * Unit tests for src/list.c
 * Run: make -C tests check
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/list.h"

/* ---------- minimal test harness ---------- */

static int _pass = 0, _fail = 0;

#define ASSERT(cond) do { \
	if (cond) { \
		printf("  PASS: %s\n", #cond); \
		_pass++; \
	} else { \
		printf("  FAIL: %s  (line %d)\n", #cond, __LINE__); \
		_fail++; \
	} \
} while(0)

#define TEST(name) static void name(void); \
	static void run_##name(void) { printf("\n[%s]\n", #name); name(); } \
	static void name(void)

/* ---------- helpers ---------- */

static char *strdup_safe(const char *s)
{
	char *p = strdup(s);
	if (!p) { perror("strdup"); exit(1); }
	return p;
}

/* ---------- tests ---------- */

TEST(test_list_new)
{
	PMList *l = list_new();
	ASSERT(l != NULL);
	ASSERT(l->data == NULL);
	ASSERT(l->prev == NULL);
	ASSERT(l->next == NULL);
	free(l);
}

TEST(test_list_free_null)
{
	/* must not crash */
	list_free(NULL);
	ASSERT(1);
}

TEST(test_list_add_single)
{
	PMList *l = NULL;
	l = list_add(l, strdup_safe("hello"));
	ASSERT(l != NULL);
	ASSERT(l->data != NULL);
	ASSERT(strcmp((char *)l->data, "hello") == 0);
	ASSERT(list_count(l) == 1);
	list_free(l);
}

TEST(test_list_add_multiple)
{
	PMList *l = NULL;
	l = list_add(l, strdup_safe("a"));
	l = list_add(l, strdup_safe("b"));
	l = list_add(l, strdup_safe("c"));
	ASSERT(list_count(l) == 3);

	/* verify order */
	PMList *p = l;
	ASSERT(strcmp((char *)p->data, "a") == 0); p = p->next;
	ASSERT(strcmp((char *)p->data, "b") == 0); p = p->next;
	ASSERT(strcmp((char *)p->data, "c") == 0);
	ASSERT(p->next == NULL);

	list_free(l);
}

TEST(test_list_count_empty)
{
	/* A freshly allocated empty node counts as 1 element; NULL counts 0 */
	ASSERT(list_count(NULL) == 0);
	PMList *l = list_new();
	ASSERT(list_count(l) == 1); /* empty sentinel node */
	free(l);
}

TEST(test_list_last)
{
	PMList *l = NULL;
	ASSERT(list_last(NULL) == NULL);

	l = list_add(l, strdup_safe("x"));
	l = list_add(l, strdup_safe("y"));
	PMList *last = list_last(l);
	ASSERT(strcmp((char *)last->data, "y") == 0);
	list_free(l);
}

TEST(test_list_isin)
{
	char *a = strdup_safe("alpha");
	char *b = strdup_safe("beta");
	char *c = strdup_safe("gamma");

	PMList *l = NULL;
	l = list_add(l, a);
	l = list_add(l, b);

	ASSERT(list_isin(l, a) == 1);
	ASSERT(list_isin(l, b) == 1);
	ASSERT(list_isin(l, c) == 0);
	ASSERT(list_isin(NULL, a) == 0);

	list_free(l);
	free(c);
}

TEST(test_is_in)
{
	PMList *l = NULL;
	l = list_add(l, strdup_safe("foo"));
	l = list_add(l, strdup_safe("bar"));

	ASSERT(is_in("foo", l) == 1);
	ASSERT(is_in("bar", l) == 1);
	ASSERT(is_in("baz", l) == 0);
	ASSERT(is_in("foo", NULL) == 0);

	list_free(l);
}

TEST(test_list_remove_middle)
{
	char *a = strdup_safe("a");
	char *b = strdup_safe("b");
	char *c = strdup_safe("c");

	PMList *l = NULL;
	l = list_add(l, a);
	l = list_add(l, b);
	l = list_add(l, c);

	l = list_remove(l, b);
	free(b); /* b's node was freed by list_remove */

	ASSERT(list_count(l) == 2);
	ASSERT(list_isin(l, a) == 1);
	ASSERT(list_isin(l, c) == 1);

	/* verify linkage */
	ASSERT(l->next != NULL);
	ASSERT(l->next->next == NULL);
	ASSERT(l->next->prev == l);

	list_free(l);
}

TEST(test_list_remove_head)
{
	char *a = strdup_safe("a");
	char *b = strdup_safe("b");

	PMList *l = NULL;
	l = list_add(l, a);
	l = list_add(l, b);

	PMList *new_head = list_remove(l, a);
	free(a);

	ASSERT(new_head != NULL);
	ASSERT(strcmp((char *)new_head->data, "b") == 0);
	ASSERT(new_head->prev == NULL);

	list_free(new_head);
}

TEST(test_list_remove_tail)
{
	char *a = strdup_safe("a");
	char *b = strdup_safe("b");

	PMList *l = NULL;
	l = list_add(l, a);
	l = list_add(l, b);

	l = list_remove(l, b);
	free(b);

	ASSERT(list_count(l) == 1);
	ASSERT(strcmp((char *)l->data, "a") == 0);
	ASSERT(l->next == NULL);

	list_free(l);
}

TEST(test_list_merge_basic)
{
	PMList *one = NULL, *two = NULL;
	one = list_add(one, strdup_safe("1"));
	one = list_add(one, strdup_safe("2"));
	two = list_add(two, strdup_safe("3"));

	PMList *merged = list_merge(one, two);
	ASSERT(list_count(merged) == 3);

	/* two's data pointers are now owned by merged; free two's nodes only */
	list_free(two);
	list_free(merged);
}

TEST(test_list_merge_null_two)
{
	PMList *l = NULL;
	l = list_add(l, strdup_safe("only"));

	PMList *result = list_merge(l, NULL);
	ASSERT(result == l);
	ASSERT(list_count(result) == 1);
	list_free(result);
}

TEST(test_list_merge_null_one)
{
	PMList *two = NULL;
	two = list_add(two, strdup_safe("x"));

	PMList *result = list_merge(NULL, two);
	ASSERT(result != NULL);
	ASSERT(list_count(result) == 1);

	list_free(two);   /* data was moved, nodes still need freeing */
	list_free(result);
}

TEST(test_list_sort)
{
	PMList *l = NULL;
	l = list_add(l, strdup_safe("banana"));
	l = list_add(l, strdup_safe("apple"));
	l = list_add(l, strdup_safe("cherry"));

	PMList *sorted = list_sort(l);
	list_free(l); /* original list freed separately */

	ASSERT(sorted != NULL);
	ASSERT(list_count(sorted) == 3);
	ASSERT(strcmp((char *)sorted->data, "apple") == 0);
	ASSERT(strcmp((char *)sorted->next->data, "banana") == 0);
	ASSERT(strcmp((char *)sorted->next->next->data, "cherry") == 0);
	list_free(sorted);
}

TEST(test_list_sort_null)
{
	ASSERT(list_sort(NULL) == NULL);
}

TEST(test_list_strcmp_helper)
{
	const char *a = "aaa";
	const char *b = "bbb";
	ASSERT(list_strcmp(&a, &b) < 0);
	ASSERT(list_strcmp(&b, &a) > 0);
	ASSERT(list_strcmp(&a, &a) == 0);
}

/* ---------- main ---------- */

int main(void)
{
	run_test_list_new();
	run_test_list_free_null();
	run_test_list_add_single();
	run_test_list_add_multiple();
	run_test_list_count_empty();
	run_test_list_last();
	run_test_list_isin();
	run_test_is_in();
	run_test_list_remove_middle();
	run_test_list_remove_head();
	run_test_list_remove_tail();
	run_test_list_merge_basic();
	run_test_list_merge_null_two();
	run_test_list_merge_null_one();
	run_test_list_sort();
	run_test_list_sort_null();
	run_test_list_strcmp_helper();

	printf("\n============================\n");
	printf("Results: %d passed, %d failed\n", _pass, _fail);
	printf("============================\n");
	return _fail > 0 ? 1 : 0;
}
