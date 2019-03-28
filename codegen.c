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

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT && node->ty != ND_ARG && node->ty != ND_DEREF) {
    error_node(__LINE__, "left value is not variable", node->ty);
  }
  int offset = 0;
  if (node->ty == ND_IDENT || node->ty == ND_ARG) {
    char *name = node->name;
    Record *rec = map_get(variables, name);
    assert_rec_exist(rec, name);
    printf("  mov rax, rbp # \n", rec->offset);
    printf("  sub rax, %d# address of var %s\n", rec->offset, name);
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_DEREF) {
    int node_ty = node->lhs->ty;
    if (node_ty == ND_IDENT) {
      char *name = node->lhs->name;
      Record *rec = map_get(variables, name);
      assert_rec_exist(rec, name);
      printf("  mov rax, rbp # \n", rec->offset);
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

size_t get_data_width_by_type(Type *type) {
  switch(type->ty) {
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
    Record *rec = map_get(variables, node->name);
    assert_rec_exist(rec, node->name);
    return get_data_width_by_record(rec);
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
  if (node->ty == ND_FN_DECL) {
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
        printf("# args %d\n", i);
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
      rec = map_get(variables, rec->name);
      assert_rec_exist(rec, rec->name);
      if(rec->type->ty == ARRAY) {
        offset = rec->offset + 8 + (int)get_data_width_by_record(rec) * (rec->type->array_size - 1);
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
        Node *b = body->data[i];
        gen(body->data[i]);
      }
    }
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp # fn epilogue\n");
    printf("  ret\n\n");
    return;
  }
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
    }
    printf("  add rsp, %d\n", num_args * 8);
  }
  printf("  call %s\n", node->name);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->ty == ND_VAR_DECL) return;
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    Record *rec = map_get(variables, node->name);
    assert_rec_exist(rec, node->name);
    if ((int)get_data_width_by_record(rec) == 4) {
      printf("  mov eax, [rbp-%d] # var %s\n", rec->offset, node->name);
    } else {
      printf("  mov rax, [rbp-%d] # var %s\n", rec->offset, node->name);
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
    printf("%s %d: expected IDENT, but got %s\n", __FILE__, __LINE__, node->lhs->ty);
    exit(1);
  }

  if (node->ty == ND_DEREF) {
    gen(node->lhs);
    printf("  mov rax, [rax]\n");
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
    gen(node->body);
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
      Record *rec = map_get(variables, node->lhs->name);
      if (rec->type->ty == INT) {
        printf("  mov [rax], edi\n");
      } else {
        printf("  mov [rax], rdi\n");
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
