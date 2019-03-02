#include "9cc.h"

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT) {
    error("left value is not variable", node->ty);
  }

  int offset = map_get_index(variables, node->name) * 8;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d # var %s\n", offset, node->name);
  printf("  push rax\n");
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_FN_CALL) {
    printf("  call %s\n", node->name);
    return;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
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
      printf("  cmp rdi, rax\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      printf("  cmp rdi, rax\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
  }
  printf("  push rax\n");
}
