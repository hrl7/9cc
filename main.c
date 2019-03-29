#include "9cc.h"

int pos, branch_id = 0;
Vector *tokens, *scopes;
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
  global_ctx->parent = NULL;
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

  Node *node, **fns, **vars;
  Record *rec;
  int num_fn_decls = 1, num_global_vars = 1;
  for (int i = 0; code[i]; i++) {
    node = code[i];
    if (node->ty == ND_FN_DECL) num_fn_decls++;
    if (node->ty == ND_VAR_DECL) {
      rec = map_get(global_ctx->vars, node->name);
      if (rec != NULL) num_global_vars++;
    }
  }

  fns = malloc(sizeof(Node *) * num_fn_decls);
  vars = malloc(sizeof(Node *) * num_global_vars);

  int fi = 0, vi = 0;
  for (int i = 0; code[i]; i++) {
    node = code[i];
    if (node->ty == ND_FN_DECL) fns[fi++] = node;
    if (node->ty == ND_VAR_DECL && map_get(global_ctx->vars, node->name) != NULL) {
      vars[vi++] = node;
    }
  }

  fns[fi] = NULL;
  vars[vi] = NULL;

  for (int i = 0; fns[i]; i++) {
    gen_fn_decl(fns[i]);
  }

  if (vi != 0) {
    printf(".bss\n");
    for (int i = 0; vars[i]; i++) {
      gen_global_var_decl(vars[i]);
    }
  }
  return 0;
}
