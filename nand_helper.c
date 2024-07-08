#include "nand_helper.h"
#include "nand.h"

// The function creates a new node containing the indicated:
//  to_be_added (pointer to the gate to be inserted in the new list node),
//  index (gate input number to be inserted in the new list node).
// It is called by the add_node_to_nand function. The possible results
// of the function are:
//  pointer to the newly created node - if all is successful,
//  NULL - if a memory allocation error occurred.
static out_node* create_out_node(nand_t *to_be_added, unsigned index) {
  struct out_node *new_node =
      (struct out_node*) malloc(sizeof(struct out_node));
  if (new_node == NULL) {
    return NULL;
  }
  new_node->nand_pointer = to_be_added;
  new_node->place = index;
  new_node->next = NULL;

  return new_node;
}

// The function adds a new node to the end of the list that represents
// the set of gates connected to the output of the gate pointed to by
// the indicated:
//  to_be_added_to (pointer to the pointer to the gate).
// The new node is to be created containing the indicated:
//  to_be_added (pointer to the gate),
//  index (gate input number).
// It uses the helper function create_out_node. The possible results
// of the function are:
//  0 - if all is successful,
//  -1 - if a memory allocation error occurred.
int add_node_to_nand(nand_t *to_be_added_to,
                     nand_t *to_be_added,
                     unsigned index) {
  struct out_node *new_node = create_out_node(to_be_added, index);
  if (new_node == NULL) {
    return -1;
  }

  if (to_be_added_to->head_out == NULL) {
    to_be_added_to->head_out = new_node;
  } else {
    (to_be_added_to->tail_out)->next = new_node;
  }

  to_be_added_to->tail_out = new_node;
  to_be_added_to->counter_out++;

  return 0;
}

// The function removes the node containing the indicated gate pointer:
//  to_be_deleted (pointer to the gate, such that the node holding
//                 it is to be removed from the list),
// from the list that represents the set of all gates connected to the output
// of the gate pointed to by the other indicated gate pointer:
//  to_be_deleted_from (pointer pointing to the gate from which the node with
//                      the gate pointer to_be_deleted is to be removed from).
// The result of the function is:
//  void.
void delete_node_from_nand(nand_t *to_be_deleted_from, nand_t *to_be_deleted) {
  struct out_node *current = to_be_deleted_from->head_out;
  struct out_node *prev = NULL;

  while (current != NULL) {
    if (current->nand_pointer == to_be_deleted) {

      if (prev == NULL) {
        to_be_deleted_from->head_out = current->next;
        to_be_deleted_from->counter_out--;
        free(current);
        return;
      } else {
        prev->next = current->next;

        if (current->next == NULL) {
          to_be_deleted_from->tail_out = prev;
        }
        to_be_deleted_from->counter_out--;
        free(current);
        return;
      }
    }
    prev = current;
    current = current->next;
  }
}

// The function removes all nodes from list that represents the set of gates
// connected to the output of the gate pointed to by the indicated pointer:
//  to_be_deleted_from (pointer to the gate from whose list of gates connected
//                      to the output all nodes are to be removed).
// and releases all inputs corresponding to the indicated gate in all gates
// connected to it. Result of the function is:
//  void.
void delete_list_from_nand(nand_t *to_be_deleted_from) {
  struct out_node *current = to_be_deleted_from->head_out;
  struct out_node *next;

  while (current != NULL) {
    next = current->next;

    (current->nand_pointer)->counter_ocupied--;
    (current->nand_pointer)->set_in_content[current->place] = 0;
    (current->nand_pointer)->set_in[current->place] = NULL;

    free(current);
    current = next;
  }

  to_be_deleted_from->counter_out = 0;
  to_be_deleted_from->head_out = NULL;
  to_be_deleted_from->tail_out = NULL;
}

// The function creates a new node containing the indicated:
//  to_be_added (pointer to the gate to be inserted in the new list node).
// The possible results of the function are:
//  pointer to a newly created node - if all is successful,
//  NULL - if a memory allocation error occurred.
static record_node* create_record_node(nand_t *to_be_added) {
  struct record_node *new_node =
      (struct record_node*) malloc(sizeof(struct record_node));
  if (new_node == NULL) {
    return NULL;
  }
  new_node->nand_pointer = to_be_added;
  new_node->next = NULL;

  return new_node;
}

// The function adds a new node containing the indicated:
//  to_be_added (pointer to the gate),
// to the end of the list that represents the set of gates that have already
// been visited by the DFS algorithm. Additional parameters
// of the function are:
//  record_list_head - pointer to the pointer to the first node of the list
//                     containing pointers to gates that have been visited,
//  record_list_tail - pointer to the pointer to last node in the same list.
// It uses the helper function create_record_node. The possible results
// of the function are:
//  0 - if all is successful,
//  -1 - if a memory allocation error occurred.
int add_to_record(record_node **list_head,
                  record_node **list_tail,
                  nand_t *to_be_added) {
  struct record_node *new_node = create_record_node(to_be_added);
  if (new_node == NULL) {
    return -1;
  }

  if (*list_head == NULL) {
    *list_head = new_node;
  } else {
    (*list_tail)->next = new_node;
  }

  *list_tail = new_node;

  return 0;
}

// The function removes all nodes from the list that represents the set
// of gates that have already been visited by the DFS algorithm.
// The parameter of the function is:
//  record_list_head - pointer to the pointer to the first node of the list
//                     containing pointers to gates that have been visited,
// The result of the function is:
//  void.
void clear_record(record_node **head_record) {
  struct record_node *current = *head_record;
  struct record_node *next;

  while (current != NULL) {
    next = current->next;

    (current->nand_pointer)->record_created = false;
    (current->nand_pointer)->record_finished = false;
    (current->nand_pointer)->to_be_visited = 0;
    (current->nand_pointer)->record_found_false = false;
    (current->nand_pointer)->record_critical_path = 0;

    free(current);
    current = next;
  }
  *head_record = NULL;
}
