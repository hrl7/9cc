#include "9cc.h"

extern Vector *tokenize(char *p);

typedef struct Tokenizer {
  Vector *tokens;
  int line;
  int col;
  char *src;
} Tokenizer;

Tokenizer *new_tokenizer(char *src) {
  Tokenizer *t = malloc(sizeof(Tokenizer));
  t->line = 1;
  t->col = 1;
  t->src = src;
  t->tokens = new_vector();
}


void set_start_pos(Tokenizer *t, Token *tok) {
  tok->line_start = t->line;
  tok->col_start = t->col;
}

void set_end_pos(Tokenizer *t, Token *tok) {
  tok->line_end = t->line;
  tok->col_end = t->col;
}

char *substr(char *p, int length) {
  char *str = malloc(sizeof(char) * (length + 1));
  strncpy(str, p, length);
  str[length] = NULL;
  return str;
}

Vector *tokenize(char *p) {
  Tokenizer *t = new_tokenizer(p);
  char *c_start = p;
  int oneline_comment = 0;
  int multiline_comment = 0;
  while (*p) {

    if (isspace(*p) || *p == 16 || *p == 17 || multiline_comment || oneline_comment) {
      if (multiline_comment && (*p == '*' && *(p + 1) == '/')) {
        multiline_comment = 0;
        p++;
        t->col +=2;
      } else if (*p == '\n') {
        t->line++;
        t->col = 0;
        oneline_comment = 0;
      } else {
        t->col++;
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
    set_start_pos(t, token);
    if (*p == '\"') {
      int i = 0;
      char *s = p + 1;
      do {
        i++;
        p++;
        t->col++;
      } while (*p != '\"' && *p != 0);
      token->input = substr(s, i - 1);
      token->ty = TK_STRING;
      p++;
      t->col++;
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      continue;
    }

    if (t->tokens->len > 1) {
      Token *last_token = t->tokens->data[t->tokens->len - 1];
      if (last_token != NULL && last_token->ty == TK_IDENT && strcmp(last_token->input, "include") == 0 && *p == '<') {
        int i = 0;
        char *s = p + 1;
        do {
          i++;
          p++;
          t->col++;
        } while (*p != '>' && *p != 0);
        token->input = substr(s, i - 1);
        token->ty = TK_HEADER_FILE_NAME;
        p++;
        t->col++;
        set_end_pos(t, token);
        vec_push(t->tokens, token);
        continue;
      }
    }

    if (*p == '\'' && p[2] == '\'') {
      token->ty = TK_CHAR;
      token->input = NULL;
      token->val = p[1];
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p += 3;
      t->col += 3;
      continue;
    }

    if (*p == '=' && *(p+1) == '=') {
      token->ty = TK_EQ;
      token->input = NULL;
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p += 2;
      t->col += 2;
      continue;
    }

    if (*p == '!' && *(p+1) == '=') {
      token->ty = TK_NEQ;
      token->input = NULL;
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p += 2;
      continue;
    }

    if (*p == '>' && *(p+1) == '=') {
      token->ty = TK_LE;
      token->input = NULL;
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p += 2;
      continue;
    }

    if (*p == '<' && *(p+1) == '=') {
      token->ty = TK_GE;
      token->input = NULL;
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p += 2;
      t->col += 2;
      continue;
    }

    if (*p == '&' && *(p+1) == '&') {
      token->ty = TK_AND;
      token->input = NULL;
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p += 2;
      t->col += 2;
      continue;
    }

    if (*p == '|' && *(p+1) == '|') {
      token->ty = TK_OR;
      token->input = NULL;
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p += 2;
      t->col += 2;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
        *p == '(' || *p == ')' || *p == '=' || *p == ';' ||
        *p == '{' || *p == '}' || *p == ',' || *p == '.' ||
        *p == '<' || *p == '>' || *p == '&' ||
        *p == '!' || *p == '?' || *p == ':' ||
        *p == '[' || *p == ']' || *p == '#' || *p == '\\') {
      token->ty = *p;
      token->input = substr(p, 1);
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      p++;
      t->col++;
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
        t->col++;
      }
      int width = p - sp;
      token->ty = TK_IDENT;
      token->input = substr(sp, width);
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      continue;
    }

    if (isdigit(*p)) {
      token->ty = TK_NUM;
      char *sp = p;
      token->val = strtol(p, &p, 10);
      token->input = substr(sp, p - sp);
      set_end_pos(t, token);
      vec_push(t->tokens, token);
      continue;
    }

    fprintf(stderr, "cannot tokenize: '%c', code: %d, at line: %d, col: %d\n", p[0], p[0], t->line, t->col);
    exit(1);
  }

  Token *token = malloc(sizeof(Token));
  token->ty = TK_EOF;
  token->input = p;
  vec_push(t->tokens, token);
  return t->tokens;
}

void export_tokens(Vector *tokens) {
  printf("# export tokens. length: %d\n#----------\n", tokens->len);
  int i = 0;
  int current_line = 0;
  int current_col = 0;
  Token *token = tokens->data[i];
  while(token != NULL && token->ty != TK_EOF) {
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

