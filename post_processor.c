#include "9cc.h"

char *debug_type(Context *ctx, Node *node) {
  if (node->ty == ND_NUM) {
    return "number literal";
  }

  if (node->ty == ND_IDENT || node->ty == ND_VAR_DECL) {
    switch(node->data_type->ty) {
      case INT:
        return "int variable";
      case PTR:
        return "pointer";
    }
  }
  printf("# unexpected node type :%d\n", node->ty);
  return "";
}

void traverse_node(Context *ctx, Node *node) {
  switch(node->ty) {
    case ND_FN_DECL:
      printf("# fn decl %s\n", node->name);
      return traverse_nodes(ctx, node->body);
    case ND_FN_CALL:
      return;
    case ND_VAR_DECL:
      printf("# var decl %s: %s\n", node->name, debug_type(ctx, node));
      return;
    case ND_ADDRESS:
      printf("# var decl\n");
      return;
    case ND_IDENT:
      printf("# ident %s, type: %s\n", node->name, debug_type(ctx, node));
      return;
    case '=':
      printf("# assignment left hand type: %s\n", debug_type(ctx,node->lhs));
      traverse_node(ctx, node->lhs);
      traverse_node(ctx, node->rhs);
      return;
    case '+':
    case '-':
      printf("# +,- left hand type: %s right hand type: %s\n",
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
