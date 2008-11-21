#include <stdio.h> 

/*
  cycle_detector - detects links that would create cycles in directed graphs of
  links between nodes. Works from the command line, prompting for and accepting
  space separated pairs of nodes in the form "start end". Nodes are either
  accepted (do not create cycles) or rejected. Reasons for rejection are "Cycle
  found" and "Bad data". Some minimal informative text is output for bad input
  data. Control-C to end program.

  Written in ANSI C by Ray Baxter (ray.baxter@gmail.com) in 2008 as solution to
  problem posed by persons who will remain anonymous. Compiled and tested on both
  Power PC and Intel Macintoshes.

  Problem statement:
 
  Imagine a graph that consists of directional links between nodes identified by
  small non-negative integers < 2**16. We define a "cycle" in the graph as a
  nonempty set of links that connect a node to itself.
 
  Imagine an application that allows insertion of links, but wants to prevent
  insertion of links that close cycles in the graph.
 
  For example, starting from an empty graph, inserting links 1->2 and 2->3 would
  succeed; but inserting a third link 3->1 would fail, since it would close the
  cycle 1->2->3->1. However, inserting a link 1->3 instead would succeed.
 
  In your favorite programming language, declare data structures to represent
  your graph, and give code for an "insert link" function that fails if a new
  link would close a cycle. What, roughly, is the space- and time-complexity of
  your solution?

  Description:
  
  Maintain for each node, those nodes that are its ancestors. Store information
  in a Boolean matrix where each row represents a node and each bit column the
  possible ancestors of that node on the graph.  When link i->j is presented, if
  node j is an ancestor of i (ancestors[i][j] is true), then creating link i->j
  would create a cycle, so reject the link. Otherwise, update the matrix so that
  node i and all its ancestors are ancestors of node j and its descendants. 

	This solution does not preserve any information about the actual links in the
	graph. For example, there is no change to the ancestors matrix when a link
	1->3 is added to an ancestors matrix that already contains links for
	1->2 and 2->3. In addition, there is no logic implemented here to
	prevent repeated insertion of identical links, so link 1->2 can be
	inserted multiple times. 

	Algorithm:

  When a new link, i->j is presented:

   If ancestors[i][j] is TRUE, return FAIL, otherwise update ancestors and
   return PASS. 
 
   When inserting the ancestors there are the two sets of links with respect to
   i and j:
 
   1) The set of links, H, which are ancestors of i. This is the set for which
      ancestors[i][h] is TRUE. This set includes the self-link, i->i, by
      definition.  

	 2) The set of links, K, which are descendants of j. This is the set for which
      ancestors[k][j] is TRUE. This set includes the self-link, j->j, by
      definition.
 
  Before linking i and j, either set H or set K may be empty (except for
  self-links) and H and K may have common members. 
 
  Inserting the link from i->j adds all H as ancestors of all K. This is
  equivalent to ancestors[k] |= ancestors[i] (an in-place logical OR of the row 
  vectors) for all values of k where ancestors[k][j] is true.

	This version of cycle_detector uses bit fields stored in #define'd type FIELD
	to store the individual ancestors. Thus, if node n is an ancestor of node i,
	then the true value indicating the relationship will be stored in FIELD
	n/FIELD_SIZE of row i of the ancestors matrix. Within that FIELD the bit
	containing the value will be the n % FIELD_SIZE bit. Bit twiddling is
	localized in the functions is_ancestor and set_ancestor and the single OR in
	insert_ancestors. 
 
  Complexity
 
  Space complexity - the solution given below requires O(N^2) bits for the
  ancestors matrix. 
 
  Time complexity - failing insertions take constant time, a single
  lookup. Insertion time depends on the richness of the interconnections in the
  graph. If C is the average number of descendants per node, then the
  time-complexity of each insertion is O(CN). The bounds on C are from 1 to
  1/2N, rising as the number of linked nodes. For a sparsely connected graph the
  time of each insertion is proportional to N. For a richly connected graph,
  each insertion would be propagated to about half of other nodes. Each
  successful insertion would take about 1/2 N^2. Successful insertions would 
  become more rare as more links were made, so effectively the average
  time of insertion would be maximal when about half of the nodes were
  connected. This is O(N^2) complexity. It's worth noting that this
  solution is implicitly parallel, operating on multiple bits
  simultaneously, so that on 64 bit hardware, the OR's would require
  10^10 operations instead of 10^16.
	
  Data Structures
  
  ancestors - N x N matrix of bits that represents links that if inserted, will
  create cycles. If ancestors[i][j] is true, then link from a node i to a node j
  will create a cycle in the graph, and is not permitted.

  Declaration depends on definition of constants TOTAL_NODES and
  FIELDS_PER_NODE. Some hardware may not be able to allocate a single block of
  2^32 bits. In that case, instead of a single matrix, solution would require an
  array of TOTAL_NODES pointers to per node bit vectors. These vectors could be
  allocated either on the fly as new nodes were added, or at
  initialization. This code is not illustrated here.

  TOTAL_NODES - a constant - is the maximum possible number of nodes. Code has
  only been tested for values of TOTAL_NODES that are powers of 2 and an
  integral multiple of FIELD_SIZE.  
 
  FIELD - a type for bit fields. The optimal type depends on hardware, but
  should be unsigned. You can't go wrong with unsigned char, short or int
  but depending on hardware performance will vary, as the predominating
  operation is an OR on these FIELDS. Larger sized FIELDs will require
  proportionally fewer operations.

*/

