/* lab5.c 
 * Lab5: Binary Search Trees
 * ECE 2230
 *
 * This file contains drivers to test the BST package.
 *
* Unit drivers:
 *   -u 0: duplicate key replacement
 *   -u 1: LL rotation case
 *   -u 2: RR rotation case
 *   -u 3: LR rotation case
 *   -u 4: RL rotation case
 *   -u 5: skewed insertion order
 *   -u 6: mixed insert/access test
 *   -u 7: rotation stress + duplicate replacement (additional test)
 *
 * Access drivers build a tree using bst_insert and then access
 * keys in the tree using bst_access.
 *   -o run the driver with an optimal tree
 *   -r run the driver with a randomly generated tree
 *   -p run the driver with a poor order for inserting keys 
 *   -w to set the number of levels in the initial tree
 *   -t to set the number of access trials 
 *
  * Tree policy:
 *   -f bst
 *   -f avl
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#include "bst.h"

/* constants used with Global variables */
#define TRUE 1
#define FALSE 0

/* Global variables for command line parameters */
int SearchPolicy = BST;
int Seed = 1246772467;
int Verbose = FALSE; // controls how much detail the program prints
int OptimalTest = FALSE;
int RandomTest = FALSE;
int PoorTest = FALSE;
int UnitNumber = -1;
int Levels = 16;
int Trials = 100000; //number of search operations performed

enum testtypes {OPTIMAL, RANDOM, POOR};


/* prototypes for functions in this file only */
void getCommandLine(int argc, char **argv);
void accessDriver(int);
void unitDriver(const int ins_keys[], const int num_inserts,
                const int acc_keys[], const int num_accesses,
                const int exp_found[], const int num_expected,
                const char *test_name);

void build_optimal(bst_t *T, int levels);
void build_random(bst_t *T, int nodes);
void build_poor(bst_t *T, int levels);

int main(int argc, char **argv)
{
    getCommandLine(argc, argv);
    printf("Seed: %d\n", Seed);
    srand48(Seed);

    /* ----- unit tests ----- */
    if (UnitNumber == 0) {
        /* duplicate replacement */
        const int ins[] = {10, 10};
        const int acc[] = {10, 5};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   NULL, 0,
                   "Duplicate key replacement");
    }

    if (UnitNumber == 1) {
        /* LL rotation case for AVL: 30,20,10 */
        const int ins[] = {30, 20, 10};
        const int acc[] = {30, 20, 10, 25};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   NULL, 0,
                   "LL rotation case");
    }

    if (UnitNumber == 2) {
        /* RR rotation case for AVL: 10,20,30 */
        const int ins[] = {10, 20, 30};
        const int acc[] = {10, 20, 30, 25};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   NULL, 0,
                   "RR rotation case");
    }

    if (UnitNumber == 3) {
        /* LR rotation case for AVL: 30,10,20 */
        const int ins[] = {30, 10, 20};
        const int acc[] = {30, 10, 20, 15};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   NULL, 0,
                   "LR rotation case");
    }

    if (UnitNumber == 4) {
        /* RL rotation case for AVL: 10,30,20 */
        const int ins[] = {10, 30, 20};
        const int acc[] = {10, 30, 20, 25};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   NULL, 0,
                   "RL rotation case");
    }

    if (UnitNumber == 5) {
        /* skewed insertion order */
        const int ins[] = {10, 20, 30, 40, 50, 60};
        const int acc[] = {10, 30, 60, 35};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   NULL, 0,
                   "Skewed insertion order");
    }

    if (UnitNumber == 6) {
        /* mixed insert/access test */
        const int ins[] = {50, 30, 70, 20, 40, 60, 80,
            10, 25, 35, 45, 55, 65, 75, 85
        };
        const int acc[] = {50, 10, 45, 65, 90, 33};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   NULL, 0,
                   "Mixed insert/access test");
    }

    if (UnitNumber == 7) {
        /* Additional unit test:
         * Stress test combining several rotation cases and a duplicate
         * key replacement.  When run with -f avl this exercises LL, RR,
         * LR, and RL rotations in succession; when run with -f bst it
         * verifies ordered structure for a non-trivial sequence.
         *
         * exp_found marks which access keys should be present (1) or
         * absent (0). The duplicate insert (key 50, value 999) is
         * verified by the access of key 50 returning the replaced value.
         */
        const int ins[] = {50, 25, 75, 10, 30, 60, 80,
                           5, 15, 27, 55, 50};
        const int acc[] = {50, 25, 75, 5, 27, 80, 100, 0, 55};
        const int exp[] = {1,  1,  1,  1, 1,  1,  0,   0, 1};
        unitDriver(ins, sizeof ins / sizeof(int),
                   acc, sizeof acc / sizeof(int),
                   exp, sizeof exp / sizeof(int),
                   "Rotation stress + duplicate replacement");
    }

    /* ----- large tree tests  ----- */
    if (OptimalTest)                       /* enable with -o flag */
        accessDriver(OPTIMAL);
    if (RandomTest)                        /* enable with -r flag */
        accessDriver(RANDOM);
    if (PoorTest)                          /* enable with -p flag */
        accessDriver(POOR);

    return 0;
}

