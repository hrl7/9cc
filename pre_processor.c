#include "9cc.h"

typedef struct PreProcessor {
  int pos;
  struct Vector *tokens;
  struct Map *macros;
  struct Meta *meta;
  char *replaced_key;
} PreProcessor;

Token *pp_current_token(PreProcessor *p) {
  return p->tokens->data[p->pos];
}

Token *pp_next_token(PreProcessor *p) {
  if (p->pos == p->tokens->len - 1) return NULL;
  return p->tokens->data[p->pos+1];
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

void replace_macro(PreProcessor *p, Token *token) {
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
  p->pos--;
  return tks;
}

void process_macro_definition(PreProcessor *p, Token *token) {
  if (strcmp(token->input, "define") == 0) {
    int start_pos = p->pos - 1;
    Token *name = pp_consume_token(p);
    p->pos++;
    Vector *body = collect_tokens_until_newline(p);
    map_put(p->macros, name->input, body);
    Vector *temp_body = map_get(p->macros, name->input);
    for (int i = 0; i <= 2 + body->len; i++) {
      vec_delete(p->tokens, start_pos);
    }
    p->pos = start_pos;
    return;
  }

  if (strcmp(token->input, "ifdef") == 0) {
    Token *name = pp_consume_token(p);
    p->pos++;
    printf("# found ifdef %s\n", name->input);

    return;
  }
  return;
}

PreProcessor *new_preprocessor(Meta *meta) {
  PreProcessor *p = malloc(sizeof(PreProcessor));
  p->meta = meta;
  p->tokens = tokens;
  p->macros = new_map();
  p->pos = 0;
  p->replaced_key = NULL;
  return p;
}

void pre_process(Meta *meta, Context *ctx, Vector *tokens) {
  printf("# start preprocessing\n");
  PreProcessor *p = new_preprocessor(meta);
  p->pos = 0;
  Token *token = pp_current_token(p);
  while(token != NULL) {
    printf("# check pos: %d, %s\n", p->pos, token->input);
    if (token->ty == TK_IDENT) {
      process_macro_definition(p, token);
      replace_macro(p, token);
    }
    token = pp_consume_token(p);
  }

  puts("\n# macro tables -------------------------");
  Vector *body;
  char *body_str;
  int length;
  Token *t;
  for (int i = 0; i < p->macros->keys->len; i++) {
    body= p->macros->vals->data[i];
    length = 0;
    for (int j = 0; j < body->len; j++) {
      t = body->data[j];
      length += strlen(t->input);
    }
    body_str = malloc(sizeof(char) * (length + 1));
    body_str[0] = '\0';
    for (int j = 0; j < body->len; j++) {
      t = body->data[j];
      strcat(body_str, t->input);
    }
    printf("# %d %s => %s (%d tokens) \n", i, p->macros->keys->data[i], body_str, body->len);
  }
  printf("# finish preprocessing\n");
}
