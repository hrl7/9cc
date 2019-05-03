#include "9cc.h"

typedef struct PreProcessor {
  int pos;
  struct Vector *tokens;
  struct Map *macros;
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
  printf("# collect tokens %d\n", tks->len);
  return tks;
}

void remove_tokens(PreProcessor *p, int index, int num) {
  for (int j = 0; j < num; j++) {
    vec_delete(p->tokens, index);
  }
}

void remove_endif(PreProcessor *p) {
  int i = p->pos;
  Token *token = p->tokens->data[i];
  while(token != NULL) {
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
    printf("#@@@ check token %s\n", token->input);
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

void process_define_macro(PreProcessor *p, Token *token) {
  if (strcmp(token->input, "define") == 0) {
    int start_pos = p->pos - 1;
    Token *name = pp_consume_token(p);
    p->pos++;
    Vector *body = collect_tokens_until_newline(p);
    map_put(p->macros, name->input, body);
    printf("# macro name %s, body length: %d\n", name->input, body->len);
    for (int i = 0; i < p->macros->keys->len; i++) {
      printf("# %d: %s, %x\n", i, p->macros->keys->data[i], p->macros->vals->data[i]);
    }
    Vector *temp_body = map_get(p->macros, name->input);
    remove_tokens(p, start_pos, 3 + body->len);
    p->pos = start_pos;
    return;
  }

  if (strcmp(token->input, "ifdef") == 0) {
    int start_pos = p->pos - 1;
    Token *name = pp_consume_token(p);
    p->pos++;
    Vector *body = map_get(p->macros, name->input);
    printf("# found ifdef %s at %x\n", name->input, body);
    remove_tokens(p, start_pos, 3); // #, ifdef, X => 3tokens
    if (body != NULL) { // defined
      printf("#@@@ %s is defined\n", name->input);
      remove_endif(p);
    } else { // target is not defined
      printf("#@@@ %s is NOT defined\n", name->input);
      remove_until_endif(p);
    }

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
  return p;
}

void pre_process(Meta *meta, Context *ctx, Vector *tokens) {
  printf("# start preprocessing\n");
  PreProcessor *p = new_preprocessor(meta);
  Token *token;
  printf("# pp1: replace predefined macro\n");
  for (int i = 0; i < tokens->len; i++) {
    token = tokens->data[i];
    if (token->ty == TK_IDENT) {
      replace_predefined_macro(meta, token);
    }
  }
  printf("# pp2: fetch define macro\n");
  for (p->pos = 0; p->pos < tokens->len; p->pos++) {
    token = tokens->data[p->pos];
    if (token->ty == TK_IDENT) {
      process_define_macro(p, token);
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
      printf("# macro %s found\n", token->input);
    }
    p->pos++;
  }
  printf("# finish preprocessing\n");
}
