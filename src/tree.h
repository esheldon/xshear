#ifndef _TREE_HEADER
#define _TREE_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "vector.h"

// recursive, so need to wait on typedef
struct tree_node {
    int64_t val;
    szvector* indices;
    struct tree_node* right, * left;
};

typedef struct tree_node TreeNode;

void tree_insert(TreeNode ** self, int64_t val, size_t index);
TreeNode* tree_find(TreeNode* self, int64_t val);


void tree_print( TreeNode *self, int level );
void tree_print_padding( char ch, int n );

TreeNode* tree_free(TreeNode* self);

size_t tree_most_members( TreeNode *self );
#endif
