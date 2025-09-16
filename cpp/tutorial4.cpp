#include <iostream>
#include "math.h"
using namespace std;

int main() {
    Circle c1(10);
    Circle* c2 = new Circle(20);
    cout << c1.getArea() << endl;
    cout << c2->getArea() << endl;
    delete c2;
    return 0;
}