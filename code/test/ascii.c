#include"stdio.h"
#define START_ASCII 32 
#define END_ASCII 126
int main(){

    for(int i = START_ASCII; i <= END_ASCII; i++){
        char tmp = (char)i;
        printf("%c %d\n", tmp, i);
    }
}