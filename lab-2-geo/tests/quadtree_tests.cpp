#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "quadtree.h"

class QuadtreeTest : public ::testing::Test
{
protected:
    Rect _world_area = Rect(-100.0, -100.0, 100.0, 100.0);

    bool hasPointWithId(const std::vector<Point>& points, uint64_t id)
    {
        return std::any_of(points.begin(), points.end(),
                           [id](const Point& p) { return p.id == id; });
    }
};

// Поиск в пустом дереве
TEST_F(QuadtreeTest, QueryEmptyTree)
{
    QuadtreeNode qt(_world_area);
    std::vector<Point> results;
    Rect search_area(-10.0, -10.0, 10.0, 10.0);

    qt.queryRange(search_area, results);
    EXPECT_TRUE(results.empty());
}

// Точка ровно на границе
TEST_F(QuadtreeTest, PointOnMinBoundary)
{
    QuadtreeNode qt(_world_area);
    Point p(_world_area.min_x, _world_area.min_y, 1);
    qt.insertPoint(p);

    std::vector<Point> results;
    qt.queryRange(Rect(-100.0, -100.0, -50.0, -50.0), results);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].id, 1);
}

// Точка ровно на линии разреза
TEST_F(QuadtreeTest, PointOnCenterSplit)
{
    QuadtreeNode qt(_world_area);
    Point p(0.0, 0.0, 42);
    qt.insertPoint(p);

    std::vector<Point> results;

    qt.queryRange(Rect(-10.0, -10.0, 0.0, 0.0), results);
    EXPECT_TRUE(hasPointWithId(results, 42));
    results.clear();

    qt.queryRange(Rect(0.0, 0.0, 10.0, 10.0), results);
    EXPECT_TRUE(hasPointWithId(results, 42));
}

// Переполнение узла
TEST_F(QuadtreeTest, NodeSubdivision)
{
    QuadtreeNode qt(_world_area);
    for (int i = 0; i < 20; ++i)
        qt.insertPoint(Point(i - 10, i - 10, i));

    std::vector<Point> results;
    qt.queryRange(Rect(-100.0, -100.0, 100.0, 100.0), results);
    EXPECT_EQ(results.size(), 20);
}

// Глубокое дерево
TEST_F(QuadtreeTest, DeepTreeLimit)
{
    QuadtreeNode qt(_world_area);
    for (int i = 0; i < 50; ++i)
        qt.insertPoint(Point(1.0, 1.0, i));

    std::vector<Point> results;
    qt.queryRange(Rect(0.0, 0.0, 2.0, 2.0), results);
    EXPECT_EQ(results.size(), 50);
}

// Поиск вне границ мира
TEST_F(QuadtreeTest, QueryOutsideWorld)
{
    QuadtreeNode qt(_world_area);
    qt.insertPoint(Point(10.0, 10.0, 1));

    std::vector<Point> results;
    qt.queryRange(Rect(200.0, 200.0, 300.0, 300.0), results);
    EXPECT_TRUE(results.empty());
}

// Частичное пересечение Rect
TEST_F(QuadtreeTest, PartialOverlap)
{
    QuadtreeNode qt(_world_area);
    qt.insertPoint(Point(50.0, 50.0, 1));
    qt.insertPoint(Point(5.0, 5.0, 2));

    std::vector<Point> results;
    qt.queryRange(Rect(0.0, 0.0, 10.0, 10.0), results);

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].id, 2);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
