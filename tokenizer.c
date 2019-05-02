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

char *substr(char *p, int length) {
  char *str = malloc(sizeof(char) * (length + 1));
  strncpy(str, p, length);
  str[length] = NULL;
  return str;
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
      token->input = substr(s, i - 1);
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
        *p == '{' || *p == '}' || *p == ',' || *p == '.' ||
        *p == '<' || *p == '>' || *p == '&' ||
        *p == '[' || *p == ']' || *p == '#') {
      token->ty = *p;
      token->input = substr(p, 1);
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
      int width = p - sp;
      token->ty = TK_IDENT;
      token->input = substr(sp, width);
      set_end_pos(token);
      vec_push(tokens, token);
      continue;
    }

    if (isdigit(*p)) {
      token->ty = TK_NUM;
      char *sp = p;
      token->val = strtol(p, &p, 10);
      token->input = substr(sp, p - sp);
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

void export_tokens() {
  printf("# export tokens\n----------\n");
  int i = 0;
  int current_line = 0;
  int current_col = 0;
  Token *token = tokens->data[i];
  while(token != NULL) {
    //printf("@@@ curreet_line: %d, token->line_start: %d\n", current_line, token->line_start);
    while (current_line < token->line_start) {
      printf("\n");
      current_line++;
      current_col = 0;
    };
    //printf("@@@ current_col: %d, token->col_start: %d | ", current_col, token->col_start);
    while (current_col < token->col_start ) {
      printf(" ");
      current_col++;
    }

    switch (token->ty) {
      case TK_STRING:
        printf("\"%s\"", token->input);
        break;
      case TK_CHAR:
        printf("\'%c\'", token->val);
        break;
      case TK_EQ:
        puts("==");
        break;
      default:
        printf("%s", token->input);
    }
    current_col = token->col_end;
    token = tokens->data[++i];
  }
}

