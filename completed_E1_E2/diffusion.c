#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//TODO
//Implement the below function
//Simulate one particle moving n steps in random directions
//Use a random number generator to decide which way to go at every step
//When the particle stops at a final location, use the memory pointed to by grid to 
//record the number of particles that stop at this final location
//Feel free to declare, implement and use other functions when needed

void one_particle(int *grid, int n)
{
	int x = n;
	int y = n;
	int z = n;
	
	for(int i = 0; i < n; i++)
	{
		int step = rand() % 6;

		if (step == 0) x--;
		if (step == 1) x++;
		if (step == 2) y++;
		if (step == 3) y--;
		if (step == 4) z++;
		if (step == 5) z--;
	}

	// to access (x,y,z)
	//grid[x * (rows * cols) + y * cols + z] += value;
	long size = 2 * (long)n + 1;
	long idx = ((long)z * size * size) + ((long)y * size) + ((long)x);
	grid[idx] += 1;
}

//TODO
//Implement the following function
//This function returns the fraction of particles that lie within the distance
//r*n from the origin (including particles exactly r*n away)
//The distance used here is Euclidean distance
//Note: you will not have access to math.h when submitting on Mimir
double density(int *grid, int n, double r)
{
	long size = 2 * (long)n + 1;

	long total = 0;
	long inside = 0;

	double radius = r * (double)n;
	double r_sq = radius * radius;

	for (long z = 0; z < size; z++){
		for (long y = 0; y < size; y++){
			for (long x = 0; x < size; x++){
				long idx = (z * size * size) + (y * size) + (x);
				int count = grid[idx];
				if (count == 0) continue;
				total += count;

				int dx = (int)x - n;
				int dy = (int)y - n;
				int dz = (int)z - n;

				double d = ((double)dx * dx) + ((double)dy * dy) + ((double)dz * dz);
				if (d <= r_sq){
					inside += count;
				}
			}
		}
	}
	if (total == 0) return 0.0;
		return (double)inside / (double)total; 

}

//use this function to print results
void print_result(int *grid, int n)
{
    printf("radius density\n");
    for(int k = 1; k <= 20; k++)
    {
        printf("%.2lf   %lf\n", 0.05*k, density(grid, n, 0.05*k));
    }
}

//TODO
//Finish the following function
//See the assignment decription on Piazza for more details
void diffusion(int n, int m)
{
	//fill in a few line of code below
	long grid_dim = (2*(long)n+1) * (2*(long)n+1) * (2*(long)n+1);
	int *grid = calloc(grid_dim, sizeof(int));
	for(int i = 1; i<=m; i++) one_particle(grid, n);
	if (grid == NULL){
		perror("Calloc Failed");
		exit(1);
	}

	print_result(grid, n);
	//fill in some code below
	free(grid);

}

int main(int argc, char *argv[])
{
	
	if(argc != 3)
	{
		printf("Usage: %s n m\n", argv[0]);
		return 0; 
	}
	int n = atoi(argv[1]);
	int m = atoi(argv[2]);

	assert(n >= 1 && n <=50);
	assert(m >= 1 && m <= 1000000);
	srand(12345);
	diffusion(n, m);
	return 0;
}




//RUN WITH:
//    gcc -std=c11 -Wall -Wextra -Werror diffusion.c -o diffusion 
//    ./diffusion 10 1000000