#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
int main(){
	int a; 
	scanf("%d", &a);
	if(a < 0){
		fprintf(stderr, "error!\n");
		return 1;
	}
	printf("%d\n", a);
	return 0;
}
