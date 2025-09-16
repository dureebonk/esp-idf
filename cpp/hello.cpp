#include <iostream>
#include <complex>
#include <vector>

using namespace std;

double square(double x) {
    return x*x;
}

void print_squre(double x) {
    cout << "the square of " << x << " is " << square(x) << endl;
}

int main()
{
    auto a = 1;
    while( a < 5 ) {
        cout << a << endl;
        a++;
    }

    for (auto b = 1; b < 5; b++) {
        cout << b << endl;
    }
}