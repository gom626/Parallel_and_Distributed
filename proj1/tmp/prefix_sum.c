#include <stdio.h>

int main(void){
	int i=0;
	int sum=0;
	for(i=0;i<1000000;i++){
		sum+=i;
	}
	printf("%d\n",sum);
	return 0;
}
