#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

Token tokens[100];

enum {
  ND_NUM = 256,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;


Node *add();
Node *mul();
Node *term();

Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Vector *new_vector();
void vec_push(Vector *vec, void *elm);

int expect(int line, int expected, int actual);
void runtest();
