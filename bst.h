/* bst.h 
 * TODO 
 * ECE 2230
 * MP 5
 *
 * TODO: Interface and tree definition for basic binary tree
 */

enum balanceoptions {BST, AVL};

#define TRUE 1
#define FALSE 0

typedef void *data_t;
typedef int bst_key_t;

typedef struct bst_node_tag {
    data_t data_ptr;
    bst_key_t key;
    int height;
    struct bst_node_tag *left;
    struct bst_node_tag *right;
} bst_node_t;

typedef struct bst_tag {
    bst_node_t *root;
    int size;       // number of keys in tree
    int policy;     // must be BST or AVL
} bst_t;


/* prototype definitions for functions in bst.c */
data_t bst_access(bst_t *bst_ptr, bst_key_t key);
bst_t *bst_construct(int);
void bst_destruct(bst_t *bst_ptr);
int bst_insert(bst_t *bst_ptr, bst_key_t key, data_t elem_ptr);

int bst_size(bst_t *bst_ptr);
int bst_int_path_len(bst_t *);
void bst_debug_print_tree(bst_t *bst_ptr);
void bst_debug_validate(bst_t *T);

/* vi:set ts=8 sts=4 sw=4: */
