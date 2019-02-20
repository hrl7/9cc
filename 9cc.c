#include "9cc.h"

int pos;
Vector *tokens;
Node *code[100];

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elm) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }

  vec->data[vec->len++] = elm;
}


Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(char name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

int consume(int ty) {
  Token *token = tokens->data[pos];
  if (token->ty != ty) return 0;
  pos++;
  return 1;
}

void error(char *str, char *arg) {
  printf(str);
  fprintf(stderr, str, arg);
  exit(1);
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node('+', node, mul());
    } else if (consume('-')) {
      node = new_node('-', node, mul());
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume('*')) {
      node = new_node('*', node, term());
    } else if (consume('/')) {
      node = new_node('/', node, term());
    } else {
      return node;
    }
  }
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')')) {
      Token *token = tokens->data[pos];
      error("no corresponding close paren %s", token->input);
    }
    return node;
  }

  Token *token = tokens->data[pos];
  if (token->ty == TK_NUM) {
    token = tokens->data[pos++];
    return new_node_num(token->val);
  }

  if (token->ty == TK_IDENT) {
    token = tokens->data[pos++];
    return new_node_ident(token->input);
  }

  error("invalid token: %s", token->input);
}

Node *assign() {
  Node *node = add();
  if (consume('=')) {
    node = new_node('=', node, assign());
  }
  return node;
}

Node *stmt() {
  Node *node = assign();
  if (!consume(';')) {
    Token *token = tokens->data[pos];
    error("expected token ';', got %s", token->input);
  }
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  gen(node->lhs);
  gen(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->ty) {
    case '+':
      printf("  add rax, rdi\n");
      break;
    case '-':
      printf("  sub rax, rdi\n");
      break;
    case '*':
      printf("  mul rdi\n");
      break;
    case '/':
      printf("  mov rdx, 0\n");
      printf("  div rdi\n");
      break;
  }
  printf("  push rax\n");
}

void tokenize(char *p) {
  int i = 0;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    Token *token = malloc(sizeof(Token));
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '=' || *p == ';') {
      token->ty = *p;
      token->input = p;
      vec_push(tokens, token);
      i++;
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      token->ty = TK_IDENT;
      token->input = p;
      vec_push(tokens, token);
      i++;
      p++;
      continue;
    }

    if (isdigit(*p)) {
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      vec_push(tokens, token);
      i++;
      continue;
    }

    fprintf(stderr, "cannot tokenize: %s\n", p);
    exit(1);
  }

  Token *token = malloc(sizeof(Token));
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}


int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }
  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }

  tokens = new_vector();
  tokenize(argv[1]);
  Node *node = add();

  printf(".intel_syntax noprefix\n");
#ifdef __APPLE__
  printf(".global _main\n");
  printf("_main:\n");
#else
  printf(".global main\n");
  printf("main:\n");
#endif

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

int expect(int line, int expected, int actual) {
  if (expected == actual) {
    return 0;
  }
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
}

void runtest() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++) {
    vec_push(vec, (void *)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  printf("OK\n");
}
