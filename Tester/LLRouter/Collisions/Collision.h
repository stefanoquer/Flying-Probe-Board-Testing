#pragma once

#include "../../Common/Math.h"
#include <array>
#include <cstdint>
#include <vector>

namespace LLRouter
{
	class ZoneSegment;

	class Probe;
} // namespace LLRouter

namespace Math::Collision
{
	void LineCollisions(const Segment &segment, const std::vector<LLRouter::ZoneSegment> &against, bool top,
						std::vector<Point> &result) noexcept;

	void RectangleCollision(const Segment &segment, const std::vector<LLRouter::ZoneSegment> &against, bool top,
							std::vector<Point> &result) noexcept;

	template <size_t nDivisions> static std::array<Point, nDivisions + 1> DivideSegment(const Segment &segment)
	{
		static_assert(nDivisions > 0, "Can not divide segment by 0");
		std::array<Point, nDivisions + 1> result;
		result[0] = segment.start;
		result.back() = segment.end;
		double stepX = (segment.end.x - segment.start.x) / static_cast<double>(nDivisions);
		double stepY = (segment.end.y - segment.start.y) / static_cast<double>(nDivisions);
		for (uint64_t i = 1; i < nDivisions - 1; i++)
		{
			result[i] = Point{.x = segment.start.x + stepX * static_cast<double>(i),
							  .y = segment.start.y + stepY * static_cast<double>(i)};
		}
		result[nDivisions] = segment.end;
		return result;
	}

	void SegmentDivision(const Segment &segment, const std::vector<LLRouter::ZoneSegment> &against, bool top,
						 std::vector<Point> &result) noexcept;

	void SegmentZoneCollision(const Segment &segment, const std::vector<LLRouter::ZoneSegment> &against, bool top,
							  LLRouter::Probe *probe);

	Point GenericPathCollision(const Segment &segment, const std::vector<LLRouter::ZoneSegment> &against, bool top);

	Point SegmentSegment(const Segment &first, const Segment &second);

	Point PointInsideRectangle(const Point &p, const Point &topLeft, const Point &width);
} // namespace Math::Collision
