#include "9cc.h"

extern void gen(Node *node);

void error_node(int line, char *str, int node_type) {
  fprintf(stderr, "\n %s %d:, %s. node_type: %d %c\n", __FILE__, line, str, node_type, node_type);
  printf("\n %s %d:, %s. node_type: %d %c\n", __FILE__, line, str, node_type, node_type);
  exit(1);
}

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT && node->ty != ND_ARG && node->ty != ND_DEREF) {
    error_node(__LINE__, "left value is not variable", node->ty);
  }
  char *name;
  if (node->ty == ND_IDENT || node->ty == ND_ARG) {
    name = node->name;
  } else if (node->ty == ND_DEREF) {
    name = node->lhs->name;
  }
  Record *rec = map_get(variables, name);
  if (rec == NULL) {
    fprintf(stderr, "undefined variable %s\n", name);
    printf("undefined variable %s\n", name);
    exit(1);
  }
  printf("  mov rax, rbp # \n", rec->offset);
  printf("  sub rax, %d# address of var %s\n", rec->offset, name);
  while (node != NULL && node->ty == ND_DEREF) {
    printf("  mov rax, [rax]\n");
    node = node->lhs;
  }
  printf("  push rax\n");
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
      rec->offset = offset;
      printf("# local vars %d, %s, offset:%d ,type:%d\n", i, rec->name, rec->offset, rec->type->ty);
      map_put(variables, rec->name, rec);
      offset += 8;
    };
    if (args != NULL) {
      Node *arg_node;
      int bp_offset;
      for (int i = 0; i < num_args; i++) {
        arg_node = args->data[i];
        bp_offset = offset + 8 * i;
        rec = new_record(arg_node->name, bp_offset, arg_node->data_type);
        map_put(variables, arg_node->name, rec);
        printf("# arg %d, %s, offset:%d ,type:%d\n", i, rec->name, rec->offset, rec->type->ty);
        if (i == 0) printf("  mov [rbp-%d], rdi # 1st arg\n", bp_offset);
        if (i == 1) printf("  mov [rbp-%d], rsi # 2nd arg\n", bp_offset);
        if (i == 2) printf("  mov [rbp-%d], rdx # 3rd arg\n", bp_offset);
        if (i == 3) printf("  mov [rbp-%d], rcx # 4th arg\n", bp_offset);
        if (i == 4) printf("  mov [rbp-%d], r8 # 5th arg\n", bp_offset);
        if (i == 5) printf("  mov [rbp-%d], r9 # 6th arg\n", bp_offset);
      }
      offset = bp_offset;
    }
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
    if (rec == NULL) {
      fprintf(stderr, "undefined variable %s\n", node->name);
      printf("undefined variable %s\n", node->name);
      exit(1);
    }
    printf("  mov rax, [rbp-%d] # var %s\n", rec->offset, node->name);
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
    printf("  mov [rax], rdi\n");
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
      printf(" # node->lhs->type: %d\n", node->lhs->ty);
      printf(" # node->rhs->type: %d\n", node->rhs->ty);
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
      printf("  cmp rdi, rax\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      printf("  cmp rdi, rax\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rdi, rax\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_GE:
      printf("  cmp rdi, rax\n");
      printf("  setge al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  sub rax, 1\n");
      printf("  cmp rdi, rax\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_GT:
      printf("  add rax, 1\n");
      printf("  cmp rdi, rax\n");
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
