/* bst.c
 * ECE 2230
 * MP 5
 *
 * Implementation of tree interface for binary search tree (BST) and
 * AVL self-balancing binary search tree.
 *
 * Insertion policy:
 *   - BST: ordinary insertion at the leaf based on key comparison.
 *   - AVL: insertion followed by rebalancing using single/double rotations
 *          so that the height invariant |left - right| <= 1 holds at every node.
 *
 * Keys are unique. Inserting a duplicate key replaces the stored value
 * without creating a new node.
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include "bst.h"

#define MYMAX(a, b) (a > b ? a : b)

/* definitions for use in bst.c only */
static int bst_avl_insert(bst_t *T, bst_key_t key, data_t elem_ptr);
void ugly_print(bst_node_t *N, int level, int policy);
int bst_debug_validate_rec(bst_node_t *N, int min, int max, int *count);
int rec_height(bst_node_t *N);
int children(bst_node_t *N);
void pretty_print(bst_t *T);

/* internal helper prototypes */
static bst_node_t *new_node(bst_key_t key, data_t elem_ptr);
static void free_subtree(bst_node_t *N);
static int node_height(bst_node_t *N);
static void update_height(bst_node_t *N);
static int balance_factor(bst_node_t *N);
static bst_node_t *rotate_right(bst_node_t *N);
static bst_node_t *rotate_left(bst_node_t *N);
static bst_node_t *avl_insert_rec(bst_node_t *N, bst_key_t key,
                                  data_t elem_ptr, int *replace);
static int ipl_rec(bst_node_t *N, int depth);


/* Finds the tree element with the matching key and returns the data that is
 * stored in this node in the tree.
 *
 * T - tree to search in key - key to find inside T
 *
 * RETURNS pointer to the data stored in the found node or NULL if no match is
 * found
 */
data_t bst_access(bst_t *T, bst_key_t key)
{
    assert(T != NULL);
    bst_node_t *cur = T->root;
    while (cur != NULL) {
        if (key == cur->key)
            return cur->data_ptr;
        else if (key < cur->key)
            cur = cur->left;
        else
            cur = cur->right;
    }
    return NULL;
}

/* Creates the header block for the tree with the provided management policy,
 * and initializes it with default 'blank' data.
 *
 * tree_policy - tree management policy to use either AVL or BST.
 *
 * RETURNS pointer to the newly created tree
 */
bst_t *bst_construct(int tree_policy)
{
    assert(tree_policy == BST || tree_policy == AVL);
    bst_t *T = (bst_t *) malloc(sizeof(bst_t));
    assert(T != NULL);
    T->root = NULL;
    T->size = 0;
    T->policy = tree_policy;
    return T;
}


/* Free all items stored in the tree including the memory block with the data
 * and the bst_node_t structure.  Also frees the header block.
 *
 * T - tree to destroy
 */
void bst_destruct(bst_t *T)
{
    if (T == NULL)
        return;
    free_subtree(T->root);
    free(T);
}

/* Recursively free a subtree, including any data the nodes own. */
static void free_subtree(bst_node_t *N)
{
    if (N == NULL)
        return;
    free_subtree(N->left);
    free_subtree(N->right);
    if (N->data_ptr != NULL)
        free(N->data_ptr);
    free(N);
}

/* Allocate and initialize a new tree node. */
static bst_node_t *new_node(bst_key_t key, data_t elem_ptr)
{
    bst_node_t *N = (bst_node_t *) malloc(sizeof(bst_node_t));
    assert(N != NULL);
    N->data_ptr = elem_ptr;
    N->key = key;
    N->height = 1; /* leaf height */
    N->left = NULL;
    N->right = NULL;
    return N;
}

/* Insert data_t into the tree with the associated key.
 * This is the main insertion interface used by the drivers.
 *
 * If the tree policy is BST, perform ordinary BST insertion.
 * If the tree policy is AVL, call bst_avl_insert(...) so that
 * the tree is rebalanced as needed.
 *
 * T - tree to insert into
 * key - search key used to locate the insertion point
 * elem_ptr - data to be stored at the node associated with key
 *
 * RETURNS 0 if the key is already in the tree and the stored data is replaced,
 * and 1 if the key is not found and a new node is inserted.
 */
