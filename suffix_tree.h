#ifndef SUFFIX_TREE_H
#define SUFFIX_TREE_H

typedef struct {
	const char *str;
	int len;
} suffix_tree_string;

struct suffix_tree;

typedef struct suffix_tree suffix_tree;

suffix_tree *suffix_tree_create_single(const char *string, int len);

suffix_tree *suffix_tree_create(const char *strings[], int n_strings);

suffix_tree *suffix_tree_create2(suffix_tree_string strings[], int n_strings);

void suffix_tree_destroy(suffix_tree *tree);

const char *suffix_tree_search(suffix_tree *tree, const char *pattern, int len);

#endif	/* suffix_tree.h */
