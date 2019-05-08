#include "9cc.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

Token *pp_current_token(PreProcessor *p) {
  return p->tokens->data[p->pos];
}

Token *pp_consume_token(PreProcessor *p) {
  p->pos++;
  return p->tokens->data[p->pos];
}

int include_counts = 0;

void replace_predefined_macro(Meta *meta, Token *token) {
  if (strcmp(token->input, "__LINE__") == 0) {
    token->ty = TK_NUM;
    token->val = token->line_start;
    return;
  }
  if (strcmp(token->input, "__FILE__") == 0) {
    token->ty = TK_STRING;
    token->input = meta->file_name;
    return;
  }
  if (strcmp(token->input, "__DATE__") == 0) {
    token->ty = TK_STRING;
    token->input = meta->date;
    return;
  }
  if (strcmp(token->input, "__TIME__") == 0) {
    token->ty = TK_STRING;
    token->input = meta->time;
    return;
  }
}

void replace_defined_macro(PreProcessor *p, Token *token) {
  Vector *body = map_get(p->macros, token->input);
  if (body != NULL) {
    int pos = p->pos;
    vec_delete(p->tokens, pos);
    vec_insert(p->tokens, body, pos);
  }
  return;
}

Vector *collect_tokens_until_newline(PreProcessor *p) {
  Vector *tks = new_vector();
  Token *t = pp_current_token(p);
  int current_line = t->line_start;
  while (t != NULL && current_line == t->line_start) {
    vec_push(tks, t);
    t = pp_consume_token(p);
  }
  //printf("# collect tokens %d\n", tks->len);
  return tks;
}

void remove_tokens(PreProcessor *p, int index, int num) {
  for (int j = 0; j < num; j++) {
    vec_delete(p->tokens, index);
  }
}

// search next #endif and remove it
void remove_endif(PreProcessor *p) {
  int i = p->pos;
  Token *token = p->tokens->data[i];
  while(token != NULL) {
    printf("# check token type: %d %c\n", token->ty, token->ty);
    if (token->ty == '#') {
      Token * next_token = p->tokens->data[i+1];
      if (next_token->ty == TK_IDENT && strcmp(next_token->input, "endif") == 0) {
        remove_tokens(p, i, 2); // #, endif
        return;
      }
    }
    token = p->tokens->data[++i];
  }
}

void remove_until_endif(PreProcessor *p) {
  int i = p->pos;
  int start_pos = p->pos - 3;
  Token *token = p->tokens->data[i];
  while(token != NULL) {
    //printf("#@@@ check token %s\n", token->input);
    if (token->ty == '#') {
      Token * next_token = p->tokens->data[i+1];
      if (next_token->ty == TK_IDENT && strcmp(next_token->input, "endif") == 0) {
        remove_tokens(p, start_pos, i - start_pos + 2); // #, endif
        p->pos = start_pos - 1;
        return;
      }
    }
    token = p->tokens->data[++i];
  }
}
/*
  bool: cmp

  if: 'if' add endif

  add: cmp || add
  add: cmp && add
  add: cmp + add
  add: cmp

  cmp: term > term
  cmp: term < term

  term: ( add )
  term: MACRO(arguments)
  term: MACRO add

  arguments: add , add
  arguments: add
*/

PPNode *pp_new_node(int ty, PPNode *left, PPNode *right) {
  PPNode *node = malloc(sizeof(PPNode));
  node->ty = ty;
  node->body = NULL;
  node->left = left;
  node->right = right;
  return node;
}

PPNode *pp_new_node_num(int val) {
  PPNode *node = malloc(sizeof(PPNode));
  node->ty = PP_NUM;
  node->body = NULL;
  node->right = NULL;
  node->left = NULL;
  node->val = val;
  return node;
}

PPNode *pp_new_node_bool(int val) {
  PPNode *node = malloc(sizeof(PPNode));
  node->ty = PP_BOOL;
  node->body = NULL;
  node->right = NULL;
  node->left = NULL;
  node->val = val != 0 ? 1 : 0;
  return node;
}


