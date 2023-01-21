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

	if (!_setmode(_fileno(stdout), _O_U8TEXT)) {
		cout << "UTF8 could not be enabled..." << endl;
		cout << "The result won't appear in the console" << endl;
	}

	if (args.size() != 2)
		cout << "The only argument should be the puzzle name" << endl;

	Grid grid{ args[1] };
	grid.solve();

	wcout << grid << endl;

	if (grid.is_solved()) {
		wcout << L"Solution found!" << endl;
	}
	else {
		wcout << L"More than one solution available." << endl;
	}
}
