
#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum {
  JUEN_OBJECT = 1,
  JUEN_ARRAY = 2,
  JUEN_STRING = 3,
  JUEN_PRIMITIVE = 4
} juentype_t;

typedef struct {
  juentype_t type;
  int start;
  int end;
  int size;
} juentok_t;


typedef struct {
  unsigned int pos;
  unsigned int toknext;
  int toksuper;
} juen_parser;

void juen_init(juen_parser *parser);
static int dump(const char *js, juentok_t *t, size_t count, int indent);

int juen_parse(juen_parser *parser, const char *js, const size_t len, juentok_t *tokens, const unsigned int num_tokens);

static juentok_t *juen_alloc_token(juen_parser *parser, juentok_t *tokens, const size_t num_tokens) {
  juentok_t *tok;
  if (parser->toknext >= num_tokens)
    return NULL;
  tok = &tokens[parser->toknext++];
  tok->start = tok->end = -1;
  tok->size = 0;
  return tok;
}

static void juen_fill_token(juentok_t *token, const juentype_t type, const int start, const int end) {
  token->type = type;
  token->start = start;
  token->end = end;
  token->size = 0;
}

static int juen_parse_primitive(juen_parser *parser, const char *js, const size_t len, juentok_t *tokens, const size_t num_tokens) {
  juentok_t *token;
  int start;
  start = parser->pos;
  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
    switch (js[parser->pos]) {
      case '\t':
      case '\r':
      case '\n':
      case ' ':
      case ',':
      case ']':
      case '}':
        goto found;
    }
    if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
      parser->pos = start;
      return -1;
    }
  }
  found:
    if (tokens == NULL) {
      parser->pos--;
      return 0;
  }
  token = juen_alloc_token(parser, tokens, num_tokens);
  if (token == NULL) {
    parser->pos = start;
    return -1;
  }
  juen_fill_token(token, JUEN_PRIMITIVE, start, parser->pos);

  parser->pos--;
  return 0;
}

static int juen_parse_string(juen_parser *parser, const char *js, const size_t len, juentok_t *tokens, const size_t num_tokens) {
  juentok_t *token;

  int start = parser->pos;

  parser->pos++;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
    char c = js[parser->pos];

    if (c == '\"') {
      if (tokens == NULL) {
        return 0;
      }
      token = juen_alloc_token(parser, tokens, num_tokens);
      if (token == NULL) {
        parser->pos = start;
        return -1;
      }
      juen_fill_token(token, JUEN_STRING, start + 1, parser->pos);

      return 0;
    }

    if (c == '\\' && parser->pos + 1 < len) {
      int i;
      parser->pos++;
      switch (js[parser->pos]) {
      case '\"':
      case '/':
      case '\\':
      case 'b':
      case 'f':
      case 'r':
      case 'n':
      case 't':
        break;
      case 'u':
        parser->pos++;
        for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0';
             i++) {
          if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) ||   /* 0-9 */
                (js[parser->pos] >= 65 && js[parser->pos] <= 70) ||   /* A-F */
                (js[parser->pos] >= 97 && js[parser->pos] <= 102))) { /* a-f */
            parser->pos = start;
            return -1;
          }
          parser->pos++;
        }
        parser->pos--;
        break;

      default:
        parser->pos = start;
        return -1;
      }
    }
  }
  parser->pos = start;
  return -1;
}
static inline void *realloc_it(void *ptrmem, size_t size) {
  void *p = realloc(ptrmem, size);
  return p;
}
static int dump(const char *js, juentok_t *t, size_t count, int indent) {
  int i, j, k;
  juentok_t *key;
  if (count == 0) {
    return 0;
  }
  if (t->type == JUEN_PRIMITIVE) {
    printf("%.*s", t->end - t->start, js + t->start);
    return 1;
  }
  else if (t->type == JUEN_STRING) {
    printf("'%.*s'", t->end - t->start, js + t->start);
    return 1;
  }
  else if (t->type == JUEN_OBJECT) {
    printf("\n");
    j = 0;
    for (i = 0; i < t->size; i++) {
      for (k = 0; k < indent; k++)
        printf("  ");

      key = t + 1 + j;
      j += dump(js, key, count - j, indent + 1);
      if (key->size > 0) {
        printf(": ");
        j += dump(js, t + 1 + j, count - j, indent + 1);
      }
      printf("\n");
    }
    return j + 1;
  }
  else if (t->type == JUEN_ARRAY) {
    j = 0;
    printf("\n");
    for (i = 0; i < t->size; i++) {
      for (k = 0; k < indent - 1; k++)
        printf("  ");
      printf("   - ");
      j += dump(js, t + 1 + j, count - j, indent + 1);
      printf("\n");
    }
    return j + 1;
  }
  return 0;
}

