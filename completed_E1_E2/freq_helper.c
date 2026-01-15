#include <assert.h>

//TODO
unsigned firstDigit(unsigned k)
{
	unsigned num = k;
	while (num/10 > 0){
		num = num/10;
	}
	return num;
	
}

//TODO -- DONE
void FirstDigitCount(unsigned *a, unsigned n, unsigned freq[10])
{
	for (int j = 0; j < 10; j++){ //initialize the array to 0 so we can just simply increment it if theres digits in arr a
		freq[j] = 0;
	}
	unsigned digit;
	int idx;

	for (int i = 0; i < n; i++){ // for every number we are checking in a, simplified with n variable
		digit = firstDigit(a[i]); //get the first digit and save it
		idx = (int) digit; //size we are using it as an index we need it to be an integer
		freq[idx] += 1; //go to the index of that digit in freq, and increment by 1
	}
	//note no return, void function
}