int bst_insert(bst_t *T, bst_key_t key, data_t elem_ptr)
{
    assert(T != NULL);

    /* If this tree uses AVL balancing, delegate the work to the
     * AVL-specific insertion function.
     */
    if (T->policy == AVL) {
        int result = bst_avl_insert(T, key, elem_ptr);
#ifdef VALIDATE
        bst_debug_validate(T);
#endif
        return result;
    }

    /* Ordinary BST insertion. */
    int result;
    if (T->root == NULL) {
        T->root = new_node(key, elem_ptr);
        T->size++;
        result = 1;
    } else {
        bst_node_t *cur = T->root;
        bst_node_t *parent = NULL;
        while (cur != NULL) {
            if (key == cur->key) {
                /* Replace stored value, free the old data block. */
                if (cur->data_ptr != NULL)
                    free(cur->data_ptr);
                cur->data_ptr = elem_ptr;
#ifdef VALIDATE
                bst_debug_validate(T);
#endif
                return 0;
            }
            parent = cur;
            if (key < cur->key)
                cur = cur->left;
            else
                cur = cur->right;
        }
        /* parent is the node where we attach the new leaf. */
        bst_node_t *fresh = new_node(key, elem_ptr);
        if (key < parent->key)
            parent->left = fresh;
        else
            parent->right = fresh;
        T->size++;
        result = 1;
    }

#ifdef VALIDATE
    bst_debug_validate(T);
#endif
    return result;
}

/* Insert data_t into the tree with the associated key. Insertion MUST
 * follow the tree's property AVL. This function should be called from
 * bst_insert for AVL tree's inserts.
 *
 * T - tree to insert into
 * key - search key to determine if key is in the tree
 * elem_ptr - data to be stored at tree node indicated by key
 *
 * RETURNS 0 if key is found and element is replaced, and 1 if key is not found
 * and element is inserted
 */
int bst_avl_insert(bst_t *T, bst_key_t key, data_t elem_ptr)
{
    int replace = 0;
    T->root = avl_insert_rec(T->root, key, elem_ptr, &replace);
    if (replace == 0)
        T->size++;
    return replace;
}

/* Height of a node, treating NULL as 0. */
static int node_height(bst_node_t *N)
{
    return (N == NULL) ? 0 : N->height;
}

/* Recompute and store this node's height from its children. */
static void update_height(bst_node_t *N)
{
    int lh = node_height(N->left);
    int rh = node_height(N->right);
    N->height = 1 + MYMAX(lh, rh);
}

/* Balance factor: height(left) - height(right). */
static int balance_factor(bst_node_t *N)
{
    return node_height(N->left) - node_height(N->right);
}

/* Right rotation around N. Returns the new subtree root. */
static bst_node_t *rotate_right(bst_node_t *N)
{
    bst_node_t *L = N->left;
    bst_node_t *LR = L->right;
    L->right = N;
    N->left = LR;
    update_height(N);
    update_height(L);
    return L;
}

/* Left rotation around N. Returns the new subtree root. */
static bst_node_t *rotate_left(bst_node_t *N)
{
    bst_node_t *R = N->right;
    bst_node_t *RL = R->left;
    R->left = N;
    N->right = RL;
    update_height(N);
    update_height(R);
    return R;
}

/* Recursive AVL insertion. Returns the (possibly new) root of the subtree.
 * On duplicate-key insertion, *replace is set to 1 and the data block is
 * swapped in place; the tree's structure does not change.
 */
static bst_node_t *avl_insert_rec(bst_node_t *N, bst_key_t key,
                                  data_t elem_ptr, int *replace)
{
    if (N == NULL)
        return new_node(key, elem_ptr);

    if (key == N->key) {
        if (N->data_ptr != NULL)
            free(N->data_ptr);
        N->data_ptr = elem_ptr;
        *replace = 1;
        return N;   /* no rebalancing required */
    }

    if (key < N->key)
        N->left = avl_insert_rec(N->left, key, elem_ptr, replace);
    else
        N->right = avl_insert_rec(N->right, key, elem_ptr, replace);

    /* If this was a duplicate replacement, the subtree shape didn't change. */
    if (*replace == 1)
        return N;

    update_height(N);
    int bf = balance_factor(N);

    /* Left-Left: insertion in left subtree of left child. */
    if (bf > 1 && key < N->left->key)
        return rotate_right(N);

    /* Right-Right: insertion in right subtree of right child. */
    if (bf < -1 && key > N->right->key)
        return rotate_left(N);

    /* Left-Right: insertion in right subtree of left child. */
    if (bf > 1 && key > N->left->key) {
        N->left = rotate_left(N->left);
        return rotate_right(N);
    }

    /* Right-Left: insertion in left subtree of right child. */
    if (bf < -1 && key < N->right->key) {
        N->right = rotate_right(N->right);
        return rotate_left(N);
    }

    return N;
}

/* RETURNS the number of keys in the tree */
int bst_size(bst_t *T)
{
    assert(T != NULL);
    return T->size;
}


/* RETURNS the computed internal path length of the tree T */
int bst_int_path_len(bst_t *T)
{
    assert(T != NULL);
    return ipl_rec(T->root, 0);
}

/* Sum of depths of all nodes in the subtree rooted at N. */
static int ipl_rec(bst_node_t *N, int depth)
{
    if (N == NULL)
        return 0;
    return depth
        + ipl_rec(N->left, depth + 1)
        + ipl_rec(N->right, depth + 1);
}


