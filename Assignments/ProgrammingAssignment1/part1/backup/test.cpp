
#include <iostream>

using namespace std ;

int increment(int i)
{
	return ++i;
}

int main(void)
{
	int i = 0; 

	while ((i = increment(i)) < 10)
	{
		cout << i << endl;
	}

	return 0;
}
