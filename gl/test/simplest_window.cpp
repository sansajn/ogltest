// vytvori prazdne okno
#include <iostream>
#include "gl/glut_window.hpp"

using namespace std;

class main_window : public ui::glut_window
{};

int main(int argc, char * argv[])
{
	main_window w;
	cout << "start" << endl;
	w.start();
	cout << "done" << endl;
	return 0;
}
