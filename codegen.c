#include "9cc.h"

extern void gen(Node *node);

void error_node(int line, char *str, int node_type) {
  fprintf(stderr, "\n %s %d:, %s. node_type: %d %c\n", __FILE__, line, str, node_type, node_type);
  printf("\n %s %d:, %s. node_type: %d %c\n", __FILE__, line, str, node_type, node_type);
  exit(1);
}

void assert_rec_exist(Record *rec, char *name) {
  if (rec == NULL) {
    fprintf(stderr, "undefined variable %s\n", name);
    printf("undefined variable %s\n", name);
    exit(1);
  }
}

void not_implemented_yet(int line) {
    fprintf(stderr, "%s %d: not implemented yet\n", __FILE__, line);
    printf("%s %d: not implemented yet\n", __FILE__, line);
    exit(1);
}

Record *lookup_var(char *name) {
  Record *rec = map_get(variables, name);
  if (rec == NULL) {
    rec = map_get(global_ctx->vars, name);
  }
  assert_rec_exist(rec, name);
  return rec;
}

int is_global_var(Record *rec) {
  return rec->offset == -1;
}

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT && node->ty != ND_ARG && node->ty != ND_DEREF) {
    error_node(__LINE__, "left value is not variable", node->ty);
  }
  if (node->ty == ND_IDENT || node->ty == ND_ARG) {
    char *name = node->name;
    Record *rec = lookup_var(name);
    if (is_global_var(rec)) {
      printf("  lea rax, %s[rip]\n", name);
    } else {
      printf("  mov rax, rbp \n");
      printf("  sub rax, %d# address of var %s\n", rec->offset, name);
    }
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_DEREF) {
    int node_ty = node->lhs->ty;
    if (node_ty == ND_IDENT) {
      char *name = node->lhs->name;
      Record *rec = lookup_var(name);
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d# address of var %s\n", rec->offset, name);
      while (node != NULL && node->ty == ND_DEREF) {
        printf("  mov rax, [rax]\n");
        node = node->lhs;
      }
      printf("  push rax\n");
      return;
    }

    if (node_ty == '+' || node_ty == '-') {
        gen(node->lhs);
        return;
    }
  }
  not_implemented_yet(__LINE__);
}

int get_str_id(char *str) {
  printf("#search str %s\n", str);
  for(int i = 0;i < strings->len; i++) {
    if (strcmp(strings->data[i], str) == 0) {
      return i;
    }
  }
  return -1;
}

size_t get_data_width_by_type(Type *type) {
  switch(type->ty) {
    case CHAR:
      return 1;
    case INT:
      return 4;
    case PTR:
    case ARRAY:
      return 8;
    default:
      printf("%s %d: unexpected type %d\n", __FILE__, __LINE__, type->ty);
      exit(1);
  }
}
size_t get_data_width_by_record(Record *rec) {
  return get_data_width_by_type(rec->type);
}

size_t get_data_width(Node *node) {
  if (node->ty == ND_IDENT) {
    Record *rec = lookup_var(node->name);
    return get_data_width_by_type(rec->type);
  }

  if (node->ty == ND_REF) {
    return 8;
  }

  if (node->ty == ND_DEREF) {
    return get_data_width(node->lhs);
  }

  if (node->ty == ND_NUM) {
    return 4;
  }

  if (node->ty == '+' || node->ty == '-') {
    size_t lhs = get_data_width(node->lhs);
    size_t rhs = get_data_width(node->rhs);
    if (lhs > rhs) {
      return rhs;
    }
    return lhs;
  }

  printf("%s %d: unexpected node type %d : %c\n", __FILE__, __LINE__, node->ty, node->ty);
  exit(1);
}

