#pragma once

#include "../Interfaces/IBoard.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Joiner
{
	using TPIndex = size_t;
	using NetIndex = size_t;
	using TestIndex = size_t;
	constexpr NetIndex InvalidNetIndex = SIZE_MAX;
	constexpr TPIndex InvalidTPIndex = SIZE_MAX;
	constexpr TestIndex InvalidTestIndex = SIZE_MAX;

	// STATS
	struct Stats
	{
		float secDuration = 0.0f;
		float percReduction = 0.0f;
		uint64_t numTestsStart = 0;
		uint64_t numTestsReduced = 0;

		[[nodiscard]] std::string ToString() const;
	};

	struct TestPoint
	{
		uint64_t id = InvalidTPIndex;
		Math::Point position = Math::Point{.x = 0, .y = 0};
		std::string name;
		bool top = true;
		bool validPositioning = true;
		NetIndex netIndex = InvalidNetIndex;
	};

	struct Net
	{

		uint64_t id = InvalidNetIndex;
		std::string name;
		std::vector<TPIndex> pointIndices;

		[[nodiscard]] bool OnlyTop(const std::vector<TestPoint> &points) const
		{
			for (const auto pointIdx : pointIndices)
			{
				if (!points[pointIdx].top && points[pointIdx].validPositioning)
				{
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool OnlyBottom(const std::vector<TestPoint> &points) const
		{
			for (const auto pointIdx : pointIndices)
			{
				if (points[pointIdx].top && points[pointIdx].validPositioning)
				{
					return false;
				}
			}
			return true;
		}
	};

	struct Test
	{
		bool operator==(const Test &other) const
		{
			return id == other.id;
		}
		uint64_t id = InvalidTestIndex;
		std::string name;
		bool executed = false;
		std::vector<TPIndex> testPointIndices;
		std::vector<NetIndex> netIndices;

		[[nodiscard]] std::string ToCSVString(const std::vector<TestPoint> &vPoints,
											  const std::vector<Net> &vNets) const;

		[[nodiscard]] bool Contains(const Test &other, bool useNets);

		[[nodiscard]] static Test JoinTests(std::vector<Test> tests, const TestIndex testIndex1,
											const TestIndex testIndex2);

		bool forcePoints = false;
		bool joined = false;
		std::vector<TestIndex> joinedTests;
	};

	class Board
	{
	public:
		static constexpr uint8_t N_PROBES = 8;

		Board() = default;

		void ReadFromFiles(const std::string &inPath);

		void Optimize();

		void SaveTo(const std::string &outPath);

	private:
		void ReadPoints(const std::string &inFile);

		void ReadTests(const std::string &inFile);

		void OptimizeInner();

		bool OkPointsPositions(const std::vector<TPIndex> &points) const;

		bool OkNetPositions(const std::vector<NetIndex> &nets) const;

		std::vector<Test> MergeSmallerTests();

		std::vector<uint64_t> GetSizeOffsets(bool reverse);

		std::vector<TestPoint> m_testPoints;
		std::unordered_map<std::string, TPIndex> m_testPointsIndicesByName;
		std::vector<Net> m_nets;
		std::unordered_map<std::string, NetIndex> m_netsByName;
		std::vector<Test> m_tests;
		std::vector<Test> m_optimizedTests;
		Stats m_stats;
	};
} // namespace Joiner
