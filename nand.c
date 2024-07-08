#include "nand.h"
#include "nand_helper.h"

// The function creates a new gate with the indicated number of inputs:
//  n (number of inputs).
// Possible returned results of the function are:
//  pointer to a gate - if everything succeeded,
//  NULL - if a memory allocation error occurred.
nand_t* nand_new(unsigned n) {
  nand_t *new_nand = (nand_t*) malloc(sizeof(nand_t));
  if (new_nand == NULL) {
    errno = ENOMEM;
    return NULL;
  }

  new_nand->counter_in = n;
  new_nand->counter_ocupied = 0;

  new_nand->set_in = (void**) malloc(n * sizeof(void*));
  if (new_nand->set_in == NULL) {
    free(new_nand);
    errno = ENOMEM;
    return NULL;
  }

  new_nand->set_in_content = (int*) malloc(n * sizeof(int));
  if (new_nand->set_in_content == NULL) {
    free(new_nand->set_in);
    free(new_nand);
    errno = ENOMEM;
    return NULL;
  }

  for (unsigned i = 0; i < n; ++i) {
    new_nand->set_in[i] = NULL;
    new_nand->set_in_content[i] = 0;
  }

  new_nand->counter_out = 0;
  new_nand->head_out = NULL;
  new_nand->tail_out = NULL;

  new_nand->record_created = false;
  new_nand->record_finished = false;
  new_nand->to_be_visited = 0;
  new_nand->record_found_false = false;
  new_nand->record_critical_path = 0;

  return new_nand;
}

// The function disconnects the input and output signals of the indicated gate:
//  g (pointer to a gate),
// and then deletes the gate by freeing all the memory used by it. Function
// uses helper functions: delete_node_from_nand and delete_list_from_nand
// (definitions of those can be found in the file nand_out.c).
// Returned result of the function is:
//  void.
void nand_delete(nand_t *g) {
  if (g == NULL) {
    return;
  }

  // Removal of gate input connections.
  for (unsigned i = 0; i < g->counter_in; ++i)
    if (g->set_in_content[i] == 2) delete_node_from_nand(
        (nand_t*) (g->set_in[i]) , g);

  free(g->set_in_content);
  free(g->set_in);

  delete_list_from_nand(g);

  free(g);
}

// The function connects the output of the indicated gate:
//  g_out (pointer to the gate whose output is to be connected to the input k
//         of the gate pointed to by g_in)
// to the indicated input:
//  k (the input number of the gate pointed to by g_in, to which the output
//     of the gate pointed to by g_out is to be connected)
// of another indicated gateway:
//  g_in (pointer to the gate whose input k is to be connected to the output
//        of the gate pointed to by g_out)
// In doing so, it possibly disconnects from this input the signal that
// was previously connected to it. Function uses helper functions:
// delete_node_from_nand and add_node_to_nand (definition of those can be found
// in the file nand_out.c). Possible returned results of the function are:
//  0 - if everything succeeded,
//  -1 - if any pointer is NULL, parameter k has an invalid value, or a memory
//       allocation error has occurred.
int nand_connect_nand(nand_t *g_out, nand_t *g_in, unsigned k) {
  if (g_out == NULL || g_in == NULL || ((ssize_t) k) >= g_in->counter_in) {
    errno = EINVAL;
    return -1;
  }

  // Disconnection.
  if (g_in->set_in_content[k] == 0) {
    g_in->counter_ocupied++;
    g_in->set_in_content[k] = 2;
  } else if (g_in->set_in_content[k] == 1) {
    g_in->set_in_content[k] = 2;
  } else {
    delete_node_from_nand( (nand_t*) (g_in->set_in[k]) , g_in);
  }

  // Connection.
  g_in->set_in[k] = (void*) g_out;
  if (add_node_to_nand(g_out, g_in, k) == -1) {
    errno = ENOMEM;
    return -1;
  }

  return 0;
}

