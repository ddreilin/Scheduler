/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  q->length = 0;
  q->head = NULL;
  q->comparer = comparer;
}

node_t * node_init( void *ptr )
{
  node_t *n = malloc(sizeof(node_t));
  n->value = *(int*)ptr;
  n->next = NULL;
  return n;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
  int index = 0;
  node_t *temp_node = q->head;
  node_t *prev_temp = NULL;
  while(temp_node != NULL){

    //found spot in queue
    if(q->comparer(temp_node->value, ptr) > 0){

      //Insert in queue
      prev_temp->next = node_init(ptr);
      prev_temp->next->next = temp_node;
      q->length = q->length + 1;
      return index;

    }

    //shift down one in the queue
    prev_temp = temp_node;
    temp_node = temp_node->next;
    index++;

  }

  //insert at the head.
  if(index == 0){
    q->head = node_init(ptr);
  }

  //insert at the end
  else{
    temp_node->next = node_init(ptr);
  }


  q->length = q->length + 1;
  return index;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
  //queue is empty
  if(q->head == NULL){
    return NULL;
  }

  //return the value of the head.
  else{
    return q->head->value;
  }
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
  //queue is empty
  if(q->head == NULL){
    return NULL;
  }

  //return the head and remove it from the queue
  else{
    node_t prev_head = q->head;
    q->head = q->head->next;
    q->length = q->length - 1;
    return prev_head;
  }
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
  //index is not in queue
  if(index > (q->length -1){
    return NULL;
  }

  else{
    node_t temp_node = q->head;
    int loops = 0;

    //loops until temp_node contains the index-th node.
    while(loops != index){
      temp_node = temp_node->next;
      loops++;
    }

    return temp_node->value;
  }

}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  int entries = 0;

  //if the head has the value
  if(q->head->value == ptr){

    while(q->head->value == ptr){
      node_t temp_node = q->head;
      q->head = q->head->next;
      q->length = q->length - 1;
      entries++;
      free(temp_node);
    }
  }

  //else check down the queue
  else{
    node_t temp_node = q->head;
    node_t prev_temp = NULL;

    while(temp_node != NULL){

      //if the node is equal and not the head
      if(temp_node->value == ptr){
        prev_temp->next = temp_node->next
        free(temp_node);
        q->length = q->length - 1;
        entries++;
      }

      prev_temp = temp_node;
      temp_node = temp_node->next;

    }
  }

	return entries;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
  int value;
  //index is not in queue
  if(index > (q->length -1){
    return NULL;
  }

  else{
    //remove the head
    if(index == 0){
      value = q->head->value;
      priqueue_poll(q);
    }

    else{

      node_t temp_node = q->head;
      node_t prev_node = NULL;
      int loops = 0;

      //loops until temp_node contains the index-th node.
      while(loops != index){
        prev_node = temp_node;
        temp_node = temp_node->next;
        loops++;
      }

      value = temp_node->value;
      prev_node->next = temp_node->next;
      q->length = q->length - 1;
      free(temp_node);
    }

  }
	return value;
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->length;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
  //please
  free(q);
}
