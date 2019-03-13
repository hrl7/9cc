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

  Type *t;
  if (node->ty == ND_IDENT || node->ty == ND_VAR_DECL) {
    switch(node->data_type->ty) {
      case INT:
        return "int variable";
      case PTR:
        t = map_get(ctx->vars, node->name);
        return debug_ptr_type(t);
    }
  }
  printf("# unexpected node type :%d\n", node->ty);
  return "";
}

void traverse_node(Context *ctx, Node *node) {
  switch(node->ty) {
    case ND_FN_DECL:
      printf("# fn decl %s\n", node->name);
      return traverse_nodes(node->ctx, node->body);
    case ND_FN_CALL:
      return;
    case ND_VAR_DECL:
      printf("# var decl %s: %s\n", node->name, debug_type(ctx, node));
      return;
    case ND_ADDRESS:
      printf("# var decl\n");
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
