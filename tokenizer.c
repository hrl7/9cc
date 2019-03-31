#include "9cc.h"

void tokenize(char *p) {
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    Token *token = malloc(sizeof(Token));
    if (*p == '\"') {
      int i = 0;
      char *s = p + 1;
      do {
        i++;
        p++;
      } while (*p != '\"' && *p != NULL);
      char *str = malloc(sizeof(char) * (i));
      memcpy(str, s, i - 1);
      str[i - 1] = NULL;
      token->input = str;
      token->ty = TK_STRING;
      printf("# found tk string %s\n", token->input);
      p++;
      vec_push(tokens, token);
      continue;
    }

    if (*p == '\'' && p[2] == '\'') {
      token->ty = TK_CHAR;
      token->input = NULL;
      token->val = p[1];
      vec_push(tokens, token);
      p += 3;
      continue;
    }

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

    if (*p == '>' && *(p+1) == '=') {
      token->ty = TK_LE;
      token->input = NULL;
      vec_push(tokens, token);
      p += 2;
      continue;
    }

    if (*p == '<' && *(p+1) == '=') {
      token->ty = TK_GE;
      token->input = NULL;
      vec_push(tokens, token);
      p += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '=' || *p == ';' ||
        *p == '{' || *p == '}' || *p == ',' ||
        *p == '<' || *p == '>' || *p == '&' ||
        *p == '[' || *p == ']') {
      token->ty = *p;
      token->input = p;
      vec_push(tokens, token);
      p++;
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      char *sp = p;
      while (('a' <= *p && *p <= 'z') ||
             ('0' <= *p && *p <= '9') ||
             *p == '_') {
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

    fprintf(stderr, "cannot tokenize: '%c', code: %d\n", p[0], p[0]);
    exit(1);
  }

  Token *token = malloc(sizeof(Token));
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}

