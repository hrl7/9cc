#include "9cc.h"

int pos;
Vector *tokens;
Node *code[100];
Map *variables;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }
  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }

  variables = new_map();
  tokens = new_vector();
  tokenize(argv[1]);
  program();

  printf(".intel_syntax noprefix\n");
#ifdef __APPLE__
  printf(".global _main\n");
  printf("_main:\n");
#else
  printf(".global main\n");
  printf("main:\n");
#endif

  printf("\n# function prolog\n");
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", variables->keys->len * 8);

  for (int i = 0; code[i]; i++) {
    printf("\n# statement: %d\n", i);
    gen(code[i]);
    //printf("  pop rax\n");
  }

  printf("\n# function epilog\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