#define TOTAL_NODES 65536  /* 2^16 */

#define FIELD           unsigned long
#define FIELD_SIZE      (sizeof(FIELD)*8)
#define FIELDS_PER_NODE (TOTAL_NODES/FIELD_SIZE)

/* Declare ancestors matrix */

FIELD ancestors[TOTAL_NODES][FIELDS_PER_NODE];

/* Return values for insert_link function */

#define FAIL 0
#define PASS 1
#define BAD_DATA 2

/* Boolean Values */

#define TRUE 1
#define FALSE 0

/* Function Prototypes */

int  insert_link(int starting_node, int ending_node); 
void insert_ancestors(int starting_node, int ending_node);
int  is_ancestor(int n_node, int n_ancestor);
void set_ancestor(int n_descendant, int n_ancestor);
void initialize_ancestors(); 

int insert_link(int start_node, int end_node) 
{
	if (start_node >= TOTAL_NODES) {
		printf("input ignored: ");
		printf("start (= %d) must be less than TOTAL_NODES (= %d)\n", start_node, TOTAL_NODES);
		return BAD_DATA;
  } else if (end_node >= TOTAL_NODES) {
		printf("input ignored: ");
		printf("end (= %d) must be less than TOTAL_NODES (= %d)\n", end_node, TOTAL_NODES);
		return BAD_DATA;
	} else if (start_node == end_node) {
		printf("input ignored: ");
		printf("start and end are identical (= %d)\n", end_node);
		return FAIL; /* This is a fail by definition */
	} else if (is_ancestor(start_node,end_node)) {
		return FAIL;
	} else {
		insert_ancestors(start_node,end_node);
		return PASS;
	}
}

void insert_ancestors(int start_node, int end_node) 
{
	/* The meat of the program. Refer to Algorithm section above. */
	int k, n_field;
	
	for(k=0;k<TOTAL_NODES;k++)
		{
			if (is_ancestor(k,end_node))
				{
					for(n_field=0;n_field<FIELDS_PER_NODE;n_field++)
						{
							ancestors[k][n_field] |= ancestors[start_node][n_field];
						}
				}
 		}
	return;
}

void initialize_ancestors() 
{
	/* By definition all self links (x->x) are closing links. */
	int i;
	for(i=0;i<TOTAL_NODES;i++)
		{
			set_ancestor(i,i);
		}
}

int is_ancestor(int n_node, int n_ancestor) 
{
	int n_target_chunk = n_ancestor / FIELD_SIZE;
	int n_target_bit = n_ancestor % FIELD_SIZE;
	FIELD bit_to_get = 1 << n_target_bit;

	if(ancestors[n_node][n_target_chunk] & bit_to_get)
		return(TRUE);
	else 
		return(FALSE);
}

void set_ancestor(int n_descendant, int n_ancestor) 
{
	int n_target_chunk = n_ancestor / FIELD_SIZE;
	int n_target_bit  = n_ancestor % FIELD_SIZE;
	int bit_to_set = 1 << n_target_bit;

	ancestors[n_descendant][n_target_chunk] |= bit_to_set;

	return;
}

int main () 
{
	int start_node, end_node, result;
	initialize_ancestors();
	while (TRUE) {
		printf ("Enter start end:  ");
		scanf ("%d %d", &start_node, &end_node);
		result = insert_link(start_node, end_node);
		if(result == FAIL) 
			{
				printf("Cycle found\n");
			}
		if(result == PASS)
			printf("Good insert\n");
		if(result == BAD_DATA)
			printf("Bad (out of bounds) data\n");
	}
}

