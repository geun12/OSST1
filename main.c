#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  true = 1,
  false = 0
} bool;

void answer (FILE* fi, char select, int check);
void answerA (FILE* fi);
void answerB (FILE* fi, int check);
void answerAll(char select, int check);
void printCotents (char* curr, int num);
bool isNumber (char num);

int main (void){
  char select = 0;

  while (select != 'q' && select != 'Q'){
    FILE* fi = NULL;
    char filename[15] = " ";
    int check = 0;
    char menu = 0;

    printf("\t------------------------menu------------------------\n");
    printf("\t  A. 한식, 중식, 양식, 일식, 고기집, 야식 중에서 선택\n");
    printf("\t  B. 특정 점수 이상의 음식점만 보기\n");
    printf("\t  Q. 끝내기\n");
    printf("\t----------------------------------------------------\n");
    printf("\t  위의 메뉴 중에서 선택하기: ");
    scanf(" %c", &select);

    switch (select){
      case 'B':
      case 'b': {
        printf("\t  점수 입력: ");
        scanf(" %d", &check);
      }
      case 'A':
      case 'a':{
      printf("\t  한식, 중식, 양식, 일식, 고기집, 야식, 모두보기 중에서 선택");
      printf("\n\t   (한식 = 1, 중식 = 2, 양식 = 3, 일식 = 4, 고기집 = 5, 야식 = 6, 모두보기 = 0)\n\t  선택하기: ");
      scanf(" %c", &menu);
      switch (menu){
        case '0': {break;}
        case '1': {strcpy(filename, "Korea.txt"); break;}
        case '2': {strcpy(filename, "China.txt"); break;}
        case '3': {strcpy(filename, "Western.txt"); break;}
        case '4': {strcpy(filename, "Japan.txt"); break;}
        case '5': {strcpy(filename, "Meat.txt"); break;}
        case '6': {strcpy(filename, "Dinner.txt"); break;}
        default: {printf("Check your answer, please\n"); break;}
      }
      
      break;
      }
      case 'Q':
      case 'q': {printf("Bye!"); break;}
      default: printf("Check your answer, please.\n");
    }
//menu가 0이 아니라면 filename에 따라서 fopen 해준 후 answer 함수를 불러서 사용자가 입력한 option에 따라 출력할 수 있도록 함.
    if (menu != '0'){
      fi = fopen (filename, "r");
      answer(fi, select, check);
      fclose(fi);
    } else {
//menu가 0이라면 FILE 포인터를 넣지 않고 answerAll 함수를 부름.
      answerAll(select, check);
    }
  }

  return 0;
}

/*
select로 사용자가 A option을 선택했는지 B option을 선택했는지 확인한 후,
A를 선택했으면 answerA 함수로 가도록 하고 B를 선택했으면 answerB 함수로 가도록 함.
*/
void answer (FILE* fi, char select, int check){
  if (select == 'A' || select == 'a')
    answerA(fi);
  else if (select == 'B' || select == 'b')
    answerB(fi, check);
}

/*
FILE 포인터를 통해서 txt 파일에서 한줄로 끝나는 string ( ex.[1] Korean (size = 1, 7 ~ 13, STRING) )
을 확인한 후, printContents 함수를 호출하여 내용을 출력하도록 함.
*/
void answerA (FILE* fi){
  char* curr = (char*)malloc(256 * sizeof(char));
  int num = 0;

  while (fgets(curr, 256, fi) != NULL){
    char endChar;

    for (int i = 0; curr[i] != '\n'; i++)
      endChar = curr[i];
    
    if (curr[0] == '[' && endChar == ')'){
      //한줄로 끝나는 string ( ex.[1] Korean (size = 1, 7 ~ 13, STRING) ) 의 경우에만 printContents 함수를 호출함.
      if ((num-1) % 6 == 0) printf("\n\n\t  -");
      printCotents(curr, num++);
    }
  }
  printf("\n\n\n");
  free(curr);
}

