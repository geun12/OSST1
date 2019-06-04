#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define BUFSIZE 20000

typedef enum{
  DEFAULT=0,
  KOREAN=1,
  WESTERN=2,
  CHINESE=3,
  JAPANESE=4,
  BARBEQUE=5,
  MIDNIGHT=6
}foodtype_t;

typedef enum {
  OBJECT = 1,
  ARRAY = 2,
  STRING = 3,
  PRIMITIVE = 4
} token_type_t;

typedef struct {
  token_type_t type;
  foodtype_t food_type;
  int start;
  int end;
  int size;
  int score;
} token_t;

typedef struct {
  int offset;
  int next;//next token to allocate
  int pre;//parent object or array
} parser;


void init_parser(parser *parser);//parser structure 초기화
int parsing(parser *parser, char *js, int len, token_t *token_arr, int arr_size);//main parser
int parsing_primitive(parser *parser, char *js, int len, token_t *token_arr, int arr_size);//sub parser. primitive가 나올 경우, 파싱한다
int parsing_string(parser *parser, char *js, int len, token_t *token_arr, int arr_size);//sub parser2. string이 나올 경우, 파싱한다


token_t *token_allocator(parser *parser, token_t *token_arr, int arr_size);//token을 하나 생성하고 초기화함
void token_fill(token_t *token, token_type_t type, int start, int end);//token 값을 지정
void *realloc_it(void *ptrmem, int size);
int stringToInt (const char *js, token_t *t);
int token_printer(char *js, token_t *t, int count, int indent,int *num);//토큰 array에 저장된 토큰을 꺼내서 토큰에 저장된 범위에 해당하는 string을 print

void typer(char *js,token_t *t,char *master_buf);
void typeFinder(char *js,token_t *t,char *master_buf);
_Bool score_getter(char *js,token_t *t,char *master_buf);
_Bool name_getter(char *js,token_t *t,char *master_buf);
void namer(char *js,token_t *t,char *master_buf);
void scorer(char *js,token_t *t,char *master_buf);


