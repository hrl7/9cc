#include "9cc.h"

extern void tokenize(char *p);

int line = 1;
int col = 1;

void set_start_pos(Token *tok) {
  tok->line_start = line;
  tok->col_start = col;
}

void set_end_pos(Token *tok) {
  tok->line_end = line;
  tok->col_end = col;
}

void tokenize(char *p) {
  char *c_start = p;
  int oneline_comment = 0;
  int multiline_comment = 0;
  while (*p) {

    if (isspace(*p) || multiline_comment || oneline_comment) {
      if (multiline_comment && (*p == '*' && *(p + 1) == '/')) {
        multiline_comment = 0;
        p++;
        col +=2;
      } else if (*p == '\n') {
        line++;
        col = 0;
        oneline_comment = 0;
      } else {
        col++;
      }
      p++;
      c_start = p;
      continue;
    }

    if (*p == '/') {
      if (*(p+1) == '/') {
        oneline_comment = 1;
        p += 2;
        continue;
      }
      if (*(p+1) == '*') {
        multiline_comment = 1;
        p += 2;
        continue;
      }
    }

    Token *token = malloc(sizeof(Token));
    set_start_pos(token);
    if (*p == '\"') {
      int i = 0;
      char *s = p + 1;
      do {
        i++;
        p++;
        col++;
      } while (*p != '\"' && *p != 0);
      char *str = malloc(sizeof(char) * (i));
      memcpy(str, s, i - 1);
      str[i - 1] = 0;
      token->input = str;
      token->ty = TK_STRING;
      p++;
      col++;
      set_end_pos(token);
      vec_push(tokens, token);
      continue;
    }

    if (*p == '\'' && p[2] == '\'') {
      token->ty = TK_CHAR;
      token->input = NULL;
      token->val = p[1];
      set_end_pos(token);
      vec_push(tokens, token);
      p += 3;
      col += 3;
      continue;
    }

    if (*p == '=' && *(p+1) == '=') {
      token->ty = TK_EQ;
      token->input = NULL;
      set_end_pos(token);
      vec_push(tokens, token);
      p += 2;
      col += 2;
      continue;
    }

    if (*p == '!' && *(p+1) == '=') {
      token->ty = TK_NEQ;
      token->input = NULL;
      set_end_pos(token);
      vec_push(tokens, token);
      p += 2;
      continue;
    }

    if (*p == '>' && *(p+1) == '=') {
      token->ty = TK_LE;
      token->input = NULL;
      set_end_pos(token);
      vec_push(tokens, token);
      p += 2;
      continue;
    }

    if (*p == '<' && *(p+1) == '=') {
      token->ty = TK_GE;
      token->input = NULL;
      set_end_pos(token);
      vec_push(tokens, token);
      p += 2;
      col += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '=' || *p == ';' ||
        *p == '{' || *p == '}' || *p == ',' ||
        *p == '<' || *p == '>' || *p == '&' ||
        *p == '[' || *p == ']') {
      token->ty = *p;
      token->input = p;
      set_end_pos(token);
      vec_push(tokens, token);
      p++;
      col++;
      continue;
    }

    if (('a' <= *p && *p <= 'z') ||
        ('A' <= *p && *p <= 'Z') ||
        *p == '_') {
      char *sp = p;
      while (('a' <= *p && *p <= 'z') ||
             ('A' <= *p && *p <= 'Z') ||
             ('0' <= *p && *p <= '9') ||
             *p == '_') {
        p++;
        col++;
      }
      int width = p - sp + 1; // last 1 for \0
      char *name = malloc(sizeof(char) * width);
      strncpy(name, sp, width);
      name[width - 1] = '\0';
      token->ty = TK_IDENT;
      token->input = name;
      set_end_pos(token);
      vec_push(tokens, token);
      continue;
    }

    if (isdigit(*p)) {
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      set_end_pos(token);
      vec_push(tokens, token);
      continue;
    }

    fprintf(stderr, "cannot tokenize: '%c', code: %d, at line: %d, col: %d\n", p[0], p[0], line, col);
    exit(1);
  }

  Token *token = malloc(sizeof(Token));
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}

