// Only this line of comment is provided 
#include <stdio.h>
int main() {
    int counter = 0;
    double total = 0, average = 0, x;

    while (scanf("%lf", &x) == 1) {
        counter = counter + 1;
        total = total + x;
        average = total/counter;
        printf("Total=%f Average=%f\n", total, average);
    }

    return 0;

}