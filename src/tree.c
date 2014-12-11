#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "tree.h"


void tree_insert(TreeNode ** self, int64_t val, size_t index) {
    if(!(*self)) {
        *self = malloc(sizeof(TreeNode));
        (*self)->left = (*self)->right = NULL;
        (*self)->val=val;
        (*self)->indices = szvector_new();
        vector_push((*self)->indices, index);
        return;
    }
    if(val < (*self)->val) {
        tree_insert(&(*self)->left, val, index);
    } else if(val > (*self)->val) {
        tree_insert(&(*self)->right, val, index);
    } else {
        vector_push((*self)->indices, index);
    }
}

TreeNode* tree_free(TreeNode* self) {
    if (self != NULL) {
        vector_free(self->indices);
        self->left    = tree_free(self->left);
        self->right   = tree_free(self->right);
        free(self);
    }
    return NULL;
}

TreeNode* tree_find(TreeNode* self, int64_t val) {
    if (!self) {
        return NULL;
    }
    if(val < self->val) {
        return tree_find(self->left, val);
    } else if(val > self->val) {
        return tree_find(self->right, val);
    } else {
        return self;
    }
}

void tree_print_padding( char ch, int n )
{
    int i;

    for ( i = 0; i < n; i++ ) {
        fprintf(stderr,"%c", ch);
    }
}

// send level=0 at first
void tree_print( TreeNode *self, int level )
{
    if ( self == NULL ) {
        tree_print_padding ( '\t', level );
        fprintf(stderr,"~");
    } else {
        tree_print( self->right, level + 1 );
        tree_print_padding ( '\t', level );
        fprintf(stderr,"%ld\n", self->val);
        tree_print( self->left, level + 1 );
    }
}


size_t tree_most_members( TreeNode *self )
{
    size_t max_ind=0, max_ind_left=0, max_ind_right=0;
    if (self != NULL) {
        max_ind = self->indices->size;

        max_ind_left = tree_most_members( self->left );
        max_ind_right = tree_most_members( self->right );

        max_ind = (max_ind > max_ind_left) ? max_ind : max_ind_left;
        max_ind = (max_ind > max_ind_right) ? max_ind : max_ind_right;
    }
    return max_ind;
}




