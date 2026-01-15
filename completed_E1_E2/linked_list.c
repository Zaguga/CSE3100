#include "linked_list.h"

// create a node with value v
node *create_node(int v) {
  node *p = malloc(sizeof(node)); // Allocate memory
  assert(p != NULL);              // you can be nicer

  // Set the value in the node.
  p->v = v;
  p->next = NULL;
  return p; // return
}

// TODO:
void insert(node **head, node *newnode) {
  if ((*head) == NULL) {
    // fill in code below
    (*head) = newnode;
  }
  node *p = (*head);
  node *q = p;
  while (p->next != NULL && newnode->v > p->v) {
    // fill in code below
    p->next = p->next;
  }
  if (newnode->v > p->v) {
    // fill in code below
    p->next = newnode;

  }
  // fill in code below
  newnode->next = NULL;
  return;
}

// add newnode the last of the linked list determined by *head and *tail
// note **head, **tail
void insert_last(node **head, node **tail, node *newnode) {
  if ((*head) == NULL) {
    (*head) = (*tail) = newnode; //single item in linked list
  } else {
    (*tail)->next = newnode; // add new node to the end
    (*tail) = newnode; //the new end is now the new node
  }
}

// remove the first node from the list
// note **head
// return a pointer to the removed content

node *remove_first(node **head) {
  node *p;

  p = (*head);
  if ((*head) != NULL)
    (*head) = (*head)->next;
  return p;
}

// print the list pointed by head
void print_list(node *head) {
  node *p = head;

  while (p != NULL) {
    printf("%d ", p->v);
    p = p->next;
  }
  printf("\n");
}

// TODO
// merge two sorted linked list as one
node *merge(node **p1, node **p2) {
  node *head = NULL, *tail = NULL;
  node *q1 = (*p1), *q2 = (*p2), *p;

  if (q1 == NULL && q2 == NULL)
    return NULL;
  if (q1 != NULL && q2 == NULL) {
    p = q1;
    (*p1) = NULL;
    return p;
  }
  if (q1 == NULL && q2 != NULL) {
    p = q2;
    (*p2) = NULL;
    return p;
  }

  while (q1 != NULL && q2 != NULL) {
    // fill in code below
   insert(**q1, **q2);
  }
  while (q1 != NULL) {
    // fill in code below
    insert_last(*head, *tail, q1);
  }
  while (q2 != NULL) {
    // fill in code below
    insert_last(*head, *tail, q2);
  }
  // fill in code below

  return head;
}
int main(int argc, char *argv[]) {
  assert(argc == 2);
  int n = atoi(argv[1]);

  int a[n];
  int b[n];

  for (int i = 0; i < n; i++) {
    a[i] = i;
    b[i] = 2 * i;
  }

  node *p1 = NULL, *p2 = NULL;

  for (int i = 0; i < n; i++) {
    node *p = create_node(a[i]);
    insert(&p1, p);
  }
  print_list(p1);

  for (int i = 0; i < n; i++) {
    node *p = create_node(b[i]);
    insert(&p2, p);
  }
  print_list(p2);

  node *p = merge(&p1, &p2);
  print_list(p);
  printf("%p\n", p1);
  printf("%p\n", p2);

  // TODO
  // fill in code below

  return 0;
}


//bro this girl is getting caught cheating and its distracting tf outta me rn, 
//anyways if i get 10 more points on grade scope i can get a 67 as my gradeeee kinda sendy