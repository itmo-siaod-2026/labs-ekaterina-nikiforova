#include "quadtree.h"

Rect::Rect(double min_x, double min_y, double max_x, double max_y)
    : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y)
{
}

bool Rect::contains(const Point& p) const
{
    return (p.x_coord >= min_x && p.x_coord <= max_x &&
        p.y_coord >= min_y && p.y_coord <= max_y);
}

bool Rect::intersects(const Rect& other) const
{
    return !(other.min_x > max_x ||
        other.max_x < min_x ||
        other.min_y > max_y ||
        other.max_y < min_y);
}

QuadtreeNode::QuadtreeNode(const Rect& boundary, int depth)
    : _boundary(boundary), _depth(depth), _is_divided(false)
{
    _points.reserve(kMaxCapacity);
}

void QuadtreeNode::subdivide()
{
    double mid_lng = (_boundary.min_x + _boundary.max_x) / 2.0;
    double mid_lat = (_boundary.min_y + _boundary.max_y) / 2.0;

    _top_left = std::make_unique<QuadtreeNode>(
        Rect(_boundary.min_x, mid_lat, mid_lng, _boundary.max_y), _depth + 1);

    _top_right = std::make_unique<QuadtreeNode>(
        Rect(mid_lng, mid_lat, _boundary.max_x, _boundary.max_y), _depth + 1);

    _bottom_left = std::make_unique<QuadtreeNode>(
        Rect(_boundary.min_x, _boundary.min_y, mid_lng, mid_lat), _depth + 1);

    _bottom_right = std::make_unique<QuadtreeNode>(
        Rect(mid_lng, _boundary.min_y, _boundary.max_x, mid_lat), _depth + 1);

    _is_divided = true;

    for (const auto& p : _points)
    {
        if (_top_left->insertPoint(p))
            continue;
        if (_top_right->insertPoint(p))
            continue;
        if (_bottom_left->insertPoint(p))
            continue;
        if (_bottom_right->insertPoint(p))
            continue;
    }
    _points.clear();
}

bool QuadtreeNode::insertPoint(const Point& point)
{
    if (!_boundary.contains(point))
        return false;

    if (!_is_divided)
    {
        if (_points.size() < kMaxCapacity || _depth >= kMaxDepth)
        {
            _points.push_back(point);
            return true;
        }
        subdivide();
    }

    if (_top_left->insertPoint(point))
        return true;
    if (_top_right->insertPoint(point))
        return true;
    if (_bottom_left->insertPoint(point))
        return true;
    if (_bottom_right->insertPoint(point))
        return true;

    return false;
}

void QuadtreeNode::queryRange(const Rect& range, std::vector<Point>& results) const
{
    if (!_boundary.intersects(range))
        return;

    if (_is_divided)
    {
        _top_left->queryRange(range, results);
        _top_right->queryRange(range, results);
        _bottom_left->queryRange(range, results);
        _bottom_right->queryRange(range, results);
    }
    else
    {
        for (const auto& p : _points)
        {
            if (range.contains(p))
                results.push_back(p);
        }
    }
}

void QuadtreeNode::clear()
{
    _points.clear();
    _top_left.reset();
    _top_right.reset();
    _bottom_left.reset();
    _bottom_right.reset();
    _is_divided = false;
}
