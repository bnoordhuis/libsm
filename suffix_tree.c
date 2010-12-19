#include "suffix_tree.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct suffix_tree_node {
	struct suffix_tree_node *next_sibling;
	struct suffix_tree_node *first_child;
	suffix_tree_string suffix;
} suffix_tree_node;

struct suffix_tree {
	suffix_tree_node *root;
};

static suffix_tree_node *new_suffix_tree_node(const char *str, int len);

static void destroy_suffix_tree_node(suffix_tree_node *node);

static int has_children(suffix_tree_node *node);

static int add_node(suffix_tree *tree, suffix_tree_node *node);

static int add_node2(suffix_tree_node *walk, suffix_tree_node *node);

static void add_child_node(suffix_tree_node *parent, suffix_tree_node *child);

static int find_split_point(suffix_tree_string *haystack, suffix_tree_string *needle);

static int split_node(suffix_tree_node *parent, suffix_tree_node *child0, int offset);

suffix_tree *suffix_tree_create(const char *strings[], int n_strings) {
	suffix_tree_string *real_strings;
	suffix_tree *tree;
	int i;

	if (n_strings == -1) {
		while (strings[++n_strings] != NULL);
	}

	if (n_strings == 0) {
		return suffix_tree_create2(NULL, 0);
	}

	real_strings = malloc(sizeof(*real_strings) * n_strings);

	if (real_strings == NULL) {
		return NULL;
	}

	for (i = 0; i < n_strings; i++) {
		real_strings[i].str = strings[i];
		real_strings[i].len = strlen(strings[i]);
	}

	tree = suffix_tree_create2(real_strings, n_strings);

	free(real_strings);

	return tree;
}

suffix_tree *suffix_tree_create2(suffix_tree_string strings[], int n_strings) {
	suffix_tree_node *node;
	suffix_tree_string *s;
	suffix_tree *tree;
	int i, k;

	tree = malloc(sizeof(*tree));

	if (tree == NULL) {
		return NULL;
	}

	tree->root = NULL;

	if (n_strings == -1) {
		while (strings[++n_strings].str != NULL);
	}

	if (n_strings == 0) {
		return tree;
	}

	assert(n_strings == 1);

	for (i = 0; i < n_strings; i++) {
		s = strings + i;

		/* while suffixes remain:
		 *   add next shortest suffix to tree
		 */
		for (k = 0; k < s->len; k++) {
			node = new_suffix_tree_node(s->str + k, s->len - k);

			if (node == NULL) {
				suffix_tree_destroy(tree);
				return NULL;
			}

			if (!add_node(tree, node)) {
				suffix_tree_destroy(tree);
				return NULL;
			}
		}
	}

	return tree;
}

void suffix_tree_destroy(suffix_tree *tree) {
	if (tree == NULL) {
		return;
	}

	destroy_suffix_tree_node(tree->root);

	free(tree);
}

static void destroy_suffix_tree_node(suffix_tree_node *node) {
	if (node == NULL) {
		return;
	}

	destroy_suffix_tree_node(node->first_child);
	destroy_suffix_tree_node(node->next_sibling);

	free(node);
}

static suffix_tree_node *new_suffix_tree_node(const char *str, int len) {
	suffix_tree_node *node;

	node = malloc(sizeof(*node));

	if (node != NULL) {
		node->next_sibling = NULL;
		node->first_child = NULL;

		node->suffix.str = str;
		node->suffix.len = len;
	}

	return node;
}

static int add_node(suffix_tree *tree, suffix_tree_node *node) {
	if (tree->root == NULL) {
		tree->root = node;
		return 1;
	}

	return add_node2(tree->root, node);
}

static int add_node2(suffix_tree_node *walk, suffix_tree_node *node) {
	int offset;

	assert(walk != NULL);
	assert(node != NULL);

	/* note: offset is measured from end of string */
	offset = find_split_point(&walk->suffix, &node->suffix);

	/* suffices have no common prefix, e.g. walk='papua' and node='apua' */
	if (offset == 0) {
		if (walk->next_sibling == NULL) {
			walk->next_sibling = node;
			return 1;
		}

		return add_node2(walk->next_sibling, node);
	}

	/* walk.suffix and node.suffix are identical */
	if (offset == walk->suffix.len && offset == node->suffix.len) {
		return 1;
	}

	/* walk='papua', node='pua' - split in walk='p', child[0]='apua', child[1]='ua' */
	if (node->suffix.len < walk->suffix.len) {
		return split_node(walk, node, offset);
	}

	/* walk='p', node='pua' */
	node->suffix.str += walk->suffix.len;
	node->suffix.len -= walk->suffix.len;

	if (node->suffix.len == 0) {
		destroy_suffix_tree_node(node);
		return 1;
	}

	if (!has_children(walk)) {
		add_child_node(walk, node);
		return 1;
	}

	return add_node2(walk->first_child, node);
}

