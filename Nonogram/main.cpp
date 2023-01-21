#include <iostream>
#include <string>
#include <vector>
#include <iostream>
#include <windows.h>

#include <fcntl.h>
#include <io.h>

#include "Grid.h"

using namespace std;

int main(int argc, char** argv) {
	vector<string> args { argv, argv + argc };

	_setmode(_fileno(stdout), _O_U8TEXT);

	if (args.size() != 2)
		cout << "The only argument should be the puzzle name" << endl;

	Grid grid{ args[1] };
	grid.Solve();

	
	wcout << L"At the end: " << endl << grid << endl;
}
