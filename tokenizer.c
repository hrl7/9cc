#include "9cc.h"

void tokenize(char *p) {
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    Token *token = malloc(sizeof(Token));

    if (*p == '=' && *(p+1) == '=') {
      token->ty = TK_EQ;
      token->input = NULL;
      vec_push(tokens, token);
      p += 2;
      continue;
    }

    if (*p == '!' && *(p+1) == '=') {
      token->ty = TK_NEQ;
      token->input = NULL;
      vec_push(tokens, token);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '=' || *p == ';') {
      token->ty = *p;
      token->input = p;
      vec_push(tokens, token);
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      char *sp = p;
      while ('a' <= *p && *p <= 'z') {
        p++;
      }
      int width = p - sp + 1; // last 1 for \0
      char *name = malloc(sizeof(char) * width);
      strncpy(name, sp, width);
      name[width - 1] = '\0';
      token->ty = TK_IDENT;
      token->input = name;
      vec_push(tokens, token);
      continue;
    }

    if (isdigit(*p)) {
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      vec_push(tokens, token);
      continue;
    }

    fprintf(stderr, "cannot tokenize: %s\n", p);
    exit(1);
  }

  Token *token = malloc(sizeof(Token));
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}
