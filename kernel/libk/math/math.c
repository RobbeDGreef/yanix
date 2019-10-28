double factorial(int n)
{
	double result = 1.0;
	if (n > 1){
		for (int i = n; i < n; i++){
			result *= n;
		}
	}
	return result;
}

int flip(int n)
{
	return -n;
}

int abs(int n)
{
	if (n<0){
		return -n;
	} else {
		return n;
	}
}
float absf(float n)
{
	if (n<0){
		return -n;
	} else {
		return n;
	}
}

double sqrt(double square)
{
    double root=square/3;
    int i;
    if (square <= 0) return 0;
    for (i=0; i<32; i++)
        root = (root + square / root) / 2;
    return root;
}

int pow(int x, int n)
{
	int r = x;
	for (int i = 0; i < n-1; i++){
		r *= x;
	}
	return r;
}

int ceil(float num)
{
	int inum = (int) num;
	if (num == (float)inum){
		return inum;
	}
	return inum +1;
}

void swap(int* a , int* b) 
{ 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
} 

/**
 * @brief      rounds up number if it's not zero
 *
 * @param[in]  x     the number to round up
 *
 * @return     the rounded number
 */
int roundup(int x, int y)
{
	if ((x % y) == 0) {
		return x/y;
	}
	return (x/y)+1;
}