// The function connects the indicated boolean signal:
//  s (pointer to the boolean signal to be connected to the input k
//     of the gate pointed to by g),
// to the indicated input:
//  k – (the number of the input of the gate indicated by g, to which
//       the boolean signal indicated by s is to be connected),
// of the indicated gate:
//  g (pointer to the gate whose input k is to be connected to the boolean
//     signal indicated by s).
// In doing so, it possibly disconnects from that input the signal that
// was previously connected to it. Function uses helper function
// delete_node_from_nand (definition of which can be found in
// the file nand_out.c). Possible returned results of the function are:
//  0 - if everything succeeded,
//  -1 - if any pointer is NULL, parameter k has an invalid value,
//       or a memory allocation error has occurred.
int nand_connect_signal(bool const *s, nand_t *g, unsigned k) {
  if(s == NULL || g == NULL || ((ssize_t) k) >= g->counter_in) {
    errno = EINVAL;
    return -1;
  }

  if (g->set_in_content[k] == 0) {
    g->counter_ocupied++;
    g->set_in_content[k] = 1;
  } else if (g->set_in_content[k] == 2) {
    g->set_in_content[k] = 1;
    delete_node_from_nand( (nand_t*) (g->set_in[k]) , g);
  }

  g->set_in[k] = (void*) s;

  return 0;
}

// The function searches the graph formed by related gates according
// to the DFS scheme. It is called recursively for each gate whose
// output is connected to one of the inputs of the gate for which it
// was called previously.
// The parameters of the function are:
//  current - pointer to gate,
//  found_false - pointer to a variable indicating whether any boolean signal
//                with the value false was found within nand_evaluate_recursion
//                function call for the current gate,
//  head_record - pointer to the pointer to the first node of the list
//                containing pointers to gates for which, within a given
//                nand_evaluate function call, the value of the boolean signal
//                at the output of the gate in question and the length of
//                the critical path ending at the gate in question have already
//                been started to get checked,
//  tail_record - pointer to the pointer to the last node in the same list.
// Result of the function is:
//  the length of the critical path ending at the gate indicated by current.
ssize_t nand_evaluate_recursion(nand_t *current,
                                bool *fill_found_false,
                                record_node **head_record,
                                record_node **tail_record) {
  // If one of 2 situations occurs:
  //  1) no output of any other gate or any boolean signal is connected to one
  //     of inputs of the gate in question,
  //  2) the nand_evaluate_recursion function has already evaluated values
  //     at given gate,
  // we can return the results immediately. In other case we, need to check
  // if nand_evaluate has already started evaluating values at given gate
  // and if not - we need to indicate that it has changed.
  if (current->counter_in != current->counter_ocupied) {
    return -1;
  }

  if (current->record_finished == true) {
    *fill_found_false = current->record_found_false;
    return current->record_critical_path;
  } else if (current->record_created == false) {
    if (add_to_record(head_record, tail_record, current) == -1) {
      return -2;
    }
    current->record_created = true;
  }

  // We need to evaluate values at given gate - if there is no inputs,
  // it's easy. If there are some inputs, we need to check those. If those are
  // other nand gates, we need to evaluate values of interest at them first.
  if (current->counter_in == 0) {
    current->record_finished = true;
    current->record_critical_path = 0;
    current->record_found_false = false;

    *fill_found_false = false;
    return 0;
  }
  ssize_t max_child_height = 0;
  for (unsigned i = 0; i < current->counter_in; ++i) {
    if (i < current->to_be_visited)
      return -1;
    else
      current->to_be_visited++;

    if (current->set_in_content[i] == 1 &&
        *((bool const*) (current->set_in[i])) == false)
      *fill_found_false = true;
    else if (current->set_in_content[i] == 2) {
      nand_t *child = (nand_t*) (current->set_in[i]);
      bool child_found_false = false;
      ssize_t child_height = nand_evaluate_recursion( child,
                                                      &child_found_false,
                                                     head_record,
                                                     tail_record);

      if (child_found_false == false)
        *fill_found_false = true;

      if (child_height == -1)
        return -1;
      else if (child_height == -2)
        return -2;
      else if (child_height > max_child_height)
        max_child_height = child_height;
    }
  }

  current->record_finished = true;
  current->record_critical_path = 1 + max_child_height;
  current->record_found_false = *fill_found_false;

  return 1 + max_child_height;
}

