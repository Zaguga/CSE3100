#include <stdio.h>
#include <stdlib.h>

int main()
{
	int n, i;

	printf("n = ");
	scanf("%d", &n);

	double pi = 0.;
	//TODO
	//add code below 
	for (int i = 0; i <= n; i++)
	{
		double sum;
		double pow = 1.0;

		sum = 4.0 / ((8.0*i) + 1.0);
		sum = sum - 2.0 / ((8.0*i) + 4.0);
		sum = sum - 1.0 / ((8.0*i) + 5.0);
		sum = sum - 1.0 / ((8.0*i) + 6.0);

		for (int j = 0; j < i; j++) //j < i bc if j <= i then when i is 0 it will add a *16 and since pow is 1, it wont be 0 but will add an extra /16 to the formula
		{
			pow = pow * 16.0;
		}

		sum = sum * (1/pow);

		pi = pi + sum;
	}







	printf("PI = %.10f\n", pi);
	return 0;
}