Token *ppi_current_token(PreProcessor *p) {
  PPContext *ctx = p->ctx;
  if (ctx->tokens->len <= ctx->pos) {
    printf("# token at pos: %d is NULL\n", ctx->pos);
    return NULL;
  }
  return ctx->tokens->data[ctx->pos];
}

int ppi_consume_ident(PreProcessor *p, const char *keyword) {
  PPContext *ctx = p->ctx;
  Token *t = ppi_current_token(p);
  if (t != NULL && t->ty == TK_IDENT && strcmp(t->input, keyword) == 0) {
    ctx->pos++;
    printf("# consumed if\n");
    return 0;
  }
  return 1;
}
int ppi_consume_ty(PreProcessor *p, int token_type) {
  PPContext *ctx = p->ctx;
  Token *t = ppi_current_token(p);
  if (t != NULL && t->ty == token_type) {
    p->ctx->pos++;
    return 0;
  }
  printf("# not consumed %d %c\n", token_type, token_type);
  return 1;
}

extern PPNode *pp_add(PreProcessor *p);
extern PPNode *pp_cmp(PreProcessor *p);

PPNode *pp_term(PreProcessor *p) {
  Token *t = ppi_current_token(p);
  if (t == NULL) {
    return NULL;
  }
  if (t->ty == TK_NUM) {
    p->ctx->pos++;
    return pp_new_node_num(t->val);
  }
  if(ppi_consume_ty(p, '(')) {
    PPNode *node = pp_add(p);
    if (!ppi_consume_ty(p, ')')) {
      fprintf(stderr, "%s:%d: expected ')', but got %c\n", __FILE__, __LINE__, ppi_current_token(p)->ty);
      exit(1);
    }
    return node;
  }
  PPNode *node = pp_add(p);
  return node;
}

PPNode *pp_cmp(PreProcessor *p) {
  PPNode *node = pp_term(p);
  return node;
}

PPNode *pp_add(PreProcessor *p) {
  PPNode *node = pp_cmp(p);
  if (ppi_consume_ty(p, '+')) {
    return pp_new_node('+', node, pp_add(p));
  }
  return node;
}

PPNode *pp_if_stmt(PreProcessor *p) {
  if(ppi_consume_ident(p, "if")) {
    PPNode *node = pp_new_node(PP_IF, NULL, NULL);
    node->body = pp_add(p);
    return node;
  }
  return NULL;
}

PPNode *pp_parse(PreProcessor *p, Vector *tokens) {
  Token *t;
  PPContext *ctx = p->ctx;
  ctx->tokens = tokens;
  ctx->pos = 1;
  for (int i = 0; i < tokens->len; i++) {
    t = tokens->data[i];
    printf("# %d: %d, %c\n", i, t->ty, t->ty);
  }

  PPNode *node = pp_if_stmt(p);
  return node;
}

PPNode *pp_node_to_bool(PreProcessor *p, PPNode *node) {
  switch(node->ty) {
    case PP_NUM:
      return pp_new_node_bool(node->val);
    default:
      fprintf(stderr, "%d: Error, not implemented for type %d\n", __LINE__, node->ty);
      exit(1);
  }
}

PPNode *pp_eval(PreProcessor *p, PPNode *node) {
  printf("# pp_eval\n");
  int start_pos = p->pos - 1;
  switch(node->ty) {
    case PP_IF: {
      printf("# eval if\n");
      PPNode *cond = pp_eval(p, node->body);
      PPNode *result = pp_node_to_bool(p, cond);
      Vector *tokens = collect_tokens_until_newline(p);
      remove_tokens(p, start_pos, tokens->len + 1);
      if (result->val) {
        // remove else cluase
        printf("# true case %d\n", cond->val);
        remove_endif(p);
        printf("# removed endif\n");
      } else {
        // remove body
        printf("# false case %d\n", cond->val);
        remove_until_endif(p);
        printf("# removed until endif\n");
      }
      break;
    }
    case PP_NUM:
      printf("# eval num, val: %d\n", node->val);
      return node;
    case '+': {
      printf("# eval +\n");
      PPNode *left = pp_eval(p, node->left);
      PPNode *right = pp_eval(p, node->right);
      return pp_new_node_num(left->val + right->val);
    }
    default:
      fprintf(stderr, "%d: not implemented yet, node->ty: %d\n", __LINE__, node->ty);
      exit(1);
  }
}

