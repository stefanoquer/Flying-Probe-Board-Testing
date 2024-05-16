#pragma once

#include "../../Common/Math.h"
#include <iostream>
#include <unordered_map>
#include <vector>

namespace LLRouter
{

	struct Zone
	{
		enum class OwnerType
		{
			eNoFly,
			eNoTouch
		};

		uint64_t ID;
		std::vector<size_t> segmentIndices;
	};

	struct ZoneSegment
	{
		uint64_t ID;
		uint64_t zoneID;
		Math::Segment segment;
		Zone::OwnerType ownerType;
		bool isTop;
	};

	struct ZonePoint
	{
		enum class PointClassification
		{
			starting,
			end,
			uninitialized
		};
		uint64_t ID{};
		uint64_t zoneID{};
		Math::Point point{};
		Zone::OwnerType ownerType;
		bool isTop{};
		PointClassification prevPoint = PointClassification::uninitialized;
		uint64_t prevPointIndex = 0;
		double minimumDistance = std::numeric_limits<double>::max();
		std::unordered_map<size_t, std::vector<Math::Point>> adjacentPoints;
	};
} // namespace LLRouter