int main(int argc, char** argv) {
  int result;
  int eof_expected = 0;
  char *js = NULL;
  size_t jslen = 0;
  char buf[BUFSIZE];

  parser p;
  token_t *tok;
  size_t tokcount = 2;

  init_parser(&p);

  tok = malloc(sizeof(*tok) * tokcount);

  FILE* fi = fopen(argv[1], "r");

  while(1) {
    result = fread(buf, 1, sizeof(buf), fi);
    if (result == 0)
      if (eof_expected != 0)
        return 0;

    js = realloc_it(js, jslen + result + 1);

    strncpy(js + jslen, buf, result);
    jslen = jslen + result;

  again:
    result = parsing(&p, js, jslen, tok, tokcount);
    if (result < 0) {
      if (result == -1) {
        tokcount = tokcount * 2;
        tok = realloc_it(tok, sizeof(*tok) * tokcount);
        if (tok == NULL) {
          return 3;
        }
        goto again;
      }
    }
    else {
      int num = 0;
      token_printer(js, tok, p.next, 0, &num);
      eof_expected = 1;
    }
  }
  fclose(fi);
  return EXIT_SUCCESS;
}
//parser의 값을 초기화
void init_parser(parser *parser) {
  parser->offset = 0;
  parser->next = 0;
  parser->pre = -1;
}
//main parsing 함수
int parsing(parser *parser, char *js, int len, token_t *token_arr, int arr_size) {

  int result;//파싱 결과를 알려주는 변수. result의 값이 0보다 작은 경우, 파싱에 실패한 것이다.
  int i;//iterator
  token_t *token;
  int count = parser->next;//토큰의 전체 갯수를 count한다.
  char c;//파서의 offset가 현재 가리키고 있는 character
  token_type_t type;

  for (; parser->offset < len && js[parser->offset] != '\0'; parser->offset++) {
    c = js[parser->offset];
    switch (c) {
    case '{':
    case '[':
      count++;
      token = token_allocator(parser, token_arr, arr_size);
      //{ 나 [ 가 나올 경우, 토큰을 하나 생성한다
      if (token == NULL) {
        return -1;
      }
      if (parser->pre != -1) {
        token_t *t = &token_arr[parser->pre];
        t->size++;
      }
      //offset이 가리키고 있는 값이 {일 경우, 해당 토큰의 type을 OBJECT로 정의한다. {가 아닌 경우는 [ 이므로 토큰의 type을 ARRAY로 정의한다.
      token->type = (c == '{' ? OBJECT : ARRAY);
      //token의 시작점을 현재 파서가 가리키고 있는 지점으로 설정한다
      token->start = parser->offset;
      parser->pre = parser->next - 1;
      break;
    case '}':
    case ']':
      if (token_arr == NULL) {
        break;
      }
      type = (c == '}' ? OBJECT : ARRAY);
      //범위. Object나 Array의 경우, 여러개의 토큰을 포함하고 있기 때문에 해당 토큰들을 object나 array에 포함시키는 과정이다.
      for (i = parser->next - 1; i >= 0; i--) {
        token = &token_arr[i];
        if (token->start != -1 && token->end == -1) {
          if (token->type != type) {
            return -1;
          }
          parser->pre = -1;
          token->end = parser->offset + 1;
          break;
        }
      }
      if (i == -1) {
        return -1;
      }
      for (; i >= 0; i--) {
        token = &token_arr[i];
        if (token->start != -1 && token->end == -1) {
          parser->pre = i;
          break;
        }
      }
      break;
      //offset이 가리키고 있는 값이 "일 경우 string parser를 부른다.
    case '\"':
      result = parsing_string(parser, js, len, token_arr, arr_size);
      if (result < 0)
        return result;
      count++;
      if (parser->pre != -1 && token_arr != NULL)
        token_arr[parser->pre].size++;
      break;
    case '\t':
    case '\r':
    case '\n':
    case ' ':
      break;
    case ':':
      parser->pre = parser->next - 1;
      break;
      // , 표시가 나왔을 경우,
      case ',':
        if (token_arr != NULL && parser->pre != -1 && token_arr[parser->pre].type != ARRAY && token_arr[parser->pre].type != OBJECT) {
          for (i = parser->next - 1; i >= 0; i--)
            if (token_arr[i].type == ARRAY || token_arr[i].type == OBJECT)
              if (token_arr[i].start != -1 && token_arr[i].end == -1) {
                parser->pre = i;
                break;
              }
        }
        break;
        //만약 offset가 위의 type 중 아무것에도 해당하지 않는다면, primitive parser를 부른다.
    default:
      result = parsing_primitive(parser, js, len, token_arr, arr_size);
      //result가 0보다 작을 경우, error 상황이다. result를 그대로 return하여 parser를 부른 변수에게 error임을 알려준다.
      if (result < 0)
        return result;
      count++;
      if (parser->pre != -1 && token_arr != NULL)
        token_arr[parser->pre].size++;
      break;
    }
  }
  if (token_arr != NULL)
    for (i = parser->next - 1; i >= 0; i--)
      if (token_arr[i].start != -1 && token_arr[i].end == -1)
        return -1;
  return count;
}
//생성된 토큰을 초기화하고 토큰 배열에 추가한다.
 token_t *token_allocator(parser *parser, token_t *token_arr, int arr_size) {
  token_t *tok;
  if (parser->next >= arr_size)
    return NULL;
    //이 토큰은 토큰 배열의 마지막 토큰 다음 토큰을 가리킨다.
  tok = &token_arr[parser->next++];
  tok->start = tok->end = -1;
  tok->size = 0;
  tok->food_type=0;
  tok->score=0;

  return tok;
}
//토큰의 값을 채우는 함수이다. start와 end는 offset가 해당 토큰을 모두 지난 후 토큰의 앞자리와 뒷자리를 의미한다. 이를 통해 토큰의 범위를 설정한다.
 void token_fill(token_t *token, token_type_t type, int start, int end) {
  token->type = type;
  token->start = start;
  token->end = end;
  token->size = 0;
  token->food_type=0;
  token->score=0;
}
//primitive 타입을 파싱하는 함수이다.
 int parsing_primitive(parser *parser, char *js, int len, token_t *token_arr, int arr_size) {
  token_t *token;
  int start;
  start = parser->offset;
  for (; parser->offset < len && js[parser->offset] != '\0'; parser->offset++) {
    switch (js[parser->offset]) {
      case '\t':
      case '\r':
      case '\n':
      case ' ':
      case ',':
      case ']':
      case '}':
        goto found;
    }
  }
  //primitive를 parsing 하던 중 tap, new line 등이 나왔을 때 만약에 allocate한 token이 없다면, offset를 다시 한칸 뒤로 옮기고 함수를 종료한다.
  found:
    if (token_arr == NULL) {
      parser->offset--;
      return 0;
  }
  token = token_allocator(parser, token_arr, arr_size);
  if (token == NULL) {
    parser->offset = start;
    return -1;
  }
  token_fill(token, PRIMITIVE, start, parser->offset);
  parser->offset--;
  return 0;
}
//string type을 파싱하는 함수이다.
 int parsing_string(parser *parser, char *js, int len, token_t *token_arr, int arr_size) {
  token_t *token;

  int start = parser->offset;
//이 함수를 부르는 시점은 offset가 "를 가리키고 있는 시점이다. 그 다음 string을 파싱하기 위해서 offset를 한칸 뒤로 옮긴다.
  parser->offset++;

  for (; parser->offset < len && js[parser->offset] != '\0'; parser->offset++) {
    char c = js[parser->offset];
    //offset를 계속 뒤로 옮기던 중, "를 만날 경우, 파싱을 마치고 종료한다. 함수를 부를 때의 "와 이 "가 만나게 되며 하나의 String Token의 범위가 완성된다.
    if (c == '\"') {
      if (token_arr == NULL) {
        return 0;
      }
      token = token_allocator(parser, token_arr, arr_size);
      if (token == NULL) {
        parser->offset = start;
        return -1;
      }
      token_fill(token, STRING, start + 1, parser->offset);

      return 0;
    }
  }
  parser->offset = start;
  return -1;
}
void *realloc_it(void *ptrmem, int size) {
  void *p = realloc(ptrmem, size);
  return p;
}
void typer(char *js,token_t *t,char *master_buf){
  strncpy(master_buf,js+t->start,t->end-t->start);
  if(strcmp(master_buf,"한식")==0)
    t->food_type=KOREAN;
  else if(strcmp(master_buf,"중식")==0)
    t->food_type=CHINESE;
  else if(strcmp(master_buf,"양식")==0)
    t->food_type=WESTERN;
  else if(strcmp(master_buf,"일식")==0)
    t->food_type=JAPANESE;
  else if(strcmp(master_buf,"고기류")==0)
    t->food_type=BARBEQUE;
  else if(strcmp(master_buf,"야식")==0)
    t->food_type=MIDNIGHT;
  memset(master_buf,0,sizeof(master_buf));
}
/*void typeFinder(char *js,token_t *t,char *master_buf){
  if(t->food_type==CHINESE)
    printf("와! 춘장! ");
  else if(t->food_type==KOREAN)
    printf("김치!! 김치!! ");
  else if(t->food_type==WESTERN)
    printf("자 찍습니다 치이ㅣㅣㅣㅣㅣㅣ즈! ");
  else if(t->food_type==JAPANESE)
    printf("이랴쎼이마세! ");
  else if(t->food_type==BARBEQUE)
    printf("아...불고기...! ");
  else if(t->food_type==KOREAN)
    printf("야식은 살 안쪄요 ");
  }*/
