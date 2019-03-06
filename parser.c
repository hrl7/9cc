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
  return node;
}

Node *new_node_fn_decl(Context *ctx, char *name,Vector *formal_args, Vector *body) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FN_DECL;
  node->name = malloc(sizeof(char) * (strlen(name) + 1));
  node->name = name;
  node->ctx = malloc(sizeof(Context));
  node->ctx = ctx;
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
  return node;
}

Node *new_node_if_stmt(Node *cond, Vector *body, Vector *else_clause) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IF;
  node->cond = cond;
  node->body = body;
  node->els = else_clause;
  return node;
}

Node *new_node_while_stmt(Node *cond, Vector *body) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_WHILE;
  node->cond = cond;
  node->body = body;
  return node;
}

Node *new_node_for_stmt(Node *init, Node *cond, Node *updater, Vector *body) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FOR;
  node->init = init;
  node->cond = cond;
  node->updater = updater;
  node->body = body;
  return node;
}

Node *new_node_ret(Node *body) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_RET;
  node->body = body;
  return node;
}

Node *new_node_var_decl(Node *ident) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_VAR_DECL;
  node->name = ident->name;
  return node;
}

Token *current_token() {
  return (Token *)tokens->data[pos];
}

void consume_and_assert(int line, char c) {
  if (!consume(c)) {
    error(line, "expected '%c', got token: %c", c, current_token()->input[0]);
  }
}

void consume_keyword_and_assert(int line, const char *keyword) {
  if (!consume_keyword(keyword)) {
    error(line, "expected '%s', got token: %s", keyword, current_token()->input);
  }
}

int consume(int ty) {
  if (current_token()->ty != ty) return 0;
  pos++;
  return 1;
}

int consume_keyword(const char *keyword) {
  Token *token = current_token();
  if (token->ty != TK_IDENT || strcmp(token->input, keyword)) return 0;
  pos++;
  return 1;
}


void error(int line, char *str, char *arg) {
  fprintf(stderr, str, arg);
  printf(str, arg);
  Token *token = tokens->data[pos];
  fprintf(stderr, "\n %s %d:, pos: %d, token-type: %c %d\n", __FILE__, line, pos, token->ty, token->ty);
  printf("\n %s %d: pos: %d, token-type: %c %d\n",__FILE__, line, pos, token->ty, token->ty);
  exit(1);
}

