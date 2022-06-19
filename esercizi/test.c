#include <stdio.h>
#include <stdlib.h>

void fun(int *a){
    a=3;
}

int main (void){
    int a=0;
    int b=1;
    printf(" a = %d, b= %d \n",a,b);
    fun(&a);
    printf(" a = %d, b= %d \n",a,b);

}