// The function determines the values of the boolean signals at the outputs
// of the indicated gates and calculates the length of the critical path.
// To do this it uses the recursively called helper function,
// nand_evaluate_recursion. For each gate, the evaluated values are stored
// in the corresponding nand_t structure. In this way, for each call
// to the nand_evaluate function, checking the inputs of each gate
// is done only once. Finally, stored values in the checked gates are
// deleted by calling the helper function clear_record (definition of which
// can be found in the file nand_record.c). Parameters of the function are:
//  g – pointer to an array of pointers to structures representing gates,
//  s – a pointer to an array that holds the function-determined values
//      of the boolean signals at the gate outputs pointed to by the pointers
//      in the array g,
//  m – the size of the arrays pointed to by g and s.
// The possible results of the function are:
//  length of the critical path - if all is successful - table s then contains
//                                the determined values of the boolean signals
//                                at the gate outputs,
//  -1 – if any pointer is NULL or the function fails for any other reason.
ssize_t nand_evaluate(nand_t **g, bool *s, size_t m) {
  if (g == NULL || s == NULL || m < 1) {
    errno = EINVAL;
    return -1;
  }

  ssize_t global_max = 0;
  ssize_t local_max;
  record_node *head_record = NULL;
  record_node *tail_record = NULL;

  for (size_t i = 0; i < m; ++i) {
    if(g[i] == NULL) {
      errno = EINVAL;
      return -1;
    }
    s[i] = false;
    local_max = nand_evaluate_recursion(g[i],
                                        &s[i],
                                        &head_record,
                                        &tail_record);
    if (local_max == -1) {
      clear_record(&head_record);
      errno = ECANCELED;
      return -1;
    } else if (local_max == -2) {
      clear_record(&head_record);
      errno = ENOMEM;
      return -1;
    } else if (local_max > global_max)
      global_max = local_max;
  }

  clear_record(&head_record);

  return global_max;
}

// The function determines the number of gate inputs connected to the output
// of the indicated gate:
//  g (pointer to the gate).
// The possible results of the function are:
//  number of gate inputs connected to gate output g - if all is successful,
//  -1 – if the pointer g is NULL.
ssize_t nand_fan_out(nand_t const *g) {
  if (g == NULL) {
    errno = EINVAL;
    return -1;
  }

  return g->counter_out;
}

// The function returns a pointer to a boolean signal or a pointer to a gate
// if either a boolean signal or another gate, respectively, is connected
// to the indicated input:
//  k (the number of the input to be checked at the gate indicated by g),
// in the indicated gate:
//  g (pointer to the gate whose input k is to be checked).
// The possible results of the function are:
//  pointer to a boolean signal - if a boolean signal is connected to the input
//                                k in the gate indicated by g,
//  pointer to a gate - if there is a gate connected to the input k in the gate
//                      indicated by g,
//  NULL – if nothing is connected to input k in the gate pointed to by g,
//         if the pointer g is NULL or the value of k is invalid.
void* nand_input(nand_t const *g, unsigned k) {
  if (g == NULL || ((ssize_t) k) >= g->counter_in) {
    errno = EINVAL;
    return NULL;
  }

  if (g->set_in_content[k] == 0) {
    errno = 0;
    return NULL;
  } else if (g->set_in_content[k] == 1)
    return (bool*) g->set_in[k];
  else
    return (nand_t*) g->set_in[k];
}

// The function returns a pointer to that gate which, given an unchanged
// set of gates connected to the output of the indicated gate:
//  g (pointer to the gate whose sequence number k is to be checked),
// can be identified permanently with the indicated sequence number:
//  k (sequence number of one of the gates connected to the output
//     of gate g; a number between zero and a value one less than
//     the value of nand_fan_out(g)).
// The possible results of the function are:
//  pointer to a gate - if all is successful,
//  NULL - if the pointer is NULL or the value of k is invalid.
nand_t* nand_output(nand_t const *g, ssize_t k) {
  if (g == NULL || k < 0 || k >= g->counter_out) {
    errno = EINVAL;
    return NULL;
  }

  const out_node *current = g->head_out;

  ssize_t i = 0;
  while (i != k) {
    current = current->next;
    i++;
  }

  return current->nand_pointer;
}
