#include "Collision.h"
#include "../Board/Zones.h"
#include "../Board/Probe.h"
#include <cmath>

using namespace std;

namespace Math::Collision
{
	void LineCollisions(const Segment& segment, const std::vector<LLRouter::ZoneSegment>& against, bool top,
			std::vector<Point>& result) noexcept
	{
		result.clear();
		result.resize(against.size());
		for (size_t i = 0; i < result.size(); i++)
		{
			if (top == against[i].isTop)
			{
				result[i] = SegmentSegment(segment, against[i].segment);
			}
			else
			{
				result[i] = InvalidPoint;
			}
		}
	}

	void RectangleCollision(const Segment& segment, const std::vector<LLRouter::ZoneSegment>& against, bool top,
			std::vector<Point>& result) noexcept
	{
		result.clear();
		result.resize(against.size());
		Point topLeft = segment.start;
		Point width{
				.x = segment.end.x - segment.start.x,
				.y = segment.end.y - segment.start.y
		};

		for (size_t i = 0; i < result.size(); i++)
		{
			if (top == against[i].isTop)
			{
				result[i] = PointInsideRectangle(against[i].segment.start, topLeft, width);
			}
			else
			{
				result[i] = InvalidPoint;
			}
		}
	}

	// TODO
	void SegmentDivision(const Segment& segment, const std::vector<LLRouter::ZoneSegment>& against, bool top,
			std::vector<Point>& result) noexcept
	{
		result.clear();
		Segment temp = segment;
		constexpr size_t NUM_DIVISIONS = 10;
		auto dividedSegment = DivideSegment<NUM_DIVISIONS>(temp);
		auto currentPoint = dividedSegment.data();
		bool okPath = false;
		for (auto nextPoint = &dividedSegment.back(); nextPoint != dividedSegment.data(); nextPoint--)
		{
			if (nextPoint == currentPoint)
			{
				if (currentPoint > &dividedSegment.back())
				{
					result.clear();
				}
				return;
			}
			auto rect = Segment{
					.start = *currentPoint,
					.end = *nextPoint
			}.CircumscribedRectangle();
			array<bool, 6> collisions{};
			vector<Point> tempCollisions;
			for (size_t i = 0; i < rect.size(); i++)
			{
				LineCollisions(rect[i], against, top, tempCollisions);
				collisions[i] = any_of(tempCollisions.cbegin(), tempCollisions.cend(), [](auto p)
				{ return p != InvalidPoint; });
			}
			RectangleCollision(rect[4], against, top, tempCollisions);
			collisions[5] = any_of(tempCollisions.cbegin(), tempCollisions.cend(), [](auto p)
			{ return p != InvalidPoint; });
			if (none_of(collisions.cbegin(), collisions.cend(), [](auto c)
			{ return c; }))
			{
				result.emplace_back(*nextPoint);
				okPath = true;
			}
			else if (!collisions[0] && !collisions[1])
			{
				result.emplace_back(rect[1].start);
				result.emplace_back(*nextPoint);
				okPath = true;
			}
			else if (!collisions[2] && !collisions[3])
			{
				result.emplace_back(rect[3].start);
				result.emplace_back(*nextPoint);
				okPath = true;
			}
			else if (collisions[4])
			{
				result.clear();
			}
			if (okPath)
			{
				currentPoint = nextPoint;
				nextPoint = &dividedSegment.back();
				nextPoint++;
			}
		}
	}

	void SegmentZoneCollision(const Segment& segment, const std::vector<LLRouter::ZoneSegment>& against, bool top,
			LLRouter::Probe* probe)
	{
		auto rect = segment.CircumscribedRectangle();
		std::fill(probe->ntzContacted.begin(), probe->ntzContacted.end(), false);
		std::fill(probe->nfzContacted.begin(), probe->nfzContacted.end(), false);
		vector<Point> collisions;
		for (const auto& rectSegment: rect)
		{
			LineCollisions(rectSegment, against, top, collisions);
			for (size_t i = 0; i < collisions.size(); i++)
			{
				if (against[i].ownerType == LLRouter::Zone::OwnerType::eNoFly && !probe->nfzContacted[against[i].zoneID])
				{
					probe->nfzContacted[against[i].zoneID] = (collisions[i] != Math::InvalidPoint);
				}
				else if(!probe->ntzContacted[against[i].zoneID])
				{
					probe->ntzContacted[against[i].zoneID] = (collisions[i] != Math::InvalidPoint);
				}
			}
		}
	}

	Point GenericPathCollision(const Segment& segment, const std::vector<LLRouter::ZoneSegment>& against, bool top)
	{
		auto rect = segment.CircumscribedRectangle();
		std::vector<Point> collisions;
		RectangleCollision(rect[4], against, top, collisions);
		for (const auto& point: collisions)
		{
			if (point != InvalidPoint)
			{
				return point;
			}
		}
		for (const auto& side: rect)
		{
			LineCollisions(side, against, top, collisions);
			for (const auto& point: collisions)
			{
				if (point != InvalidPoint)
				{
					return point;
				}
			}
		}
		return InvalidPoint;
	}

	Point PointInsideRectangle(const Point& p, const Point& topLeft, const Point& width)
	{
		return ((p.x > topLeft.x && p.x < topLeft.x + width.x) &&
		        (p.y > topLeft.y && p.y < topLeft.y + width.y)) ? p : InvalidPoint;
	}

	Point SegmentSegment(const Segment& first, const Segment& second)
	{
		constexpr static double epsilon = std::numeric_limits<double>::epsilon();
		const double A1 = first.end.y - first.start.y;
		const double B1 = first.start.x - first.end.x;
		const double C1 = A1 * first.start.x + B1 * first.start.y;

		const double A2 = second.end.y - second.start.y;
		const double B2 = second.start.x - second.end.x;
		const double C2 = A2 * second.start.x + B2 * second.start.y;

		const double denominator = A1 * B2 - A2 * B1;
		if (abs(denominator) < epsilon)
		{
			return InvalidPoint;
		}
		const double x = (B2 * C1 - B1 * C2) / denominator;
		const double y = (A1 * C2 - A2 * C1) / denominator;

		const double minX1 = std::min(first.start.x, first.end.x) - epsilon;
		const double maxX1 = std::max(first.start.x, first.end.x) + epsilon;
		const double minY1 = std::min(first.start.y, first.end.y) - epsilon;
		const double maxY1 = std::max(first.start.y, first.end.y) + epsilon;

		const double minX2 = std::min(second.start.x, second.end.x) - epsilon;
		const double maxX2 = std::max(second.start.x, second.end.x) + epsilon;
		const double minY2 = std::min(second.start.y, second.end.y) - epsilon;
		const double maxY2 = std::max(second.start.y, second.end.y) + epsilon;

		const std::array<bool, 4> check = {
				!(x < minX1 || x > maxX1), // outside x 1
				!(y < minY1 || y > maxY1), // outside y 1
				!(x < minX2 || x > maxX2), // outside x 2
				!(y < minY2 || y > maxY2)  // outside y 2
		};
		if (check[0] && check[1] && check[2] && check[3])
		{
			return { x, y };
		}
		return InvalidPoint;
	}
}