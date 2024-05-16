#pragma once

#include <cmath>
#include <limits>
#include <array>

namespace Math
{

	struct Point
	{
		double x;
		double y;

		static double Distance(const Point& a, const Point& b);

		static double GDistance(const Point& a, const Point& b);

	};

	bool operator==(const Point& p1, const Point& p2);

	bool operator!=(const Point& p1, const Point& p2);

	constexpr Point InvalidPoint{
			.x = std::numeric_limits<double>::quiet_NaN(),
			.y = std::numeric_limits<double>::quiet_NaN()
	};


	struct Segment
	{
		Point start;
		Point end;

		/*
		* return value: result[0] and result[2] are the horizontal lines
		*				result[1] and result[3] are the vertical lines
		*				result[4] is the direct path itself (diagonal of the rectangle)
		 *				Order:
		 *				- 0: top line
		 *				- 1: left line
		 *				- 2: bottom line
		 *				- 3: right line
		 *				- 4: this segment
		*/
		[[nodiscard]] std::array<Segment, 5> CircumscribedRectangle() const noexcept;
	};
}