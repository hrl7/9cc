#include "9cc.h"

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
  Token *token;
  for (int i = 0; i < tokens->len; i++) {
    token = tokens->data[i];
    if (token->ty == TK_IDENT) {
      replace_predefined_macro(meta, token);
    }
  }
  printf("# finish preprocessing\n");
}
