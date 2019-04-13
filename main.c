#include "9cc.h"
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

int pos, branch_id = 0;
Vector *tokens, *scopes, *strings;
Node *code[100];
Map *variables;
Context *global_ctx;

void setup_meta(Meta *meta) {
  meta->date = malloc(sizeof(char) * 12);
  meta->time = malloc(sizeof(char) * 9);
  time_t t = time(NULL);
  strftime(meta->date,12, "%b %d %Y", localtime(&t));
  strftime(meta->time, 9, "%H:%M:%S", localtime(&t));
  printf("# %s\n", meta->date);
  printf("# %s\n", meta->time);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }
  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }

  int length = strlen(argv[1]);
  char *fname = argv[1];
  char *ext = fname + length -2;
  char *src;

  Meta *meta;
  meta = malloc(sizeof(Meta));

  if (strcmp(ext, ".c") == 0) {
    struct stat *st = malloc(sizeof(struct stat));
    FILE *fp = fopen(fname, "r");
    if (fp == NULL) {
      fprintf(stderr, "failed to open file: %s\n", fname);
      exit(1);
    }
    if (stat(fname, st) != 0) {
      fprintf(stderr, "failed to get stat: %s, error: %s\n", fname, strerror(errno));
      exit(1);
    }
    meta->file_name = fname;
    src = malloc(sizeof(char) * st->st_size);
    fread(src, 1, st->st_size, fp);
  } else {
    src = argv[1];
  }

  global_ctx = new_context("global");
  global_ctx->parent = NULL;
  scopes = new_vector();
  strings = new_vector();
  //printf("# src => %s\n", src);
  variables = new_map();
  tokens = new_vector();

  tokenize(src);
  printf("# finish tokenize\n");

  setup_meta(meta);
  pre_process(meta, global_ctx, tokens);

  parse(global_ctx);
  printf("# finish parsing\n");

  post_process(global_ctx, code);

  printf(".intel_syntax noprefix\n");
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


  if (strings->len != 0) {
    printf(".section .data\n");
    char *str;
    for (int i = 0; i < strings->len; i++) {
      str = strings->data[i];
      printf(".LC%d:\n", i);
      printf("  .ascii \"%s\"\n", str);
      printf("  .byte 0\n");
    }
  }

  if (vi != 0) {
    printf(".section .data\n");
    for (int i = 0; vars[i]; i++) {
      gen_global_var_decl(vars[i]);
    }
  }
  printf(".section .text\n");
#ifdef __APPLE__
  printf(".global _main\n");
#else
  printf(".global main\n");
#endif

  for (int i = 0; fns[i]; i++) {
    gen_fn_decl(fns[i]);
  }

  return 0;
}
