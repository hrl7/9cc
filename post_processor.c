#include "9cc.h"

extern void traverse_node(Context *ctx, Node *node);
extern void traverse_nodes(Context *ctx, Vector *nodes);

char *debug_ptr_type(Type *type) {
  if (type->ty == INT) {
    return "int";
  }
  if (type->ty == PTR) {
    char *type_name = debug_ptr_type(type->ptr_of);
    char *str = malloc(sizeof(char) * 100);
    strcpy(str, "pointer of ");
    strcat(str, type_name);
    return str;
  }
}

char *debug_type(Context *ctx, Node *node) {
  if (node->ty == ND_NUM) {
    return "number literal";
  }

  Record *rec;
  if (node->ty == ND_IDENT || node->ty == ND_VAR_DECL) {
    rec = get_record(ctx, node->name);
    if (rec == NULL || rec->type == NULL) {
      fprintf(stderr, "# unexpected record for %s\n", node->name);
      printf("# unexpected record for %s\n", rec->type);
      exit(1);
    }
    switch(rec->type->ty) {
      case INT:
        return "int variable";
      case PTR:
        return debug_ptr_type(rec->type);
    }
  }
  if (node->ty == ND_DEREF) {
    Node *term = node->lhs;
    char *r = debug_type(ctx, term);
    char *type_name = malloc(sizeof(char) * (strlen(r) + 10));
    strcpy(type_name, "deref of ");
    strcat(type_name, r);
    return type_name;
  }
  printf("# unexpected node type :%d %c\n", node->ty, node->ty);
  fprintf(stderr, "# unexpected node type :%d\n", node->ty);
  exit(1);
}

void traverse_fn_decl(Context *ctx, Node *node) {
  // Map<char*, Record>
  printf("# node ty %d ctx: %x %s\n", node->ty, ctx, node->ctx->name);
  Map *variables = ctx->vars;
  Vector *args = node->args;
  int num_args = args == NULL ? 0 : args->len;
  int offset = 8;
  char *var_name;
  Record *rec;
  for (int i = 0; i < variables->keys->len; i++) {
    var_name = variables->keys->data[i];
    printf("# local vars %d %s\n", i, var_name);
    rec = map_get(variables, var_name);
    printf("# %s\n", rec->name);
    if (rec == NULL) {
      fprintf(stderr, "got invalid ident %s\n", var_name);
      printf("got invalid ident %s at postprocess\n", var_name);
      exit(1);
    }
    rec->offset = offset;
    printf("# traverse fn decl rec->offset %d\n", rec->offset);
    offset += 8;
  };
  if (args != NULL) {
    Node *arg_node;
    int bp_offset;
    for (int i = 0; i < num_args; i++) {
      arg_node = args->data[i];
      bp_offset = offset + 8 * i;
      var_name = arg_node->name;
      rec = new_record(var_name, bp_offset, arg_node->data_type);
      map_put(variables, arg_node->name, rec);
      printf("# arg %d, %s at post process\n", i, arg_node->name);
    }
    offset = bp_offset;
  }
  traverse_nodes(node->ctx, node->body);
}

void traverse_node(Context *ctx, Node *node) {
  switch(node->ty) {
    case ND_FN_DECL:
      printf("# fn decl %s\n", node->name);
      printf("# ctx: %x\n", ctx);
      return traverse_fn_decl(ctx, node);
    case ND_FN_CALL:
      return;
    case ND_VAR_DECL:
      printf("# var decl %s: %s\n", node->name, debug_type(ctx, node));
      return;
    case ND_REF:
      printf("# reference\n");
      return;
    case ND_IDENT:
      printf("# identifier %s, type: %s\n", node->name, debug_type(ctx, node));
      return;
    case '=':
      printf("# assignment left hand type: %s\n", debug_type(ctx,node->lhs));
      traverse_node(ctx, node->lhs);
      traverse_node(ctx, node->rhs);
      return;
    case '+':
    case '-':
      printf("# +,- left hand type: %s, right hand type: %s\n",
        debug_type(ctx, node->lhs),
        debug_type(ctx, node->rhs));
      traverse_node(ctx, node->lhs);
      traverse_node(ctx, node->rhs);
      return;

    case ND_IF:
      printf("# if\n");
      return;
    case ND_WHILE:
      printf("# while\n");
      return;
    case ND_FOR:
      printf("# for \n");
      return;
    case ND_RET:
      printf("# return\n");
      return;

    case ND_EQ:
    case ND_NEQ:
    case ND_LT:
    case ND_GT:
    case ND_LE:
    case ND_GE:
      printf("# if\n");
      return;

  }
}

void traverse_nodes(Context *ctx, Vector *nodes) {
  for(int i = 0; i < nodes->len; i++) {
    traverse_node(ctx, nodes->data[i]);
  }
}

void post_process(Context *ctx, Node **codes) {
  printf("# start post process\n");
  for(int i = 0; codes[i]; i++) {
    printf("# code [%d]\n", i);
    traverse_node(ctx, codes[i]);
  }
  printf("# finish post process\n");
}
