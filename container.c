#include "9cc.h"

Map *new_map() {
  Map *map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void free_map(Map *map) {
  free_vector(map->keys);
  free_vector(map->vals);
  free(map);
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

int map_get_index(Map *map, char *key) {
  for (int i = map->keys->len - 1; i >= 0; i--) {
    if (strcmp(map->keys->data[i], key) == 0) {
      return i;
    }
  }
  return -1;
}

void *map_get(Map *map, char *key) {
  int index = map_get_index(map, key);
  if (index != -1) {
    return map->vals->data[index];
  }
  return NULL;
}

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void free_vector(Vector *vec) {
  free(vec->data);
  free(vec);
}

void vec_push(Vector *vec, void *elm) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }

  vec->data[vec->len++] = elm;
}

Record *new_record(char *name, int offset, Type *type, int is_arg) {
  Record *rec = malloc(sizeof(Record));
  rec->name = malloc(sizeof(char) * (strlen(name) + 1));
  rec->type = malloc(sizeof(Type));
  rec->offset = offset;
  rec->type = type;
  strcpy(rec->name, name);
  rec->is_arg = is_arg;
  return rec;
}

Record *get_record(Context *ctx, char *name) {
  Context *cur = ctx;
  Record *rec;
  do {
    rec = map_get(cur->vars, name);
    cur = cur->parent;
  } while(rec == NULL && cur != NULL && cur->vars != NULL);
  if (rec == NULL) {
    fprintf(stderr, "got unexpected record: %s\n", name);
    exit(1);
  }
  return rec;
}


int expect_str(int line, char *expected, char *actual) {
  if (strcmp(expected, actual) == 0) {
    return 0;
  }
  fprintf(stderr, "%d: %s expected, but got %s\n", line, expected, actual);
  exit(1);
}

int expect(int line, int expected, int actual) {
  if (expected == actual) {
    return 0;
  }
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
}

Context *new_context(const char *name) {
  Context *ctx = malloc(sizeof(Context));
  ctx->vars = new_map();
  ctx->parent = malloc(sizeof(Context));
  ctx->name = malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(ctx->name, name);
  return ctx;
}

void test_map() {
  printf("test_map\n");
  Map *map = new_map();

  expect(__LINE__, NULL, (int)map_get(map, "foo"));

  map_put(map, "foo", (void *)2);
  expect(__LINE__, 2, (int)map_get(map, "foo"));

  map_put(map, "bar", (void *)4);
  expect(__LINE__, 4, (int)map_get(map, "bar"));

  map_put(map, "foo", (void *)6);
  expect(__LINE__, 6, (int)map_get(map, "foo"));
  free_map(map);
  printf("OK\n");
}

void test_vector() {
  printf("test_vector\n");
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++) {
    vec_push(vec, (void *)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);
  printf("OK \n");
}

void test_context() {
  printf("test_context\n");
  Context *ctx = new_context("test");
  char *name = malloc(sizeof(char) * 5);
  strcpy(name, "test");
  Type *t = new_int_type();
  expect(__LINE__, 0, t->ty);
  Record *rec = new_record(name, 4, t, 0);
  map_put(ctx->vars, name, rec);
  Record *trec;
  trec = get_record(ctx, name);
  expect_str(__LINE__, "test", ctx->name);
  expect_str(__LINE__, "test", trec->name);
  expect(__LINE__, 4, trec->offset);
  expect(__LINE__, 0, trec->type->ty);
  expect(__LINE__, 0, trec->type->ptr_of);
  printf("OK\n");
}

void runtest() {
  test_vector();
  test_map();
  test_context();
}
