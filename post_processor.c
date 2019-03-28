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
  if (type->ty == ARRAY) {
    char *type_name = debug_ptr_type(type->ptr_of);
    char *str = malloc(sizeof(char) * 100);
    strcpy(str, "array of ");
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
      case ARRAY:
        return debug_ptr_type(rec->type);
      default:
        printf("%s %d: unexpected type :%d\n",__FILE__, __LINE__, rec->type->ty);
        fprintf(stderr, "%s %d:, nexpected type :%d\n", __FILE__, __LINE__, rec->type->ty);
        exit(1);

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

  if (node->ty == '+' || node->ty == '-' || node->ty == '*' || node->ty == '/') {
    char *l = debug_type(ctx, node->lhs);
    char *r = debug_type(ctx, node->rhs);
    char *type_name = malloc(sizeof(char) * (strlen(r) + strlen(l) + 7));
    strcpy(type_name, "expr:");
    strcat(type_name, l);
    strcat(type_name, &(node->ty));
    strcat(type_name, r);
    return type_name;
  }

  if (node->ty == ND_VAR_DECL) {
    if (node->args != NULL) {
      printf("# args length %d\n", node->args->len);
    }
    return "args";
  }
  printf("%s %d: unexpected node type :%d %c\n",__FILE__, __LINE__, node->ty, node->ty);
  fprintf(stderr, "%s %d:, nexpected node type :%d %c\n", __FILE__, __LINE__, node->ty, node->ty);
  exit(1);
}

// returns addr width. if give node is not addr value, return 0
int get_addr_width(Context *ctx, Node *node) {
  if (node->ty == ND_NUM ||
      node->ty == ND_EQ || node->ty == ND_NEQ ||
      node->ty == ND_GE || node->ty == ND_LE ||
      node->ty == ND_GT || node->ty == ND_LT ||
      node->ty == '*' || node->ty == '/') {
    return 0;
  }

  if (node->ty == ND_IDENT) {
    Record *rec = get_record(ctx, node->name);
    if (rec->type->ty == INT) {
      return 0;
    }
    if (rec->type->ty == PTR || rec->type->ty == ARRAY) {
      Type *t = rec->type->ptr_of;
      if (t == NULL) {
        printf("%s, %d: unexpected type\n", __FILE__, __LINE__);
        exit(1);
      }

      switch(t->ty) {
        case INT:
          return 4;
        case PTR:
          return 8;
        default:
          printf("%s, %d: unexpected type\n", __FILE__, __LINE__);
          exit(1);
      }
    }
  }

  if (node->ty == ND_DEREF) {
    node = node->lhs;
    printf("# get addr width DEREF %d %c\n", node->ty, node->ty);
    if (node->ty == ND_IDENT) {
      Record *rec = get_record(ctx, node->name);
      if (rec->type->ptr_of->ty == INT) {
        return 0;
      }
    } else {
      if (get_addr_width(ctx, node) <= 4) {
        return 0;
      };
    }
    printf("%s, %d: not implemented yet for node type %d %c\n", __FILE__, __LINE__, node->ty, node->ty);
    exit(1);
  }

  if (node->ty == '+' || node->ty == '-') {
    int lhs_width = get_addr_width(ctx, node->lhs);
    int rhs_width = get_addr_width(ctx, node->rhs);
    printf("# lhs width %d, rhs width %d\n", lhs_width, rhs_width);
    if (lhs_width > rhs_width) {
      return lhs_width;
    }
    return rhs_width;
  }
  printf("%s, %d: not implemented node type %d %c\n", __FILE__, __LINE__, node->ty, node->ty);
  exit(1);
}

void traverse_fn_decl(Context *ctx, Node *node) {
  // Map<char*, Record>
  Map *variables = ctx->vars;
  Vector *args = node->args;
  int num_args = args == NULL ? 0 : args->len;
  int offset = 8;
  char *var_name;
  Record *rec;
  if (variables != NULL) {
    for (int i = 0; i < variables->keys->len; i++) {
      var_name = variables->keys->data[i];
      printf("# local vars %d %s, offset: %d\n", i, var_name, offset);
      rec = map_get(variables, var_name);
      if (rec == NULL) {
        fprintf(stderr, "got invalid ident %s\n", var_name);
        printf("got invalid ident %s at postprocess\n", var_name);
        exit(1);
      }
      offset += (int)get_data_width_by_record(rec);
      rec->offset = offset;
      if (rec->type->ty == ARRAY) {
        printf("# array_of  size: %d, array_size %d\n",  get_data_width_by_type(rec->type->ptr_of), rec->type->array_size);
        offset += rec->type->array_size * get_data_width_by_type(rec->type->ptr_of);
      }
    };
  }
  if (args != NULL) {
    Node *arg_node;
    int bp_offset;
    for (int i = 0; i < num_args; i++) {
      arg_node = args->data[i];
      bp_offset = offset + 8 * i;
      var_name = arg_node->name;
      rec = new_record(var_name, bp_offset, arg_node->data_type, 1);
      map_put(variables, var_name, rec);
      printf("# arg %d, %s, offset: %d at post process\n", i, arg_node->name, bp_offset);
    }
    offset = bp_offset;
  }
  traverse_nodes(node->ctx, node->body);
}

void traverse_node(Context *ctx, Node *node) {
  switch(node->ty) {
    case ND_FN_DECL:
      printf("# fn decl %s\n", node->name);
      return traverse_fn_decl(node->ctx, node);
    case ND_FN_CALL:
      return;
    case ND_VAR_DECL:
      printf("# var decl %s: %s\n", node->name, debug_type(ctx, node));
      return;
    case ND_RET:
      traverse_node(ctx, node->body);
      return;
    case ND_DEREF:
      traverse_node(ctx, node->lhs);
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
      int lhs_width = get_addr_width(ctx, node->lhs);
      printf("# node->lhs width %d\n", lhs_width);
      int rhs_width = get_addr_width(ctx, node->rhs);
      printf("# node->rhs width %d\n", rhs_width);
      if (lhs_width != 0 && rhs_width == 0) {
        printf("# multiply rhs\n");
        node->rhs = new_node('*', new_node_num(lhs_width), node->rhs);
        traverse_node(ctx, node->lhs);
        return;
      }
      if (rhs_width != 0 && lhs_width == 0) {
        printf("# multiply lhs\n");
        node->lhs = new_node('*', new_node_num(rhs_width), node->lhs);
        traverse_node(ctx, node->rhs);
        return;
      }
      traverse_node(ctx, node->lhs);
      traverse_node(ctx, node->rhs);
      return;

    case ND_REF:
    case ND_IF:
    case ND_WHILE:
    case ND_FOR:
    case ND_EQ:
    case ND_NEQ:
    case ND_LT:
    case ND_GT:
    case ND_LE:
    case ND_GE:
    default:
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