void gen_fn_decl(Node *node) {
  if (node->ty != ND_FN_DECL) {
    printf("%s %d: unexpected node type %d : %c\n", __FILE__, __LINE__, node->ty, node->ty);
    exit(1);
  }
  printf("%s:\n", node->name);
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");

  variables = new_map();
  Vector *args = node->args;
  // Map<char*, Record>
  Map *local_vars = node->ctx->vars;
  int num_args = args == NULL ? 0 : args->len;
  int offset = 8;
  char *var_name;
  Record *rec;
  for (int i = 0; i < local_vars->keys->len; i++) {
    rec = local_vars->vals->data[i];
    printf("# local vars %d, %s, offset:%d ,type:%d\n", i, rec->name, rec->offset, rec->type->ty);
    if (rec->is_arg) continue;
    map_put(variables, rec->name, rec);
    offset = rec->offset;
  };
  if (args != NULL) {
    Node *arg_node;
    for (int i = 0; i < num_args; i++) {
      arg_node = args->data[i];
      var_name = arg_node->name;
      rec = map_get(local_vars, var_name);
      assert_rec_exist(rec, var_name);
      offset = rec->offset;
      map_put(variables, var_name, rec);
      printf("# arg %d, %s, offset:%d ,type:%d\n", i, var_name, offset, arg_node->data_type->ty);
      if (i == 0) printf("  mov [rbp-%d], rdi # 1st arg\n", offset);
      if (i == 1) printf("  mov [rbp-%d], rsi # 2nd arg\n", offset);
      if (i == 2) printf("  mov [rbp-%d], rdx # 3rd arg\n", offset);
      if (i == 3) printf("  mov [rbp-%d], rcx # 4th arg\n", offset);
      if (i == 4) printf("  mov [rbp-%d], r8 # 5th arg\n", offset);
      if (i == 5) printf("  mov [rbp-%d], r9 # 6th arg\n", offset);
    }
  }
  for (int i = 0; i < local_vars->keys->len; i++) {
    rec = local_vars->vals->data[i];
    if(rec->type->ty == ARRAY) {
      printf("# i: %d, found array %s, type %d, size: %d\n", i, rec->name, rec->type->ty, (int)rec->type->array_size);
      offset = rec->offset + 8 + (int)get_data_width_by_record(rec) * rec->type->array_size;
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d # last elem of %s\n", offset, rec->name);
      printf("  mov [rbp-%d], rax\n", rec->offset);
    }
  };
  printf("  sub rsp, %d\n", offset);
  Vector *body = node->body;
  if (body != NULL) {
    for (int i = 0; i < body->len; i++) {
      printf("# in function %s, statement: %d\n", node->name, i);
      gen(body->data[i]);
    }
  }
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp # fn epilogue\n");
  printf("  ret\n\n");
  return;
}

void gen_if(Node *node) {
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 1\n");
  int end_label_id = branch_id++;
  int else_label_id;

  if (node->els != NULL ) {
    else_label_id = branch_id++;
    printf("  jne .L%d # jump to else clause\n", else_label_id);
  } else {
    printf("  jne .L%d # jump to end if\n", end_label_id);
  }

  Vector *body = node->body;
  for (int i = 0; i < body->len; i++) {
    gen(body->data[i]);
  }

  if (node->els != NULL && node->els->len > 0) {
    printf("  jmp .L%d # jump to end clause\n", end_label_id);
    Vector *els = node->els;
    printf(".L%d: # else clause\n", else_label_id);
    for (int i = 0; i < els->len; i++) {
      gen(els->data[i]);
    }
  }

  printf(".L%d: # end if clause\n", end_label_id);
}

void gen_while(Node *node) {
  int cond_label_id = branch_id++;
  int body_label_id = branch_id++;
  printf("  jmp .L%d\n", cond_label_id);
  printf(".L%d: # while-body\n", body_label_id);
  Vector *body = node->body;
  for (int i = 0; i < body->len; i++) {
    gen(body->data[i]);
  }
  printf(".L%d: # while-cond\n", cond_label_id);
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 1\n");
  printf("  je .L%d # jump to while-body\n", body_label_id);
}

void gen_for(Node *node) {
  int cond_label_id = branch_id++;
  int updater_label_id = branch_id++;
  int body_label_id = branch_id++;

  gen(node->init);
  printf("  jmp .L%d\n", cond_label_id);
  printf(".L%d: # for-body\n", body_label_id);
  Vector *body = node->body;
  for (int i = 0; i < body->len; i++) {
    gen(body->data[i]);
  }
  printf(".L%d: # for-updater\n", updater_label_id);
  gen(node->updater);
  printf(".L%d: # for-cond\n", cond_label_id);
  gen(node->cond);
  printf("  pop rax\n");
  printf("  cmp rax, 1\n");
  printf("  je .L%d # jump to for-updater\n", body_label_id);
}