/*
program: e
program: fn_decl program

#fn_def: fn_decl block
#block: '{' stmt '}'
#type: "int"

var_decl: 'int' ident

stmt: e
stmt: if_stmt stmt
stmt: while_stmt stmt
stmt: 'return' assign ';' stmt
stmt: var_decl ';'
stmt: assign ';' stmt

if_stmt: 'if' '(' assign ')' assign ';'
if_stmt: 'if' '(' assign ')' '{' stmt '}'
if_stmt: 'if' '(' assign ')' '{' stmt '}' 'else' assgin ';'
if_stmt: 'if' '(' assign ')' '{' stmt '}' 'else' '{' stmt '}'

while_stmt: 'while' '(' assign ')' assign ';'
while_stmt: 'while' '(' assign ')' '{' stmt '}'

fn_decl: 'int' ident '(' formal_args ')' '{' stmt '}'

formal_args: e
formal_args: 'int' ident
formal_args: 'int' ident ',' formal_args

assign: cmp
assign: add "=" assign

cmp: add
cmp: add "==" add
cmp: add "!=" add
cmp: add ">" add
cmp: add "<" add
cmp: add ">=" add
cmp: add "<=" add

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
    }
    return new_node_ident(name);
  }

  error(__LINE__, "invalid token: %s", token->input);
}

Node *cmp() {
  Node *node = add();
  if (consume(TK_EQ)) return new_node(ND_EQ, node, add());
  if (consume(TK_NEQ)) return new_node(ND_NEQ, node, add());
  if (consume(TK_GE)) return new_node(ND_GE, node, add());
  if (consume(TK_LE)) return new_node(ND_LE, node, add());
  if (consume('<')) return new_node(ND_GT, node, add());
  if (consume('>')) return new_node(ND_LT, node, add());
  return node;
}

Node *assign() {
  Node *node = cmp();
  if (consume('=')) {
    node = new_node('=', node, assign());
  }
  return node;
}

Node *if_stmt() {
  if (consume_keyword("if")) {
    consume_and_assert(__LINE__, '(');
    Node *cond = assign();
    consume_and_assert(__LINE__, ')');

    Vector *body = NULL;
    Vector *else_clause = NULL;
    if (consume('{')) {
      body = stmt();
      consume_and_assert(__LINE__, '}');
    } else {
      body = new_vector();
      vec_push(body, assign());
      consume_and_assert(__LINE__, ';');
    }

    if (consume_keyword("else")) {
      if (consume('{')) {
        else_clause = stmt();
        consume_and_assert(__LINE__, '}');
      } else {
        else_clause = new_vector();
        vec_push(else_clause, assign());
        consume_and_assert(__LINE__, ';');
      }
    }
    return new_node_if_stmt(cond, body, else_clause);
  }
  return NULL;
}

Node *while_stmt() {
  if (consume_keyword("while")) {
    consume_and_assert(__LINE__, '(');
    Node *cond = assign();
    consume_and_assert(__LINE__, ')');

    Vector *body = NULL;
    if (consume('{')) {
      body = stmt();
      consume_and_assert(__LINE__, '}');
    } else {
      body = new_vector();
      vec_push(body, assign());
      consume_and_assert(__LINE__, ';');
    }
    return new_node_while_stmt(cond, body);
  }
  return NULL;
}

Node *for_stmt() {
  if (consume_keyword("for")) {
    consume_and_assert(__LINE__, '(');
    Node *init= assign();
    consume_and_assert(__LINE__, ';');
    Node *cond = assign();
    consume_and_assert(__LINE__, ';');
    Node *updater = assign();
    consume_and_assert(__LINE__, ')');

    Vector *body = NULL;
    if (consume('{')) {
      body = stmt();
      consume_and_assert(__LINE__, '}');
    } else {
      body = new_vector();
      vec_push(body, assign());
      consume_and_assert(__LINE__, ';');
    }
    return new_node_for_stmt(init, cond, updater, body);
  }
  return NULL;
}

Node *ret() {
  if (consume_keyword("return")) {
    Node *node = assign();
    consume_and_assert(__LINE__, ';');
    return new_node_ret(node);
  }
  return NULL;
}

Node *var_decl() {
  if (consume_keyword("int")) {
    Node *node = ident();
    return new_node_var_decl(node);
  }
  return NULL;
}

Vector *stmt(Context *ctx) {
  Vector *stmts = new_vector();
  while(current_token()->ty != '}') {
    Node *node;

    node = if_stmt();
    if (node != NULL) {
      vec_push(stmts, node);
      continue;
    }

    node = while_stmt();
    if (node != NULL) {
      vec_push(stmts, node);
      continue;
    }

    node = for_stmt();
    if (node != NULL) {
      vec_push(stmts, node);
      continue;
    }

    node = ret();
    if (node != NULL) {
      vec_push(stmts, node);
      continue;
    }

    node = var_decl();
    if (node != NULL) {
      printf("# var decl found\n");
      vec_push(stmts, node);
      vec_push(ctx->vars, node->name);
      consume_and_assert(__LINE__, ';');
      continue;
    }

    vec_push(stmts, assign());
    consume_and_assert(__LINE__, ';');
  }
  return stmts;
}

Vector *formal_args() {
  if (consume('(')) {
    if (current_token()->ty == ')') {
      pos++;
      return NULL;
    }
    Vector *arguments = new_vector();
    consume_keyword_and_assert(__LINE__, "int");
    vec_push(arguments, ident());
    while(consume(',')) {
      consume_keyword_and_assert(__LINE__, "int");
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

Node *fn_decl(Context *ctx) {
  int last_pos = pos;
  if (!consume_keyword("int")) return NULL;
  Node *fn_name = ident();
  if (fn_name != NULL && current_token()->ty == '(') {
    Node *args = formal_args();
    if (consume('{')) {
      Context *local_ctx = new_context(fn_name->name);
      local_ctx->parent = ctx;
      Vector *body = stmt(local_ctx);
      if (consume('}')) {
        return new_node_fn_decl(local_ctx, fn_name->name, args, body);
      }
      error(__LINE__, "no corresponding closing brace %s", current_token()->input);
    }
  }
  error(__LINE__, "no corresponding closing paren %s", current_token()->input);
}

void program(Context *ctx) {
  int i = 0;
  Token *token = tokens->data[pos];
  while (token->ty != TK_EOF) {
    code[i++] = fn_decl(ctx);
    token = tokens->data[pos];
  }
  code[i] = NULL;
}

void parse(Context *ctx) {
  program(ctx);
}
