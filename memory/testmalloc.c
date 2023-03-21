//https://www.learn-c.org/en/Linked_lists

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    int val;
    struct node *next;
} node_t;

void print_list(node_t *head)
{
    node_t *current = head;

    while (current != NULL)
    {
        printf("%d\n", current->val);
        current = current->next;
    }
}

void main()
{
    node_t *head;
    if (!head)
    {
        printf("generate head\n");
        head = (node_t *)malloc(sizeof(node_t));
        head->val = 12;
    }
    head->val = 1;

    free(head);
}