/* unit driver to build a custom tree and test access behavior
 *
 * ins_keys - array with the list of keys to insert into the tree
 * num_inserts - number of keys to insert
 *
 * acc_keys - array with the list of keys to access
 * num_accesses - number of keys to access
 *
 * exp_found - optional array indicating whether each access should succeed
 *             use 1 for found and 0 for not found
 *             if NULL, access results are printed but not asserted
 *
 * test_name - label printed for the unit test
 */
void unitDriver(const int ins_keys[], const int num_inserts,
                const int acc_keys[], const int num_accesses,
                const int exp_found[], const int num_expected,
                const char *test_name)
{
    int i;
    int *ip;
    bst_t *T;
    data_t dp;

    T = bst_construct(SearchPolicy);

    printf("\n\n====== Unit Driver: %s ======\n\n", test_name);
    printf("Tree policy: %s\n", SearchPolicy == BST ? "BST" : "AVL");

    printf("Inserting %d items into tree\n", num_inserts);
    for (i = 0; i < num_inserts; i++) {
        ip = (int *) malloc(sizeof(int));
        assert(ip != NULL);
        *ip = ins_keys[i];
        bst_insert(T, ins_keys[i], ip);
    }

    printf("Tree after inserts:\n");
    bst_debug_print_tree(T);

    printf("Validating tree...");
    bst_debug_validate(T);
    printf("passed\n");

    printf("Tree size = %d\n", bst_size(T));
    printf("Internal path length = %d\n", bst_int_path_len(T));

    printf("\nAccess tests (%d total)\n", num_accesses);
    for (i = 0; i < num_accesses; i++) {
        dp = bst_access(T, acc_keys[i]);
        if (dp == NULL) {
            printf("  access(%d) -> NULL\n", acc_keys[i]);
            if (exp_found != NULL && i < num_expected)
                assert(exp_found[i] == 0);
        } else {
            printf("  access(%d) -> found value %d\n",
                   acc_keys[i], *(int *) dp);
            if (exp_found != NULL && i < num_expected)
                assert(exp_found[i] == 1);
        }
    }

    printf("\nFinal validation...");
    bst_debug_validate(T);
    printf("passed\n");

    /* remove and free all items from tree */
    bst_destruct(T);
    printf("====== End Unit Driver ======\n\n");

}


/* driver to build and test trees in a large scale.  Creates tree with half of keys
 * in tree and half missing.  Uses access to find random keys.  Note that this
 * algorithm does not delete keys from the tree.
 *
 * test_type - type of test (OPTIMAL, RANDOM, POOR)
 */
