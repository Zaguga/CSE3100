#include <stdio.h>
#include <stdlib.h>

int oddSumHelp(int count, int bound, int value)
{
	//4 base cases

	//base case 1: if solution is found
	if (count == 0 && value == 0){
		return 1; 
	}
	//base case 2: if value less than 0, meaning over shot value
	if(value < 0){
		return 0;
	}
	if(count < 0){
		return 0;
	}
	//base case 3: if bound is 0, ran out of numbers to try
	if(bound < 0){
		return 0;
	}
	//base case 4: recursion
	if (oddSumHelp(count-1, bound-2, value-bound)){
		printf("%d ", bound);
		return 1;
	}
	else{
		return oddSumHelp(count, bound-2, value);
	}
}




//Do not change the code below
void oddSum(int count, int bound, int value)
{
    	if(value <= 0 || count <= 0 || bound <= 0) return;
    
    	if(bound % 2 == 0) bound -= 1;

    	if(!oddSumHelp(count, bound, value)) printf("No solutions.\n");
	else printf("\n");
}

int main(int argc, char *argv[])
{
	if(argc != 4) return -1;

	int count = atoi(argv[1]);
	int bound = atoi(argv[2]);
	int value = atoi(argv[3]);

	//oddSum(12,30,200);
	//oddSum(10,20,100);
	//oddSum(20,20,200);
	oddSum(count, bound, value);
	return 0;
}
