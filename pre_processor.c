#include "9cc.h"

typedef struct PreProcessor {
  int pos;
  struct Vector *tokens;
  struct Meta *meta;
} PreProcessor;

Token *pp_current_token(PreProcessor *p) {
  return p->tokens->data[p->pos];
}

Token *pp_consume_token(PreProcessor *p) {
  p->pos++;
  return p->tokens->data[p->pos];
}

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

void pre_process(Meta *meta, Context *ctx, Vector *tokens) {
  printf("# start preprocessing\n");
  PreProcessor *p = malloc(sizeof(PreProcessor));
  p->meta = meta;
  p->tokens = tokens;
  p->pos = 0;
  Token *token;
  for (int i = 0; i < tokens->len; i++) {
    token = tokens->data[i];
    if (token->ty == TK_IDENT) {
      replace_predefined_macro(meta, token);
    }
  }
  while (pp_current_token(p) != NULL) {
    token = pp_current_token(p);
    if (token->ty == '#') {
      token = pp_consume_token(p);
      printf("# macro %s found\n", token->input);
    }
    p->pos++;
  }
  printf("# finish preprocessing\n");
}