int main() {
  int r;
  int eof_expected = 0;
  char *js = NULL;
  size_t jslen = 0;
  char buf[BUFSIZ];

  juen_parser p;
  juentok_t *tok;
  size_t tokcount = 2;

  juen_init(&p);

  tok = malloc(sizeof(*tok) * tokcount);

  while(1) {
    r = fread(buf, 1, sizeof(buf), stdin);
    if (r == 0)
      if (eof_expected != 0)
        return 0;

    js = realloc_it(js, jslen + r + 1);

    strncpy(js + jslen, buf, r);
    jslen = jslen + r;

  again:
    r = juen_parse(&p, js, jslen, tok, tokcount);
    if (r < 0) {
      if (r == -1) {
        tokcount = tokcount * 2;
        tok = realloc_it(tok, sizeof(*tok) * tokcount);
        if (tok == NULL) {
          return 3;
        }
        goto again;
      }
    }
    else {
      dump(js, tok, p.toknext, 0);
      eof_expected = 1;
    }
  }

  return EXIT_SUCCESS;
}

int juen_parse(juen_parser *parser, const char *js, const size_t len, juentok_t *tokens, const unsigned int num_tokens) {
  int r;
  int i;
  juentok_t *token;
  int count = parser->toknext;

  for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
    char c;
    juentype_t type;

    c = js[parser->pos];
    switch (c) {
    case '{':
    case '[':
      count++;
      if (tokens == NULL) {
        break;
      }
      token = juen_alloc_token(parser, tokens, num_tokens);
      if (token == NULL) {
        return -1;
      }
      if (parser->toksuper != -1) {
        juentok_t *t = &tokens[parser->toksuper];

        t->size++;
      }
      token->type = (c == '{' ? JUEN_OBJECT : JUEN_ARRAY);
      token->start = parser->pos;
      parser->toksuper = parser->toknext - 1;
      break;
    case '}':
    case ']':
      if (tokens == NULL) {
        break;
      }
      type = (c == '}' ? JUEN_OBJECT : JUEN_ARRAY);


      for (i = parser->toknext - 1; i >= 0; i--) {
        token = &tokens[i];
        if (token->start != -1 && token->end == -1) {
          if (token->type != type) {
            return -1;
          }
          parser->toksuper = -1;
          token->end = parser->pos + 1;
          break;
        }
      }
      if (i == -1) {
        return -1;
      }
      for (; i >= 0; i--) {
        token = &tokens[i];
        if (token->start != -1 && token->end == -1) {
          parser->toksuper = i;
          break;
        }
      }
      break;
    case '\"':
      r = juen_parse_string(parser, js, len, tokens, num_tokens);
      if (r < 0)
        return r;
      count++;
      if (parser->toksuper != -1 && tokens != NULL)
        tokens[parser->toksuper].size++;

      break;
    case '\t':
    case '\r':
    case '\n':
    case ' ':
      break;
    case ':':
      parser->toksuper = parser->toknext - 1;
      break;
    case ',':
      if (tokens != NULL && parser->toksuper != -1 && tokens[parser->toksuper].type != JUEN_ARRAY && tokens[parser->toksuper].type != JUEN_OBJECT) {
        for (i = parser->toknext - 1; i >= 0; i--)
          if (tokens[i].type == JUEN_ARRAY || tokens[i].type == JUEN_OBJECT)
            if (tokens[i].start != -1 && tokens[i].end == -1) {
              parser->toksuper = i;
              break;
            }
      }
      break;
    default:
      r = juen_parse_primitive(parser, js, len, tokens, num_tokens);
      if (r < 0)
        return r;

      count++;

      if (parser->toksuper != -1 && tokens != NULL)
        tokens[parser->toksuper].size++;
      break;
    }
  }

  if (tokens != NULL)
    for (i = parser->toknext - 1; i >= 0; i--)
      if (tokens[i].start != -1 && tokens[i].end == -1)
        return -1;




  return count;
}

void juen_init(juen_parser *parser) {
  parser->pos = 0;
  parser->toknext = 0;
  parser->toksuper = -1;
}
