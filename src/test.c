#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>
#include "error.h"
typedef struct client_list_s
{
    int client;
    char *addr;
    struct client_list_s *next;
} client_list_t;

static client_list_t *new_client_list(int client, char *addr)
{
    client_list_t *head = (client_list_t *)malloc(sizeof(client_list_t));
    if (!head)
    {
        err_handle("Error: Fail to new client list");
    }
    head->addr = addr;
    head->client = client;
    head->next = NULL;
    return head;
}

static client_list_t *delete_client_list(client_list_t **list, int client)
{
    if (!list)
        return NULL;

    client_list_t **indirect = list;
    while ((*indirect)->client != client)
        indirect = &(*indirect)->next;
    *indirect = (*indirect)->next;
}

static void push_back_client(client_list_t **list, int client, char *addr)
{
    if (!(*list))
        *list = new_client_list(client, addr);
    else
    {
        client_list_t *tmp = *list;
        while (tmp->next)
            tmp = tmp->next;
        tmp->next = new_client_list(client, addr);
    }
}
int main()
{
    client_list_t *head = new_client_list(1, 'a');
    push_back_client(&head, 2, 'x');
    push_back_client(&head, 3, 'z');
    delete_client_list(&head, 2);
    delete_client_list(&head, 2);
}