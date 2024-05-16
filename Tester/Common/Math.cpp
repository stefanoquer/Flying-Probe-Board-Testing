//
// Created by Andrea Calabrese on 09/05/22.
//

#include "Math.h"
#include <algorithm>
#include <cmath>

namespace Math
{

	double Point::Distance(const Point &a, const Point &b)
	{
		auto x = a.x - b.x;
		auto y = a.y - b.y;
		return sqrt(x * x + y * y);
	}

	double Point::GDistance(const Point &a, const Point &b)
	{
		/*
		const double dx = abs(a.x - b.x);
		const double dy = abs(a.y - b.y);
		const double timex = - 4.07605736582576e-20 * dx * dx * dx * dx
							 + 3.36959418008742e-14 * dx * dx * dx
							 - 1.01324894668801e-8 * dx * dx
							 + 0.0019951263455892 * dx
							 + 72.8645284317949;
		const double timey = 2.64850687198754e-14 * dy * dy * dy
							 - 1.36054101863365e-8 * dy * dy
							 + 0.00298595851932793 * dy
							 + 39.125243693251;
		return std::max(timex, timey);
		 */
		return Point::Distance(a, b);
	}

	bool operator==(const Point &p1, const Point &p2)
	{
		return (std::isnan(p1.x) && std::isnan(p2.x)) || (p1.x == p2.x && p1.y == p2.y);
	}

	bool operator!=(const Point &p1, const Point &p2)
	{
		return !(p1 == p2);
	}

	std::array<Segment, 5> Segment::CircumscribedRectangle() const noexcept
	{
		std::array<Segment, 5> result{};
		// Top horizontal
		result[0] = Segment{.start = start, .end = Point{.x = end.x, .y = start.y}};
		// Left vertical
		result[1] = Segment{.start = start, .end = Point{.x = start.x, .y = end.y}};
		// Bottom horizontal
		result[2] = Segment{.start = Point{.x = start.x, .y = end.y}, .end = end};
		// Right vertical
		result[3] = Segment{.start = Point{.x = end.x, .y = start.y}, .end = end};

		result[4] = *this;
		return result;
	}
} // namespace Math