_Bool score_getter(char *js,token_t *t,char *master_buf){
  strncpy(master_buf,js+t->start,t->end-t->start);
  return strcmp(master_buf,"점수")==0;
}
_Bool name_getter(char *js,token_t *t,char *master_buf){
  //strncpy(master_buf,js+t->start,t->end-t->start);
  return strcmp(master_buf,"이름")==0;
}
/*void namer(char *js,token_t *t,char *master_buf){
//  strncpy(master_buf,js+t->start,t->end-t->start);
//  strcpy(t->name,master_buf);
  //printf("%s",t->name);
  //memset(master_buf,0,sizeof(master_buf));
}*/
void scorer(char *js,token_t *t,char *master_buf){
  strncpy(master_buf,js+t->start,t->end-t->start);
  t->score=atoi(master_buf);
  memset(master_buf,0,sizeof(master_buf));
}
 int token_printer(char *js, token_t *t, int count, int indent,int*num) {
   int i, j, k;
   token_t *key;
   if (count == 0) {
     return 0;
   }
   if (t->type == PRIMITIVE) {
     printf("[%d] %d", *num, stringToInt(js, t));
     printf(" (size = %d, %d ~ %d, PRIMITIVE)\n", t->size, t->start, t->end);
     return 1;
   }
   else if (t->type == STRING) {
     printf("[%d] %.*s", *num, t->end - t->start, js + t->start);
     printf(" (size = %d, %d ~ %d, STRING)\n", t->size, t->start, t->end);
     return 1;
   }
   else if (t->type == OBJECT) {
     printf("[%d] ", *num);
     for (int i = t->start; i < t->end; i++){
       printf("%c", js[i]);
     }
     printf(" (size = %d, %d ~ %d, OBJECT)\n", t->size, t->start, t->end);
     j = 0;
     for (i = 0; i < t->size; i++) {
       key = t + 1 + j;
       ++(*num);
       j += token_printer(js, key, count - j, indent + 1, num);
       if (key->size > 0) {
         ++(*num);
         j += token_printer(js, t + 1 + j, count - j, indent + 1, num);
       }
     }
     return j + 1;
   }
   else if (t->type == ARRAY) {
     printf("[%d] ", *num);
     for (int i = t->start; i < t->end; i++){
       printf("%c", js[i]);
     }
     printf(" (size = %d, %d ~ %d, ARRAY)\n", t->size, t->start, t->end);
     j = 0;
     for (i = 0; i < t->size; i++) {
       ++(*num);
       j += token_printer(js, t + 1 + j, count - j, indent + 1, num);
     }
     return j + 1;
   }
   return 0;

}
int stringToInt (const char *js, token_t *t){
  int num = 0;
  for (int i = t->start; i < t->end; i++){
    num = num * 10 + (js[i]-48);
  }
  return num;
}
