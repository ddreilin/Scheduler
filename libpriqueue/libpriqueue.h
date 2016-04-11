/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_

typedef int (*comparer_t)(const void *, const void *);

/**
  Priqueue Node Data Structure
*/
typedef struct _node_t
{
  void *value;
  struct _node_t *next;
} node_t;

/**
  Priqueue Data Structure
*/
typedef struct _priqueue_t
{
  int length;
  node_t *head;
  comparer_t comparer;
} priqueue_t;


void   priqueue_init     (priqueue_t *q, comparer_t cmp);

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */
