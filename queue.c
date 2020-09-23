#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
 * Refer to @RinHizakura
 * For replacing 'strcpy'
 */
#ifndef strlcpy
#define strlcpy(dst, src, sz) snprintf((dst), (sz), "%s", (src))
#endif

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));
    if (q == NULL)
        return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    if (q == NULL)
        return;

    while (q->head != NULL) {
        list_ele_t *tmp = q->head;  // Save temp q
        q->head = q->head->next;    // Make q go to next
        free(tmp->value);
        free(tmp);
    }
    free(q);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    if (q == NULL)
        return false;
    list_ele_t *newh;
    newh = malloc(sizeof(list_ele_t));
    // Case when malloc fail in creating newh
    if (newh == NULL)
        return false;
    char *tmp = malloc(sizeof(char) * (strlen(s) + 1));
    // Case when malloc fail in creating tmp
    if (tmp == NULL) {
        free(newh);
        return false;
    }

    newh->value = tmp;
    strlcpy(newh->value, s, (strlen(s) + 1));
    newh->next = q->head;
    q->head = newh;
    // For the case when the queue is empty
    if (q->size == 0)
        q->tail = newh;
    q->size++;
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    if (q == NULL)
        return false;
    list_ele_t *newh;
    newh = malloc(sizeof(list_ele_t));
    // Case when malloc fail in creating newh
    if (newh == NULL)
        return false;
    char *tmp = malloc(sizeof(char) * (strlen(s) + 1));
    // Case when malloc fail in creating tmp
    if (tmp == NULL) {
        free(newh);
        return false;
    }
    newh->value = tmp;
    strlcpy(newh->value, s, (strlen(s) + 1));
    newh->next = NULL;

    // Case when the queue is empty
    if (q->size == 0) {
        q->tail = newh;
        q->head = newh;
    } else {
        q->tail->next = newh;
        q->tail = newh;
    }
    q->size++;
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return true if successful.
 * Return false if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 * The space used by the list element and the string should be freed.
 */
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
    if (q == NULL)
        return false;
    if (sp == NULL || q->head == NULL)
        return false;
    if (bufsize > strlen(q->head->value)) {
        strncpy(sp, q->head->value, strlen(q->head->value));
        sp[strlen(q->head->value)] = '\0';
    } else {
        strncpy(sp, q->head->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_ele_t *tmp = q->head;
    q->head = q->head->next;
    free(tmp->value);
    free(tmp);
    q->size--;

    // For the case when the queue is empty
    if (q->size == 0)
        q->tail = NULL;
    return true;
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    if (q == NULL || q->head == NULL)
        return 0;
    return q->size;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    if (q == NULL || q->head == NULL)
        return;
    list_ele_t *cursor = NULL;

    q->tail = q->head;
    while (q->head) {
        list_ele_t *next = q->head->next;
        q->head->next = cursor;
        cursor = q->head;
        q->head = next;
    }
    q->tail->next = NULL;
    q->head = cursor;
}

/*
 * move nodes to position, used in merge sort
 */
void move(list_ele_t **destRef, list_ele_t **sourceRef)
{
    list_ele_t *tmp = *sourceRef;
    *sourceRef = tmp->next;
    tmp->next = *destRef;
    *destRef = tmp;
}

/*
 * Split queue into groups
 */
void split(list_ele_t *head, list_ele_t **front_ref, list_ele_t **back_ref)
{
    // Case when length is less than 2
    if (head == NULL || head->next == NULL) {
        *front_ref = head;
        *back_ref = NULL;
        return;
    }

    list_ele_t *first = head;
    list_ele_t *second = head->next;

    // First goes a step, Second goes two steps.
    // When second reach the end, first will be the mid point.
    while (second != NULL) {
        second = second->next;  // For the case when length is odd number.
        if (second != NULL) {
            first = first->next;
            second = second->next;
        }
    }

    *front_ref = head;
    *back_ref = first->next;
    first->next = NULL;
}
/*
 * Merge the groups in order
 */
list_ele_t *merge(list_ele_t *a, list_ele_t *b)
{
    // Refer to RinHizakura, using dummy first node to hang the result
    list_ele_t dummy;
    list_ele_t *tail = &dummy;
    dummy.next = NULL;

    while (1) {
        if (a == NULL) {
            tail->next = b;
            break;
        } else if (b == NULL) {
            tail->next = a;
            break;
        }
        // strcmp, if a<b, return negative value
        if (strcmp(a->value, b->value) < 0) {
            move(&tail->next, &a);
        } else {
            move(&tail->next, &b);
        }
        tail = tail->next;
    }
    return dummy.next;
}

/*
 * Perform merge sort
 */
void merge_sort(list_ele_t **head)
{
    if (*head == NULL || (*head)->next == NULL)
        return;

    list_ele_t *a;
    list_ele_t *b;

    split(*head, &a, &b);

    merge_sort(&a);
    merge_sort(&b);

    *head = merge(a, b);
}


/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(queue_t *q)
{
    if (q == NULL || q->head == NULL)
        return;

    merge_sort(&q->head);
}
