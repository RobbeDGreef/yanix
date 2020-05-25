int getbit(int n, int k)
{
	return (n & (1 << k)) >> k;
}