void accessDriver(int test_type)
{
    int i;
    int range_num_ints;
    int size;
    int key;
    int ipl;
    int suc_trials, unsuc_trials;
    bst_t *test_tree;
    data_t dp;
    clock_t start, end;

    /* print parameters for this test run */
    printf("\n----- Access driver -----\n");
    printf("  Access trials: %d\n", Trials);
    printf("  Levels for tree: %d\n", Levels);

    range_num_ints = pow(2, Levels);

    /* build tree.  Key range is twice size of tree and each key 
     * is either even or odd.
     */
    test_tree = bst_construct(SearchPolicy);
    printf("  Build");
    if (test_type == OPTIMAL) {
        printf(" optimal");
        build_optimal(test_tree, Levels);
    } else if (test_type == RANDOM) {
        printf(" random");
        build_random(test_tree, pow(2,Levels)-1);
    } else if (test_type == POOR) {
        printf(" poor");
        build_poor(test_tree, Levels);
    } else {
        printf("invalid option in access test?\n");
        exit(1);
    }
    printf(" tree with size=%d\n", range_num_ints-1);

    if (Verbose)
        bst_debug_print_tree(test_tree);

    size = bst_size(test_tree);
    assert(size == range_num_ints-1);

    printf("  Validating tree...");
    bst_debug_validate(test_tree);
    printf(" passed\n");

    if (Trials > 0) {
        suc_trials = unsuc_trials = 0;
        start = clock();
        for (i = 0; i < Trials; i++) {
            key = ((int) (drand48() * (range_num_ints * 2 - 1))) + 1;
            dp = bst_access(test_tree, key);
            if (dp == NULL) {
                unsuc_trials++;
            } else {
                suc_trials++;
            }
        }
        end = clock();

        assert(size == bst_size(test_tree));
        ipl = bst_int_path_len(test_tree);

        printf("  After access exercise, time=%g ms, tree size=%d\n",
               1000 * ((double)(end - start)) / CLOCKS_PER_SEC, size);
        printf("  Successful accesses: %d\n", suc_trials);
        printf("  Unsuccessful accesses: %d\n", unsuc_trials);
        printf("  Internal path length: %d\n", ipl);
    }

    /* remove and free all items from tree */
    bst_destruct(test_tree);
    printf("----- End of access driver -----\n\n");
}


/* create one node to be inserted in tree with an even key
 *
 * T - tree to insert into
 * key - key for the new node
 */
void build_one_node(bst_t *T, int key)
{
    int *np = (int *) malloc(sizeof(int));
    *np = -2*key;
    bst_insert(T, 2*key, np);
}

/* build a complete tree with the lowest level full.  The tree has 
 * 2^(levels)-1 nodes and is perfectly balanced.  The key range is twice the 
 * size of the tree and only the even keys are inserted in the tree.
 * 
 * T - tree to insert into
 * levels - number of levels in the tree
 */
void build_optimal(bst_t *T, int levels)
{
    int n, start, inc, i, j;
    n = pow(2, levels);
    for (i = 0; i<levels; i++) {
        start = n/pow(2, i + 1); inc = 2 * start;
        for (j = start; j < n; j += inc) {
            build_one_node(T, j);
        }
    }
}

/* build a random tree with n nodes.  The nodes are numbered 2 to 2n and are
 * inserted randomly with a uniform distribution.  Only the even keys are
 * inserted.  Use Knuth shuffle to create random permutation.
 *
 * T - tree to insert into
 * nodes - number of nodes to add
 */
void build_random(bst_t *T, int nodes)
{
    int *narray;
    int i, key, temp;

    narray = (int *) malloc(nodes*sizeof(int));

    for (i = 0; i<nodes; i++)
        narray[i] = i;

    for (i = 0; i<nodes; i++) {
        key = (int) (drand48() * (nodes - i)) + i;
        assert(i <= key && key < nodes);
        temp = narray[i]; 
        narray[i] = narray[key]; 
        narray[key] = temp;
        build_one_node(T, narray[i]+1);
    }

    free(narray);
}

/* build a tree with 2^(levels)-1 nodes.  The nodes are numbered 2 to 2n and are
 * inserted in a poor order.  Only the even keys are inserted.  
 * 
 * The number of sets is sets
 * The size of a set is set_size
 *
 * Keys in a set are ascending, decending, or zig-zag
 *
 * T - tree to insert into
 * levels - used to calcuate the nubmer of nodes
 */
