#include "9cc.h"

int pos;
int branch_id = 0;
Vector *tokens;
Vector *scopes;
Node *code[100];
Map *variables;
Context *global_ctx;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }
  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }

  global_ctx = new_context("global");
  scopes = new_vector();
  printf("# src => %s\n", argv[1]);
  variables = new_map();
  tokens = new_vector();

  tokenize(argv[1]);
  printf("# finish tokenize\n");

  parse(global_ctx);
  printf("# finish parsing\n");

  post_process(global_ctx, code);

  printf(".intel_syntax noprefix\n");
#ifdef __APPLE__
  printf(".global _main\n");
#else
  printf(".global main\n");
#endif

  for (int i = 0; code[i]; i++) {
    gen_fn_decl(code[i]);
  }

/*

  printf("\n# function prologue\n");
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", variables->keys->len * 8);


  printf("\n# function epilogue\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  */
  return 0;
}
