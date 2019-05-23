#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    UNDEFINED = 0,
    OBJECT = 1,
    ARRAY = 2,
    STRING = 3,
    PRIMITIVE = 4
}type_t;

typedef struct {
    type_t type;
    int start;
    int end;
    int size;
} tok_t;

void checkType (char* line, tok_t* token, int size);
void checkSize (char* line, tok_t* token, int size);
void printType(tok_t* token, int size);
void printSize(tok_t* token, int size);

int main (void) {
    char line[100] = "{\"name\":false, \"age\":53, \"major\":[\"computer\",\"electronic\"]}";
    tok_t* token = (tok_t*)malloc(100*sizeof(tok_t));
    int i = 0;
    
    token[0].start = 2;
    token[0].end = 6;
    token[1].start = 8;
    token[1].end = 13;
    token[2].start = 16;
    token[2].end = 19;
    token[3].start = 21;
    token[3].end = 23;
    token[4].start = 26;
    token[4].end = 31;
    token[5].start = 33;
    token[5].end = 57;
    /**
    if (argc > 0){
        fp = fopen(argv, "r");
    }else{
        printf("please input file name");
        exit(-1);
    }
    token = (tok_t*)malloc(checkSize(fp)*sizeof(tok_t));
    **/
    checkType(line, token, 6);
    checkSize(line, token, 6);

    printType(token, 6);
    printSize(token, 6);

    return 0;
}
void printType(tok_t* token, int size){
    for (int i = 0; i < size; i++){
        if (token[i].start == token[i].end) return;
        if (token[i].type == UNDEFINED){
            printf("UNDEFINED.\n");
        } else if (token[i].type == OBJECT){
            printf("OBJECT.\n");
        } else if (token[i].type == ARRAY){
            printf("ARRAY.\n");
        } else if (token[i].type == STRING){
            printf("STRING.\n");
        } else if (token[i].type == PRIMITIVE){
            printf("PRIMITIVE.\n");
        }
    }
}
void printSize(tok_t* token, int size){
    for (int i = 0; i < size; i++){
        if (token[i].start == token[i].end) return;
        printf("%d ", token[i].size);
    }
}

void checkType (char* line, tok_t* token, int size){
    for (int i = 0; i < size; i++){
        if (token[i].start == token[i].end) return;
        type_t curr = UNDEFINED;

        if (line[token[i].start] == '['){
            token[i].type = ARRAY;
            continue;
        }
        if (line[token[i].start] == '{'){
            token[i].type = OBJECT;
            continue;
        }
        if (line[token[i].start-1] == '"'){
            token[i].type = STRING;
            continue;
        }

        int size = token[i].end-token[i].start;
        int start = token[i].start;
        char* check = (char*)malloc(size*sizeof(char));
        for (int j = 0; j < size; j++){
            check[j] = line[start++];
        }

        if (strcmp("true", check) == 0 || strcmp("false", check) == 0 || strcmp("null", check) == 0){
            token[i].type = PRIMITIVE;
            continue;
        }
        if (line[token[i].start] == '-'){
            if (line[token[i].start+1] >= '0' && line[token[i].start+1] <= '9'){
                token[i].type = PRIMITIVE;
                continue;
            }
        }else {
            if (line[token[i].start] >= '0' && line[token[i].start] <= '9'){
                token[i].type = PRIMITIVE;
                continue;
            }
        }
    }
}

void checkSize (char* line, tok_t* token, int size){
    for (int i = 0; i < size; i++){
        if (token[i].start == token[i].end) return;
        int size2 = 0;
        if (token[i].type == STRING){
            for (int j = token[i].end; ; j++){
                if (line[j] == ':') {
                    token[i].size = 1;
                    break;
                }
                if (line[j] == ',') {
                    token[i].size = 0;
                    break;
                }
            }
        } else if (token[i].type == PRIMITIVE){
            token[i].size = 0;
        } else if (token[i].type == ARRAY){
            for (int j = token[i].start; j < token[i].end; j++){
                if (line[j] == ',') size2++;
            }
            token[i].size = ++size2;
        } else if (token[i].type == OBJECT){
            for (int j = token[i].start; j < token[i].end; j++){
                if (line[j] == ':') size2++;
            }
            token[i].size = size2;
        }
    }
}