void build_poor(bst_t *T, int levels)
{
    int n, start, inc, i, j;
    int low, high;

    n = pow(2, levels);
    for (i = 0; i<levels/2; i++) {
        start = n/pow(2, i + 1); inc = 2 * start;
        for (j = start; j < n; j += inc) {
	    build_one_node(T, j);
        }
    }

    int sets = pow(2, levels/2);
    int set_size = pow(2, levels - levels/2);

    for (i = 0; i < sets; i++) {
        low = i*set_size + 1; 
        high = (i+1)*set_size - 1;
        int type = (int) 4*drand48();
        
        // randomly select type of insertion pattern
        // ascending
        if (type == 0) {
            for (j = low; j <= high; j++) {
                build_one_node(T, j);
            }
        } else if (type == 1) {  // decending
	        for (j = high; j >= low; j--) {
                build_one_node(T, j);
            }
	    } else if (type == 2) {  // zig-zag, low first
	        for (j = 0; j < (set_size-1)/2; j++) {
                build_one_node(T, low + j);
                build_one_node(T, high - j);
            }
            if (j*2 != set_size-1) {
                build_one_node(T, low + j);
            }
        } else {  // zig-zag, high first
	        for (j = 0; j < (set_size-1)/2; j++) {
                build_one_node(T, high - j);
                build_one_node(T, low + j);
            }
            if (j*2 != set_size-1) {
                build_one_node(T, low + j);
            }
        }
    }
}



/* read in command line arguments and store in global variables for easy
 * access by other functions.
 *
 * argc - argument count
 * argv - argument string array
 *
 */
void getCommandLine(int argc, char **argv)
{
    /* optopt--if an unknown option character is found
     * optind--index of next element in argv 
     * optarg--argument for option that requires argument 
     * "x:" colon after x means argument required
     */
    int c;
    int index;

    while ((c = getopt(argc, argv, "w:t:s:f:voru:p")) != -1)
        switch(c) {
            case 'w': Levels = atoi(optarg);       break;
            case 't': Trials = atoi(optarg);       break;
            case 's': Seed = atoi(optarg);         break;
            case 'v': Verbose = TRUE;              break;
            case 'o': OptimalTest = TRUE;          break;
            case 'r': RandomTest = TRUE;           break;
            case 'p': PoorTest = TRUE;             break;
            case 'u': UnitNumber = atoi(optarg);   break;
            case 'f':
                if (strcmp(optarg, "bst") == 0)
                    SearchPolicy = BST;
                else if (strcmp(optarg, "avl") == 0)
                    SearchPolicy = AVL;
                else {
                    fprintf(stderr, "invalid search policy: %s\n", optarg);
                    exit(1);
                }
                break;
            case '?':
                if (isprint(optopt))
                    fprintf(stderr, "Unknown option %c.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
            default:
                printf("Lab5 command line options\n");
                printf("General options ---------\n");
                printf("  -v        turn on verbose prints (default off)\n");
                printf("  -s 123    seed for random number generator\n");
                printf("  -f bst|avl\n");
                printf("            Type of tree\n");
                printf("  -u N      run unit test N\n");
                printf("              0 duplicate key replacement\n");
                printf("              1 LL rotation case\n");
                printf("              2 RR rotation case\n");
                printf("              3 LR rotation case\n");
                printf("              4 RL rotation case\n");
                printf("              5 skewed insertion order\n");
                printf("              6 mixed insert/access test\n");
                printf("              7 rotation stress + duplicate replacement\n");
                printf("  -o        run access test driver with optimum tree\n");
                printf("  -r        run access test driver with random tree\n");
                printf("  -p        run driver with poor insertion order\n");
                printf("\nOptions for test driver ---------\n");
                printf("  -w 16     levels in tree for drivers\n");
                printf("  -t 50000  number of trials in drivers\n");
                exit(1);
        }
    for (index = optind; index < argc; index++)
        printf("Non-option argument %s\n", argv[index]);
}

/* vi:set ts=8 sts=4 sw=4 et: */