static int find_split_point(suffix_tree_string *haystack, suffix_tree_string *needle) {
	int k, len;

	assert(haystack != NULL);
	assert(haystack->str != NULL);
	assert(haystack->len != 0);

	assert(needle != NULL);
	assert(needle->str != NULL);
	assert(needle->len != 0);

	k = 0;

	/* len = min(haystack->len, needle->len) */
	len = haystack->len < needle->len ? haystack->len : needle->len;

	while (k < len
		&& haystack->str[k] == needle->str[k])
	{
		k++;
	}

	return k;
}

static int has_children(suffix_tree_node *node) {
	return node->first_child != NULL;
}

static void add_child_node(suffix_tree_node *parent, suffix_tree_node *child) {
	assert(parent != NULL);

	assert(child != NULL);
	assert(child->next_sibling == NULL);

	child->next_sibling = parent->first_child;

	parent->first_child = child;
}

static int split_node(suffix_tree_node *parent, suffix_tree_node *child0, int offset) {
	suffix_tree_node *child1;

	assert(parent != NULL);
	assert(child0 != NULL);

	assert(offset > 0);
	assert(offset < parent->suffix.len);

	assert(parent->suffix.len > 1);
	assert(child0->suffix.len > 0);

	/* don't split if the child is a single-character string */
	if (child0->suffix.len == 1) {
		add_child_node(parent, child0);
		return 1;
	}

	child1 = new_suffix_tree_node(parent->suffix.str + offset, parent->suffix.len - offset);

	if (child1 == NULL) {
		return 0;
	}

	child0->suffix.str += offset;
	child0->suffix.len -= offset;

	parent->suffix.len = offset;

	assert(parent->suffix.len > 0);
	assert(child0->suffix.len > 0);
	assert(child1->suffix.len > 0);

	add_child_node(parent, child0);
	add_child_node(parent, child1);

	return 1;
}

#include <stdio.h>

int suffix_tree_node_dump(suffix_tree_node *node, FILE *stream, int level) {
	int i, n;

	if (node == NULL) {
		return 0;
	}

	for (i = 0; i < level; i++) {
		putc('-', stream);
	}

	fwrite(node->suffix.str, 1, node->suffix.len, stream);

	putc('\n', stream);

	n = 1;

	n += suffix_tree_node_dump(node->first_child, stream, level + 1);

	n += suffix_tree_node_dump(node->next_sibling, stream, level);

	return n;
}

void suffix_tree_dump(suffix_tree *tree, FILE *stream) {
	int n;

	assert(tree != NULL);

	n = suffix_tree_node_dump(tree->root, stream, 0);

	printf("*** %d nodes in tree.\n", n);
}

#define RUN_UNIT_TESTS

#ifdef RUN_UNIT_TESTS

int main(void) {
	{
		suffix_tree_string a = { "papua", 5 };
		suffix_tree_string b = { "pua", 3 };
		int n = find_split_point(&a, &b);
		assert(n == 1);
	}

	{
		suffix_tree_string a = { "xyzzy", 5 };
		suffix_tree_string b = { "foo", 3 };
		int n = find_split_point(&a, &b);
		assert(n == 0);
	}

	{
		suffix_tree_string a = { "xyzzy", 5 };
		suffix_tree_string b = { "yyy", 3 };
		int n = find_split_point(&a, &b);
		assert(n == 0);
	}

	{
		suffix_tree_string a = { "xyzzy", 5 };
		suffix_tree_string b = { "xxx", 3 };
		int n = find_split_point(&a, &b);
		assert(n == 1);
	}

	{
		suffix_tree_string a = { "papua", 5 };
		suffix_tree_string b = { "papa", 4 };
		int n = find_split_point(&a, &b);
		assert(n == 3);
	}

	{
		suffix_tree_string s = { "papua", 5 };
		suffix_tree *tree = suffix_tree_create2(&s, 1);

		suffix_tree_dump(tree, stdout);
		suffix_tree_destroy(tree);
	}

	{
		suffix_tree_string s = { "mississippi", 11 };
		suffix_tree *tree = suffix_tree_create2(&s, 1);

		suffix_tree_dump(tree, stdout);
		suffix_tree_destroy(tree);
	}

	{
		static const char input[] = "Constructing such a tree for the string S takes time and space linear in the length of S. Once constructed, several operations can be performed quickly, for instance locating a substring in S, locating a substring if a certain number of mistakes are allowed, locating matches for a regular expression pattern etc. Suffix trees also provided one of the first linear-time solutions for the longest common substring problem. These speedups come at a cost: storing a string's suffix tree typically requires significantly more space than storing the string itself.";
		suffix_tree_string s = { input, sizeof(input) - 1 };
		suffix_tree *tree = suffix_tree_create2(&s, 1);

		suffix_tree_dump(tree, stdout);
		suffix_tree_destroy(tree);
	}
	return 0;
}

#endif	/* RUN_UNIT_TESTS */