void gen_fn_call(Node *node) {
  int num_args = 0;
  if (node->args != NULL) {
    num_args = node->args->len;
    Node *arg_node;
    for (int i = num_args - 1; i >= 0; i--) {
      arg_node = node->args->data[i];
      gen(arg_node);
      if (i == 0) printf("  mov rdi, [rsp] # 1st arg\n");
      if (i == 1) printf("  mov rsi, [rsp] # 2nd arg\n");
      if (i == 2) printf("  mov rdx, [rsp] # 3rd arg\n");
      if (i == 3) printf("  mov rcx, [rsp] # 4th arg\n");
      if (i == 4) printf("  mov r8, [rsp] # 5th arg\n");
      if (i == 5) printf("  mov r9, [rsp] # 6th arg\n");
      printf("  mov al, 0\n");
    }
    printf("  add rsp, %d\n", num_args * 8);
  }
  printf("  call %s\n", node->name);
  printf("  push rax\n");
}

void gen_global_var_decl(Node *node) {
  variables = global_ctx->vars;
  Record *rec = lookup_var(node->name);
  printf("%s:\n", rec->name);
  int width = (int)get_data_width_by_record(rec);
  if (node->init == NULL) {
    printf("  .zero %d\n", width);
    return;
  }

  Node *init = node->init;
  if (init->ty == ND_NUM || init->ty == ND_CHAR) {
    switch (width) {
      case 1:
        printf("  .byte %d\n", init->val);
        return;
      case 4:
        printf("  .long %d\n", init->val);
        return;
      default:
        not_implemented_yet(__LINE__);
    }
  }
}

Node *find_ptr_node(Node *node) {
  printf("# ptr_node %d\n", node->ty);
  Record *rec;
  switch(node->ty) {
    case ND_IDENT:
      return node;
    case '+':
    case '-': {
      if (node->lhs->ty == ND_IDENT) {
        rec = lookup_var(node->lhs->name);
        if (rec->type->ty == PTR || rec->type->ty == ARRAY) {
          return node->lhs;
        }
      }
      if (node->rhs->ty == ND_IDENT) {
        rec = lookup_var(node->rhs->name);
        if (rec->type->ty == PTR || rec->type->ty == ARRAY) {
          return node->rhs;
        }
      }
    }
  }
  return NULL;
}

int get_derefed_size(Node *node) {
  printf("# get derefed size %d %c\n", node->ty, node->ty);
  Node *ptr_node = find_ptr_node(node);
  if (ptr_node == NULL) {
    printf("# cannot find ptr_node\n");
    return -1;
  }
  Record *rec = lookup_var(ptr_node->name);
  switch(rec->type->ptr_of->ty) {
    case INT:
      return 4;
    case CHAR:
      return 1;
    case PTR:
    case ARRAY:
      return 8;
  }
  return -1;
}

