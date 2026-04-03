#pragma once

#include <vector>
#include <memory>

struct Point
{
    double x_coord; // Долгота (X) -lon lng
    double y_coord; // Широта (Y) - lat
    uint64_t id;

    Point(double x_val, double y_val, uint64_t id_val)
        : x_coord(x_val), y_coord(y_val), id(id_val)
    {
    }
};

struct Rect
{
    double min_x;
    double min_y;
    double max_x;
    double max_y;

    Rect(double min_x, double min_y, double max_x, double max_y);

    bool contains(const Point& p) const;
    bool intersects(const Rect& other) const;
};

class QuadtreeNode
{
public:
    explicit QuadtreeNode(const Rect& boundary, int depth = 0);
    ~QuadtreeNode() = default;

    QuadtreeNode(const QuadtreeNode&) = delete;
    QuadtreeNode& operator=(const QuadtreeNode&) = delete;

    bool insertPoint(const Point& point);

    void queryRange(const Rect& range, std::vector<Point>& results) const;

    void clear();

private:
    static const int kMaxCapacity = 16;
    static const int kMaxDepth = 20;

    Rect _boundary;
    int _depth;
    bool _is_divided;

    std::vector<Point> _points;

    std::unique_ptr<QuadtreeNode> _top_left;
    std::unique_ptr<QuadtreeNode> _top_right;
    std::unique_ptr<QuadtreeNode> _bottom_left;
    std::unique_ptr<QuadtreeNode> _bottom_right;

    void subdivide();
};