/* prints the tree T */
void bst_debug_print_tree(bst_t *T)
{
    if (T == NULL) {
        printf("(null tree)\n");
        return;
    }
    if (T->root == NULL) {
        printf("(empty tree)\n");
        return;
    }

    ugly_print(T->root, 0, T->policy);//XTRA
    printf("\n");
    if (T->size < 64)
    pretty_print(T);
}

/* basic print function for a binary tree
 *
 * N - node of tree to print
 * level - level in which the node resides
 * policy - BST or AVL
 */
void ugly_print(bst_node_t *N, int level, int policy)
{
    int i;
    if (N == NULL) return;
    ugly_print(N->right, level+1, policy);
    if (policy == AVL) {
	    for (i = 0; i<level; i++) printf("       ");
	        printf("%5d-%d\n", N->key, N->height);
    } else {
	    for (i = 0; i<level; i++) printf("     ");
	        printf("%5d\n", N->key);
    }
    ugly_print(N->left, level+1, policy);
}

/* Basic validation function for tree T */
void bst_debug_validate(bst_t *T)
{
    int size = 0;
    assert(bst_debug_validate_rec(T->root, INT_MIN, INT_MAX, &size) == TRUE);
    assert(size == T->size);
    if (T->policy == AVL)
	    rec_height(T->root);
}

/* A recursive validation function based on allowable key ranges
 *
 * N - current node to validate
 * min - lower bound for keys in this subtree
 * max - upper bound for keys in this subtree
 * count - number of visited nodes
 */
int bst_debug_validate_rec(bst_node_t *N, int min, int max, int *count)
{
    if (N == NULL)
        return TRUE;
    if (N->key <= min || N->key >= max)
        return FALSE;
    assert(N->data_ptr != NULL);
    *count += 1;
    return bst_debug_validate_rec(N->left, min, N->key, count) &&
        bst_debug_validate_rec(N->right, N->key, max, count);
}

/* Verifies AVL tree properties */
int rec_height(bst_node_t *N)
{
    if (N == NULL)
	    return 0;
    int lh = rec_height(N->left);
    int rh = rec_height(N->right);
    int lean = lh - rh;
    assert(lean == 0 || lean == 1 || lean == -1);
    return 1 + MYMAX(lh, rh);

}

/* Recursive function to count children */
int children(bst_node_t *N)
{
    if (N == NULL) return 0;
    return 1 + children(N->left) + children(N->right);
}

/* Prints the tree to the terminal in ASCII art*/
void pretty_print(bst_t *T)
{
    if (T == NULL || T->root == NULL || T->size == 0) {
        printf("(empty tree)\n");
        return;
    }

    typedef struct queue_tag {
	    bst_node_t *N;
	    int level;
	    int list_sum;
    } queue_t;

    queue_t Q[T->size];
    int q_head = 0;
    int q_tail = 0;
    int i, j;
    int current_level = 0;
    int col_cnt = 0;
    bst_node_t *N;

    Q[q_tail].N = T->root;
    Q[q_tail].level = 0;
    Q[q_tail].list_sum = 0;
    q_tail++;
    for (i = 0; i < T->size; i++)
    {
	assert(q_head < T->size);
	N = Q[q_head].N;
	if (Q[q_head].level > current_level) {
	    printf("\n");
	    current_level++;
	    col_cnt = 0;
	}
	int left_ch = children(N->left);
	int my_pos = 1 + left_ch + Q[q_head].list_sum;
	int left_child_pos = my_pos;
	if (N->left != NULL)
	    left_child_pos = 1 + Q[q_head].list_sum + children(N->left->left);
	int right_child_pos = my_pos;
	if (N->right != NULL)
	    right_child_pos = my_pos + 1 + children(N->right->left);
	for (j = col_cnt + 1; j <= right_child_pos; j++)
	{
	    if (j == my_pos)
		if (left_child_pos < my_pos)
		    if (N->key < 10)
			printf("--%d", N->key);
		    else if (N->key < 100)
			printf("-%d", N->key);
		    else
			printf("%d", N->key);
		else
		    printf("%3d", N->key);
	    else if (j == left_child_pos)
		//printf("  |");
		printf("  /");
	    else if (j > left_child_pos && j < my_pos)
		printf("---");
	    else if (j > my_pos && j < right_child_pos)
		printf("---");
	    else if (j == right_child_pos)
		//printf("--|");
		printf("-\\ ");
	    else
		printf("   ");
	}
	col_cnt = right_child_pos;

	if (N->left != NULL) {
	    Q[q_tail].N = N->left;
	    Q[q_tail].level = Q[q_head].level + 1;
	    Q[q_tail].list_sum = Q[q_head].list_sum;
	    q_tail++;
	}
	if (N->right != NULL) {
	    Q[q_tail].N = N->right;
	    Q[q_tail].level = Q[q_head].level + 1;
	    Q[q_tail].list_sum = Q[q_head].list_sum + left_ch + 1;
	    q_tail++;
	}
	q_head++;
    }
    printf("\n");
}

/* vi:set ts=8 sts=4 sw=4 et: */