void process_define_macro(PreProcessor *p, Token *token) {
  printf("# process define macro\n");
  if (strcmp(token->input, "define") == 0) {
    int start_pos = p->pos - 1;
    Token *name = pp_consume_token(p);
    p->pos++;
    Vector *body = collect_tokens_until_newline(p);
    map_put(p->macros, name->input, body);
    //printf("# macro name %s, body length: %d\n", name->input, body->len);
    for (int i = 0; i < p->macros->keys->len; i++) {
      //printf("# %d: %s, %x\n", i, p->macros->keys->data[i], p->macros->vals->data[i]);
    }
    Vector *temp_body = map_get(p->macros, name->input);
    remove_tokens(p, start_pos, 3 + body->len);
    p->pos = start_pos;
    return;
  }

  if (strcmp(token->input, "ifdef") == 0 || strcmp(token->input, "ifndef") == 0) {
    int start_pos = p->pos - 1;
    int inverted = token->input[2] == 'n'; // ifNdef pr ifDef
    Token *name = pp_consume_token(p);
    p->pos++;
    Vector *body = map_get(p->macros, name->input);
    //printf("# found ifdef/ifndef %s at %x\n", name->input, body);
    if (body == NULL) {
      //printf("#@@@ %s is defined\n", name->input);
    } else {
      //printf("#@@@ %s is NOT defined\n", name->input);
    }

    remove_tokens(p, start_pos, 3); // #, ifdef, X => 3tokens
    if (inverted ? body == NULL : body != NULL) { // defined
      remove_endif(p);
    } else { // target is not defined
      remove_until_endif(p);
    }

    return;
  }


  if (strcmp(token->input, "if") == 0) {
    int start_pos = p->pos - 1;
    Vector *body = collect_tokens_until_newline(p);
    PPNode *node = pp_parse(p, body);
    if (node == NULL) {
      fprintf(stderr, "ERROR: invalid node generated at %d\n", p->pos);
      exit(1);
    }
    p->pos = start_pos;
    puts("# start eval if macro\n");
    pp_eval(p, node);
    return;
  }

  if (strcmp(token->input, "undef") == 0) {
    int start_pos = p->pos - 1;
    Token *name = pp_consume_token(p);
    p->pos++;
    map_delete(p->macros, name->input);
    remove_tokens(p, start_pos, 3); // #, undex, X => 3tokens
    return;
  }


  if (strcmp(token->input, "include") == 0) {
    int start_pos = p->pos - 1;
    Token *name = pp_consume_token(p);
    p->pos++;
    const char *filename = name->input;
    printf("# include %s from %s at %d\n", name->input, p->meta->file_name, p->pos);
    remove_tokens(p, start_pos, 3); // #, include, X => 3tokens
    include_counts++;
    char *path, *src;
    int i = 0;
    while(search_paths[i] != NULL) {
      int path_len = strlen(search_paths[i]);
      path = malloc(sizeof(char) * (path_len + strlen(filename) + 2));
      path[0] = '\0';
      strcat(path, search_paths[i]);
      path[path_len] = '/';
      path[path_len + 1] = '\0';
      strcat(path, filename);
      struct stat *st = malloc(sizeof(struct stat));
      errno = 0;
      if (stat(path, st) != 0) {
        if (errno != 2) {
          fprintf(stderr, "failed to get stat: %s, error: %s\n", path, strerror(errno));
          exit(1);
        }
      }

      if (st->st_size != 0 ) {
        printf("# tokenize: %s\n", path);
        Meta *header_meta = malloc(sizeof(Meta));
        header_meta->file_name = path;
        src = malloc(sizeof(char) * st->st_size);
        FILE *fp = fopen(path, "r");
        if (fp == NULL) {
          printf("# errorno: %d, includes: %d\n", errno,include_counts);
          fprintf(stderr, "# fp: %x, errno: %d, %s\n", fp, errno, strerror(errno));
          fclose(fp);
          free(path);
          free(st);
          free(fp);
          printf("# error\n");
          _exit(1);
        }
        fread(src, 1, st->st_size, fp);
        //printf("# size: %d, src: %s\n", st->st_size, src);
        fclose(fp);
        Vector *header_tokens = tokenize(src);
        // this contains only EOF.
        if (header_tokens->len == 1) return;
        Meta *meta = malloc(sizeof(Meta));
        meta->file_name = path;
        setup_meta(meta);
        printf("# preprocess: %s, tokens: %d\n#-----------", path, header_tokens->len);
        pre_process(meta, global_ctx, header_tokens);
        //export_tokens(header_tokens);
        printf("# output for %s\n#-------------", path);
        //export_tokens(header_tokens);
        printf("#------------- header token length: %d, src token length: %d\n", header_tokens->len, p->tokens->len);
        vec_delete(header_tokens, header_tokens->len -1); // remove TK_EOF
        Token *last_token = header_tokens->data[header_tokens->len - 1];
        int header_lines = last_token->line_start;
        Token *tk;
        for (int j = p->pos; j < p->tokens->len; j++) {
          tk = p->tokens->data[j];
          tk->line_start += header_lines;
          tk->line_end += header_lines;
        }
        vec_insert(p->tokens, header_tokens, p->pos);
        printf("#------------combined src token length: %d\n", p->tokens->len);
        p->pos = start_pos + header_tokens->len;;
        return;
      }
      free(path);
      i++;
    }
    fprintf(stderr, "cannot find %s in search paths\n", name->input);
    exit(1);
  }

  printf("# UNDEFINED directive %s at col: %d, line: %d, pos: %d\n", token->input, token->col_start, token->line_start, p->pos);
  return;
}

