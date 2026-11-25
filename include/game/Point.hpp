#ifndef _POINT_HPP_
#define _POINT_HPP_

class Point {
public:
    int x;
    int y;

    Point() : x(0), y(0) {}
    Point(int x_coord, int y_coord) : x(x_coord), y(y_coord) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    bool operator<(const Point& other) const {
        if (x != other.x) {
            return x < other.x;
        }
        return y < other.y;
    }
};

#endif // _POINT_HPP_