void gen(Node *node) {
  if (node->ty == ND_VAR_DECL) {
    Record *rec = lookup_var(node->name);
    if (rec->offset == -1) {
      gen_global_var_decl(node);
    }
    return;
  }
  if (node->ty == ND_FN_DECL) {
    gen_fn_decl(node);
    return;
  }
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }
  if (node->ty == ND_CHAR) {
    printf("  push %d\n", node->val);
    return;
  }
  if (node->ty == ND_STRING) {
    int id = get_str_id(node->str);
    printf("  mov rax, OFFSET FLAT:.LC%d\n", id);
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_IDENT) {
    Record *rec = lookup_var(node->name);
    int width = get_data_width_by_record(rec);
    if (rec->offset == -1) {
      switch(width) {
        case 1:
          printf("  xor rax, rax\n");
          printf("  mov ax, %s[rip] # var %s\n", node->name);
          break;
        case 4:
          printf("  xor rax, rax\n");
          printf("  mov eax, %s[rip] # var %s\n", node->name);
          break;
        case 8:
          printf("  mov rax, %s[rip] # var %s\n", node->name);
          break;
      }
    } else {
      switch(width) {
        case 1:
          printf("  xor rax, rax\n");
          printf("  mov ax, [rbp-%d] # var %s\n", rec->offset, node->name);
          break;
        case 4:
          printf("  xor rax, rax\n");
          printf("  mov eax, [rbp-%d] # var %s\n", rec->offset, node->name);
          break;
        case 8:
          printf("  mov rax, [rbp-%d] # var %s\n", rec->offset, node->name);
          break;
      }
    }
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_REF) {
    if (node->lhs != NULL && node->lhs->ty == ND_IDENT) {
      Record *rec = map_get(variables, node->lhs->name);
      if (rec == NULL) {
        fprintf(stderr, "undefined variable %s\n", node->name);
        printf("undefined variable %s\n", node->name);
        exit(1);
      }
      printf("  lea rax, [rbp-%d] # &%s\n", rec->offset, node->lhs->name);
      printf("  push rax\n");
      return;
    }
    printf("%s %d: expected IDENT, but got %c %d\n", __FILE__, __LINE__, node->lhs->ty, node->lhs->ty);
    exit(1);
  }

  if (node->ty == ND_DEREF) {
    gen(node->lhs);
    int width = get_derefed_size(node->lhs);
    printf("  mov rax, [rax]\n");
    printf("# data width: %d byte\n", width);
    switch(width) {
      case 1:
        printf("  mov rcx, 0xff\n");
        printf("  and rax, rcx\n");
        break;
      case 4:
        printf("  mov rcx, 0xffff\n");
        printf("  and rax, rcx\n");
        break;
    }
    printf("  add rsp, 8\n");
    printf("  push rax\n");
    printf("# node deref ^\n" );
    return;
  }

  if (node->ty == ND_IF) {
    gen_if(node);
    return;
  }

  if (node->ty == ND_WHILE) {
    gen_while(node);
    return;
  }

  if (node->ty == ND_FOR) {
    gen_for(node);
    return;
  }

  if (node->ty == ND_FN_CALL) {
    gen_fn_call(node);
    return;
  }

  if (node->ty == ND_RET) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp # fn epilogue\n");
    printf("  ret\n\n");
    return;
  }

  if (node->ty == '=') {
    printf("# assignment\n");
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    if (node->lhs->ty == ND_IDENT) {
      Record *rec = lookup_var(node->lhs->name);
      int width = get_data_width_by_record(rec);
      printf("# lhs width: %d\n", width);
      switch(width) {
        case 1:
          printf("  movb [rax], dil\n");
          break;
        case 4:
          printf("  mov [rax], edi\n");
          break;
        case 8:
          printf("  mov [rax], rdi\n");
          break;
      }
    } else {
      printf("  mov [rax], rdi\n");
    }
    printf("  push rdi\n");
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
    case ND_EQ:
      if ((int)get_data_width(node->lhs) == 4 || (int)get_data_width(node->rhs) == 4) {
        printf("  cmp edi, eax\n");
      } else {
        printf("  cmp rdi, rax\n");
      }
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      if ((int)get_data_width(node->lhs) == 4 || (int)get_data_width(node->rhs) == 4) {
        printf("  cmp edi, eax\n");
      } else {
        printf("  cmp rdi, rax\n");
      }
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      if ((int)get_data_width(node->lhs) == 4 || (int)get_data_width(node->rhs) == 4) {
        printf("  cmp edi, eax\n");
      } else {
        printf("  cmp rdi, rax\n");
      }
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_GE:
      if ((int)get_data_width(node->lhs) == 4 || (int)get_data_width(node->rhs) == 4) {
        printf("  cmp edi, eax\n");
      } else {
        printf("  cmp rdi, rax\n");
      }
      printf("  setge al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  sub rax, 1\n");
      if ((int)get_data_width(node->lhs) == 4 || (int)get_data_width(node->rhs) == 4) {
        printf("  cmp edi, eax\n");
      } else {
        printf("  cmp rdi, rax\n");
      }
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_GT:
      printf("  add rax, 1\n");
      if ((int)get_data_width(node->lhs) == 4 || (int)get_data_width(node->rhs) == 4) {
        printf("  cmp edi, eax\n");
      } else {
        printf("  cmp rdi, rax\n");
      }
      printf("  setge al\n");
      printf("  movzb rax, al\n");
      break;
    default:
      fprintf(stderr, "not implemented yet NODE Type :%d\n", node->ty);
      printf("not implemented yet NODE Type :%d\n", node->ty);
      exit(1);
  }
  printf("  push rax\n");
}
