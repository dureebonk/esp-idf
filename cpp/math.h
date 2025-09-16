#define PI 3.14159265358979323846

class Circle {
    public:
        int getRadius() const { 
            return radius;  
        }
        double getArea() const {
            return PI * radius * radius;
        }

        Circle(int r) : radius(r) {}
    private:
        int radius;
};