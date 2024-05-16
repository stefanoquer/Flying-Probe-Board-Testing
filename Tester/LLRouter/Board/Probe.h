#pragma once

#include "../../Common/Math.h"
#include "../DataStructures/PriorityQueue.h"
#include "Zones.h"
#include <array>
#include <cstdint>
#include <vector>

namespace LLRouter
{
	class Board;

	struct ProbeStats
	{
		double easyPathTime = 0;
		double quickFixPathTime = 0;
		double dijkstraTime = 0;
		uint64_t nEasyPath = 0;
		uint64_t nQuickFixPath = 0;
		uint64_t nDijkstra = 0;
		uint64_t nPathDetections = 0;
	};

	class Probe
	{
	public:
		enum class PathSelection
		{
			eEasy,
			eQuickFix,
			eDijkstra,
			eInvalid
		};
		constexpr static uint64_t N_PROBES = 8;

	public:
		Probe(size_t id, size_t nProbes, Math::Point position, const Board &board);

		void PathFromPointToPoint(const Board &board);

		void PassAroundNoZone(const ZonePoint &start, ZonePoint &end, const Board &board,
							  bool shouldIgnoreContactedNoFly = false);

		void CalculateXMinMax(size_t start = 0);

		size_t ID;
		bool top;
		Math::Point position;
		Math::Point futurePosition{};
		std::vector<Math::Point> pathResults;
		PathSelection currentPathSelection{};

		// CORRECTION
		std::pair<uint64_t, double> xMin;
		std::pair<uint64_t, double> xMax;

		ProbeStats stats;
		std::vector<bool> nfzContacted;
		std::vector<bool> ntzContacted;

	private:
		std::vector<int64_t> m_reverseSolutionDijkstra;
		PriorityQueue<int64_t> m_queue;
		std::vector<ZonePoint> m_zonePoints;
		static constexpr bool m_smart = true;
		std::vector<ZonePoint *> m_pointsToContact;
	};

	using ProbeConfiguration = std::vector<Probe>;
} // namespace LLRouter
