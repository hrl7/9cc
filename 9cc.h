#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EOF,
  TK_EQ, // ==
  TK_NEQ, // !=
  TK_GE, // <= greater or equal
  TK_LE, // <= less or equal
};

typedef struct Token {
  int ty;
  int val;
  char *input;
} Token;


enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_EQ,
  ND_NEQ,
  ND_GE, // greater or equal
  ND_LE, // less or equal
  ND_GT, // greater than
  ND_LT, // less than
  ND_FN_CALL,
  ND_FN_DECL,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_RET,
  ND_VAR_DECL,
  ND_ADDRESS,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char *name; // IDENT, FN_CALL function name
  struct Vector *args; // Vector of Node for FN_CALL, FN_DECL
  struct Vecotr *body; // Vector of Node for FN_DECL, ND_IF, ND_WHILE, ND_FOR
  struct Node *cond; // Condition for ND_IF, ND_WHILE, ND_FOR
  struct Node *init; // ND_FOR
  struct Node *updater; // ND_FOR
  struct Vector *els; // else clause for ND_IF
  struct Context *ctx;
  struct Type *data_type;
} Node;

typedef struct Type {
  enum { INT, PTR } ty;
  struct Type *ptr_of;
} Type;

typedef struct Context {
  struct Vector *vars;
  struct Context *parent;
  char *name;
} Context;

typedef struct Vector {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct Map {
  Vector *keys;
  Vector *vals;
} Map;

Node *add();
Node *mul();
Node *term();
Vector *stmt();
Node *ident();
Vector *formal_args();
Vector *actual_args();
void program();

Context *new_context(const char *name);
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(Type *type, char *name);

Vector *new_vector();
void vec_push(Vector *vec, void *elm);

Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

int expect(int line, int expected, int actual);
void runtest();

extern Vector *scopes;
extern Map *variables;
extern int pos;
extern int branch_id;
extern Node *code[100];
extern Vector *tokens;
