#include "9cc.h"

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

Node *new_node_fn_call(char *name, Vector *arguments) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FN_CALL;
  node->name = malloc(sizeof(char) * (strlen(name) + 1));
  node->name = name;
  node->args = malloc(sizeof(Vector));
  node->args = arguments;
  if (!map_get(variables, name)) {
    // TODO: replace latter "name" with Type annotation
    map_put(variables, name, name);
  }
  return node;
}

Node *new_node_fn_decl(char *name,Vector *formal_args, Vector *body) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FN_DECL;
  node->name = malloc(sizeof(char) * (strlen(name) + 1));
  node->name = name;
  if (formal_args) {
    node->args = formal_args;
  }
  if (body) {
    node->body = body;
  }
  return node;
}

Node *new_node_ident(char *name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = malloc(sizeof(char) * (strlen(name) + 1));
  node->name = name;
  if (!map_get(variables, name)) {
    // TODO: replace latter "name" with Type annotation
    map_put(variables, name, name);
  }
  return node;
}

int consume(int ty) {
  Token *token = tokens->data[pos];
  if (token->ty != ty) return 0;
  pos++;
  return 1;
}

Token *current_token() {
  return (Token *)tokens->data[pos];
}

void error(int line, char *str, char *arg) {
  fprintf(stderr, str, arg);
  printf(str, arg);
  Token *token = tokens->data[pos];
  fprintf(stderr, "\n%d:, pos: %d, token-type: %c %d\n",line, pos, token->ty, token->ty);
  printf("\n%d: pos: %d, token-type: %c %d\n",line, pos, token->ty, token->ty);
  exit(1);
}

/*
program: fn_decl program
program: stmt program
program: e

#fn_def: fn_decl block
#block: '{' stmt '}'
#type: "int"

stmt: assign ";"

fn_decl: ident '(' formal_args ')' '{' stmt '}'

formal_args: e
formal_args: ident
formal_args: ident ',' ident

assign: cmp
assign: add "=" assign

cmp: add
cmp: add "==" add
cmp: add "!=" add

add: mul
add: add "+" mul
add: add "-" mul

mul: term
mul: mul "*" term
mul: mul "/" term

term: "(" add ")"
term: num
term: ident
term: fnCall

fn_call: ident '(' args ')'

args: e
args: add
args: add ',' add

*/
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

Vector *actual_args() {
  if (((Token *)tokens->data[pos])->ty == ')') {
    pos++;
    return NULL;
  }
  Vector *arguments = new_vector();
  Node *node = add();
  vec_push(arguments, node);
  while(consume(',')) {
    vec_push(arguments, add());
  }
  if (consume(')')) {
    return arguments;
  }
  error(__LINE__, "no corresponding close paren %s", ((Token *)tokens->data[pos])->input);
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')')) {
      Token *token = tokens->data[pos];
      error(__LINE__, "no corresponding close paren %s", token->input);
    }
    return node;
  }

  Token *token = tokens->data[pos];
  if (token->ty == TK_NUM) {
    token = tokens->data[pos++];
    return new_node_num(token->val);
  }

  if (token->ty == TK_IDENT) {
    char *name = ((Token *)tokens->data[pos++])->input;
    if (consume('(')) {
      return new_node_fn_call(name, actual_args());
      //error("no corresponding close paren %s", token->input);
    }
    return new_node_ident(name);
  }

  error(__LINE__, "invalid token: %s", token->input);
}

Node *cmp() {
  Node *node = add();
  if (consume(TK_EQ)) {
    node = new_node(ND_EQ, node, add());
  } else if (consume(TK_NEQ)) {
    node = new_node(ND_NEQ, node, add());
  }
  return node;
}

Node *assign() {
  Node *node = cmp();
  if (consume('=')) {
    node = new_node('=', node, assign());
  }
  return node;
}

Node *stmt() {
  Node *node = assign();
  if (!consume(';')) {
    error(__LINE__, "expected token ';', got %s", current_token()->input);
  }
  return node;
}

Vector *formal_args() {
  if (consume('(')) {
    if (current_token()->ty == ')') {
      pos++;
      return NULL;
    }
    Vector *arguments = new_vector();
    vec_push(arguments, ident());
    while(consume(',')) {
      vec_push(arguments, add());
    }
    if (consume(')')) {
      return arguments;
    }
    //error(__LINE__, "no corresponding close paren %s", current_token()->input);
  }
  return NULL;
}

Node *ident() {
  Token *token = tokens->data[pos];
  if (token->ty == TK_IDENT) {
    char *name = ((Token *)tokens->data[pos++])->input;
    return new_node_ident(name);
  }
  return NULL;
}

Node *fn_decl() {
  int last_pos = pos;
  Node *fn_name = ident();
  if (fn_name != NULL && current_token()->ty == '(') {
    Node *args = formal_args();
    if (consume('{')) {
      Node *body = new_vector();
      vec_push(body, stmt());
      if (consume('}')) {
        return new_node_fn_decl(fn_name->name, args, body);
      }
      error(__LINE__, "no corresponding closing brace %s", current_token()->input);
    }
  }
  // if it's not fn_decl, rewind the token position
  // perhaps I should implement peek function
  pos = last_pos;
  return NULL;
}

void program() {
  int i = 0;
  Token *token = tokens->data[pos];
  while (token->ty != TK_EOF) {
    code[i] = fn_decl();
    if (code[i] == NULL) {
      code[i] = stmt();
    }
    i++;
    token = tokens->data[pos];
  }
  code[i] = NULL;
}

