#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <Windows.h>

using namespace std;

void main()
{
	string localString = "Ermolovich Lab";

	while (1)
	{
		cout << localString.c_str() << endl;
		_getch();
	}
}