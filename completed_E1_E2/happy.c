#include <stdio.h>
#include <stdlib.h>

int main()
{
	int n;

	printf("n = ");
	scanf("%d", &n);

	int m = n;
	//TODO
	//add code below
	int sum = 0;
	while (n != 1 && n != 4)
	{
		while (n > 0)
		{
		int digit = n%10;
		n = n/10;
		sum = sum + (digit * digit);
		}
		n = sum;
		printf("%d\n",n);
		sum = 0;
	}



//./a.out



	if(n==1) printf("%d is a happy number.\n", m);
	else printf("%d is NOT a happy number.\n", m);
	return 0;
}