PreProcessor *new_preprocessor(Meta *meta, Vector *tokens) {
  PreProcessor *p = malloc(sizeof(PreProcessor));
  p->meta = meta;
  p->tokens = tokens;
  p->macros = new_map();
  p->pos = 0;
  p->ctx = malloc(sizeof(PPContext));
  p->ctx->pos = 0;
  return p;
}

PreProcessor *pre_process(Meta *meta, Context *ctx, Vector *tokens) {
  printf("# start preprocessing\n");
  PreProcessor *p = new_preprocessor(meta, tokens);
  Token *token, *last_token;
  printf("# pp1: replace predefined macro\n");
  for (int i = 0; i < tokens->len; i++) {
    token = tokens->data[i];
    if (token->ty == TK_IDENT) {
      replace_predefined_macro(meta, token);
    }
  }
  printf("# pp2: fetch define macro\n");
  for (p->pos = 0; p->pos < tokens->len; p->pos++) {
    // printf("# check pos %d / %d\n", p->pos,tokens->len);
    token = tokens->data[p->pos];
    if (p->pos > 0) {
      last_token = tokens->data[p->pos - 1];
      if (last_token->ty == '#' && token->ty == TK_IDENT) {
        process_define_macro(p, token);
      }
    }
  }
  p->pos = 0;
  token = pp_current_token(p);
  printf("# pp3: replace define macro\n");
  while(token != NULL) {
    if (token->ty == TK_IDENT) {
      replace_defined_macro(p, token);
    }
    token = pp_consume_token(p);
  }
  while (pp_current_token(p) != NULL) {
    token = pp_current_token(p);
    if (token->ty == '#') {
      token = pp_consume_token(p);
      //printf("# macro %s found\n", token->input);
    }
    p->pos++;
  }
  printf("# finish preprocessing\n");
  return p;
}
