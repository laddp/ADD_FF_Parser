#include <sstream>
#include <string>
using namespace std;

#include "Util.h"

double DoubleFromChar(char arg[])
{
    string s = arg;
    istringstream iss;
    iss.str(s);

    double f = 0;
    iss >> f;
    return f;
}
