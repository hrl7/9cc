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
  ND_IDENT = 257,
  ND_EQ = 258,
  ND_NEQ = 259,
  ND_GE = 260, // greater or equal
  ND_LE = 261, // less or equal
  ND_GT = 262, // greater than
  ND_LT = 263, // less than
  ND_FN_CALL = 264,
  ND_FN_DECL = 265,
  ND_IF = 266,
  ND_WHILE = 267,
  ND_FOR = 268,
  ND_RET = 269,
  ND_VAR_DECL = 270,
  ND_REF = 271,
  ND_DEREF = 272,
  ND_ARG,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  int offset;
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
  enum { INT, PTR, ARRAY } ty;
  struct Type *ptr_of;
  size_t array_size;
} Type;

typedef struct Context {
  struct Context *parent;
  char *name;
  struct Map *vars; // Map<var_name, Record>
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

typedef struct Record {
  int offset;
  char *name;
  Type *type;
  int is_arg;
} Record;

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
Node *new_node_ident(char *name);
Type *new_int_type();
Type *new_ptr_type(Type *ptr_of);

size_t get_data_width(Node *node);
size_t get_data_width_by_record(Record *rec);
size_t get_data_width_by_type(Type *type);

Vector *new_vector();
void vec_push(Vector *vec, void *elm);

Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

Record *new_record(char *name, int offset, Type *type, int is_arg);
Record *get_record(Context *ctx, char *name);
void free_vector(Vector *vec);

int expect(int line, int expected, int actual);
void runtest();

extern Vector *scopes;
extern Map *variables;
extern int pos;
extern int branch_id;
extern Node *code[100];
extern Vector *tokens;
