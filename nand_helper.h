#ifndef NAND_HELPER
#define NAND_HELPER

#include "nand.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// The structure represents a node of a list that is associated with each
// nand gate to symbolize the set of all gates connected to its output.
// The fields of that structure are:
//  nand_pointer - pointer to a nand gate connected to the nand gate that
//                 is associated with the list,
//  place - number of input of the gate associated with the list that
//          the gate pointed at by nand_pointer occupies,
//  next - pointer to another nand gate connected to the nand gate that
//         is associated with the list.
typedef struct out_node {
  nand_t *nand_pointer;
  unsigned place;
  struct out_node *next;
} out_node;

// The structure represents a node of a list that is associated with each
// execution of nand_evaluate function to keep track of nand gates that were
// visited by it. The fields of that structure are:
//  nand_pointer - pointer to a nand gate that was visited in a current
//                 execution of nand_evaluate function, associated
//                 with the list,
//  next - pointer to another nand gate that was visited in a current
//         execution of nand_evaluate function, associated with the list.
typedef struct record_node {
  nand_t *nand_pointer;
  struct record_node *next;
} record_node;

// The structure represents a nand gate, its fields are:
//  counter_in - number of gate inputs (fixed),
//  counter_ocupied - number of gate inputs with boolean signal or nand gate
//                    output connected to them (variable),
//  set_in - a pointer to an array with pointers pointing at inputs
//           to the gate,
//  set_in_content - pointer to an array with a numerical description
//                   of contents placed in set_in array, those are;
//                   0 if the place at specified index in set_in array
//                   is occupied by an empty pointer, 1 if it is occupied
//                   by a pointer to a boolean signal or 2 if the place
//                   is occupied by a pointer to a nand gate,
//  counter_out - number of gate inputs connected to the output of a given
//                nand gate (variable),
//  head_out - pointer to the first node (out_node) of the list of containing
//             pointers to the gates connected to the output of the given
//             nand gate,
//  tail_out - pointer to the last node of the list same list,
//  to_be_visited - index of set_in that is to be checked next in the current
//                  execution of nand_evaluate function,
//  record_created - tells whether the gate has already been visited within
//                   current execution of nand_evaluate function,
//  record_finished - tells whether the values calculated by nand_evaluate
//                    function (boolean signal at the output and critical path)
//                    have already been determined for the gate within
//                    current execution of nand_evaluate function,
//  record_found_false - if the value held by record_created is true,
//                       record_found_false value tells whether any boolean
//                       signal with value false was found within current
//                       execution of nand_evaluate_recursion (execution for
//                       the given nand gate),
//  record_critical_path - if the value held by record_created is true,
//                         record_critical_path value is equal to the length
//                         of the maximum critical path found within current
//                         execution of nand_evaluate_recursion function
//                         (execution for the given nand gate).
struct nand {
  unsigned counter_in;
  unsigned counter_ocupied;
  void **set_in;
  int *set_in_content;
  ssize_t counter_out;
  out_node *head_out;
  out_node *tail_out;
  unsigned to_be_visited;
  bool record_created;
  bool record_finished;
  bool record_found_false;
  ssize_t  record_critical_path;
};

int  add_node_to_nand(nand_t *to_be_added_to, nand_t *to_be_added, unsigned index);
void delete_node_from_nand(nand_t *to_be_deleted_from, nand_t *to_be_deleted);
void delete_list_from_nand(nand_t *to_be_deleted_from);
int  add_to_record(record_node **list_head, record_node **list_tail, nand_t *to_be_added);
void clear_record(record_node **head_record);

#endif