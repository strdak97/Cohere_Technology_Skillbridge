/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    /* What if malloc returned NULL? */
    if(q == NULL) {
      return NULL;
    }

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    /* How about freeing the list elements and the strings? */
    /* Free queue structure */
    if(q == NULL) {
      return;
    }

    list_ele_t *curr = q->head;
    list_ele_t *temp = curr;
    
    while(curr != NULL) {
      temp = curr;
      curr = curr->next;
      free(temp->value);
      free(temp);
    }

    free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    /* What should you do if the q is NULL? */
    /* Don't forget to allocate space for the string and copy it */
    /* What if either call to malloc returns NULL? */
    if(q == NULL) {
      return false;
    }

    list_ele_t *newHead = malloc(sizeof(list_ele_t)); 

    if(newHead == NULL) {
      return false;
    }

    newHead->value = malloc(strlen(s) + 1);
    newHead->next = NULL;
    
    if(newHead->value == NULL) {
      free(newHead);
      return false;
    }

    strcpy(newHead->value, s);
    newHead->next = q->head;
    q->head = newHead;
    q->size++;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    /* You need to write the complete code for this function */
    /* Remember: It should operate in O(1) time */
    if(q == NULL) {
      return false;
    }

    list_ele_t *newTail = malloc(sizeof(list_ele_t));

    if(newTail == NULL) {
      return false;
    }

    newTail->value = malloc(strlen(s) + 1);
    if(newTail->value == NULL) {
      free(newTail);
      return false;
    }

    strcpy(newTail->value, s);
    newTail->next = NULL; 

    if (!q->head) {
        q->head = newTail;
    }

    if (q->tail) {
        q->tail->next = newTail;
    }
    q->tail = newTail;
    q->size++;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    /* You need to fix up this code. */
    if(q == NULL || q->head == NULL || sp == NULL) {
      return false;
    }
    list_ele_t *remHead = q->head;
    q->head = remHead->next;

    if(q->tail == remHead) {
      q->tail = NULL;
    }

    strncpy(sp, remHead->value, bufsize);
    sp[bufsize - 1] = '\0';

    free(remHead->value);
    free(remHead);

    q->size--;
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if(q == NULL || q->head == NULL) {
      return 0;
    }

    return q->size;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    /* You need to write the code for this function */
    if(q == NULL || q->head == NULL) {
      return;
    }

    list_ele_t *prev = NULL;
    list_ele_t *curr = q->head;
    list_ele_t *next = NULL;

    while(curr != NULL) { //need to study this more
      next = curr->next;
      curr->next = prev;
      prev = curr;
      curr = next;
    }
    
    q->tail = q->head;
    q->head = prev;
}

