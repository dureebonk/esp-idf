#include <iostream>
#include <cstring>

using namespace std;

struct Student
{
    /* data */
    int id;
    char name[20];
};

int main() {
    Student* s1 = new Student{1, "Kim"};
    s1->id = 2;
    strcpy(s1->name, "Lee");
    cout << s1->id << endl;
    cout << s1->name << endl;

    delete s1;
    return 0;
}