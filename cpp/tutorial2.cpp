#include <iostream>
#include "math.h"

using namespace std;

int circle(const double r, double *area, double *length) {
    *area = PI * r * r;
    *length = 2 * PI * r;
    return 0;
}

int main()
{
    double a {10.0};
    double* area = new double;
    double* length = new double;
    circle(a, area, length);
    cout << *area << endl;
    cout << *length << endl;
    return 0;
}