/*
char 포인터 안에 내용만 출력되도록 char 배열을 만든 후 출력함.
*/
void printCotents (char* curr, int num){
  int i = 0, size = 0, j = 0;
  bool check1 = false;
  bool check2 = true;
  char content[256] = "";

  for (i = 0; curr[i] != '\n'; i++){
    if (curr[i] == ']') {
      check1 = true;
      continue;
      //curr[i]가 ']'인 경우는 배열이 끝난 경우와 [3] 이런 식으로 숫자 카운트가 끝난 경우임.
      //1. 배열이 끝난 경우: printCotents 함수가 한줄로 끝나는 경우에만 호출이 되므로 포함되지 않는 경우임.
      //2. 숫자 카운트가 끝난 경우: 뒤에 원하는 내용이 나오므로 content char 배열에 값을 넣어줌.
      //-> curr[i]가 '(' 를 만나기 전까지 (size = 0, 1023 ~ 1032, STRING) 의 형식의 시작이므로 내용이 끝남과 동일함.
    }
    else if (curr[i] == '(') break;

    if (check1) content[j++] = curr[i];
  }

  while (curr[i] != '\n'){
    //curr[i]의 값은 '('의 경우와 curr[i]는 '\n'의 값인 경우가 있음.
    //1. curr[i] == '\n'의 경우: while loop에 들어올 수 없으므로 고려하지 않음.
    //2. curr[i] == '('의 경우: (size = 0, 1023 ~ 1032, STRING) 의 형식의 시작
    if (check2 && curr[i] == ',') check2 = false;
    //','인 경우 뒤에 범위가 올 수 있으므로 size를 가지기 위해서는 적절하지 않음.
    //-> check2의 값을 false로 바꾸어 뒤에 있는 숫자는 출력하지 않도록 함.

    if (check2 && isNumber(curr[i])){
      size = curr[i] - 48;
    }
    i++;
  }

  if (size == 0) printf(" : %s", content);
  else {
    if (num != 0) printf("\n\t   ");
    else printf("\n\t");
    printf("%s", content);
  }
}

/*
FILE 포인터를 통해서 txt 파일에서 OBJECT 인 부분만 확인하여 OBJECT 타입의 사이즈가 3인 경우와
그 속의 음식점의 point 점수가 사용자가 입력한 point 점수 이상이라면 printContents 함수를 호출하여 내용을 출력하도록 함.
*/
void answerB (FILE* fi, int check){
  char* curr = (char*)malloc(256 * sizeof(char));
  int point = 0, size = 0;
  int num = 0;
  int j = 0;

  while (fgets(curr, 256, fi) != NULL){
    bool check1 = false;
    bool check2 = false;
    char endChar;

    for (int i = 0; curr[i] != '\n'; i++){
      if (curr[i+1] == '\n' && curr[0] == '[' && curr[i] == ')' && num == 0)
        printCotents(curr, num++);
        //맨 처음 나오는 STRING 인 종류(한식, 중식, 일식, 양식, 등등)를 출력하도록 함.

      if (curr[i] == ':') check1 = true;
      //curr[i] == ':' 인 경우 뒤에 value값이 나옴. 그중 point인 경우 음식점의 점수를 얻을 수 있음.
      else if (check1 && curr[i] == '"') check1 = false;
      //만약 curr[i] == '"'라서 STRING 값이 나온다면 뒤에 숫자가 나와도 무의미하므로 check1을 false로 만듦.

      if (curr[i] == '(') check2 = true;
      //curr[i] == '('인 경우 뒤에 (size = 0, 1023 ~ 1032, STRING) 형식이 나옴.
      else if (check2 && curr[i] == ',') check2 = false;
      //','인 경우 뒤에 범위가 올 수 있으므로 size를 가지기 위해서는 적절하지 않음.
      //-> check2의 값을 false로 바꾸어 뒤에 있는 숫자는 출력하지 않도록 함.

      if (check1 && isNumber(curr[i])) {
        point = curr[i] - 48;
        //curr[i] 의 값이 숫자인 경우, point의 값을 받음.
      }
      if (check2 && isNumber(curr[i])){
        size = curr[i] - 48;
        //curr[i] 의 값이 숫자인 경우, size의 값을 받음.
      }
    }

    if (point >= check && size == 3){
      //음식점의 이름과 point, 한줄평을 가지는 하나의 OBJECT의 경우 size가 3임.
      printf("\n\t  -");
      for (int k = 0; k < 6; k++){
        if (fgets(curr, 256, fi) == NULL) {
          free(curr);
          return;
        }

        printCotents(curr, num++);
      }
      printf("\n");
      point = 0, size = 0;
    }
  }
  printf("\n\n");

  free(curr);
}

/*
character가 숫자인지 아닌지 확인함.
*/

bool isNumber (char num){
  if (num >= '0' && num <= '9') return true;

  return false;
}

/*
만약 사용자가 모든 음식점을 출력하고 싶어한다면 answerAll 함수를 사용하여 모든 파일이름에 따라서 answer 함수를 부른 후,
파일에 사용자가 입력한 option에 따라 출력하도록 함.
*/
void answerAll(char select, int check){
  char filenames[7][15] = {"Korea.txt", "China.txt", "Western.txt", "Japan.txt", "Meat.txt", "Dinner.txt"};
  for (int i = 0; i < 6; i++){
    FILE* fi = fopen(filenames[i], "r");
    answer(fi, select, check);
    fclose(fi);
  }
}