#pragma once

#include "../Interfaces/IBoard.h"
#include <array>
#include <cfloat>
#include <map>
#include <vector>

namespace HLRouter
{
	struct Stats
	{
		double totDistance = 0;
		double maxDistance = 0;
		double minDistance = DBL_MAX;
		uint64_t numPositionings = 0;
		float duration = 0;

		[[nodiscard]] std::string ToString() const;
	};

	using TPIndex = size_t;
	using NetIndex = size_t;
	using TestIndex = size_t;
	constexpr NetIndex InvalidNetIndex = SIZE_MAX;
	constexpr TPIndex InvalidTPIndex = SIZE_MAX;
	constexpr TestIndex InvalidTestIndex = SIZE_MAX;

	struct TestPoint
	{
		uint64_t id = 0;
		Math::Point position = Math::Point{.x = 0, .y = 0};
		std::string name;
		bool top = true;
		bool validPositioning = true;
		NetIndex netIndex = InvalidNetIndex;
		bool fixed = false;
		static double Distance(const TestPoint &tp1, const TestPoint &tp2);

		static double Distance(const Math::Point p1, bool topP1, const TestPoint &tp2);
	};

	struct Net
	{
		uint64_t id = 0;
		std::string name;
		std::vector<TPIndex> pointIndices;
		static std::pair<std::pair<TPIndex, TPIndex>, double> DistanceNets(const std::vector<Net> &nets,
																		   const std::vector<TestPoint> &testPoints,
																		   const NetIndex n1, const NetIndex n2);

		static std::pair<TPIndex, double> DistancePointNet(const std::vector<Net> &nets,
														   const std::vector<TestPoint> &testPoints, const TPIndex p,
														   const NetIndex n);

		static std::pair<TPIndex, double> DistancePointNetSide(const std::vector<Net> &nets,
															   const std::vector<TestPoint> &testPoints,
															   const Math::Point p, bool top, const NetIndex n);

		[[nodiscard]] bool OnlyTop(const std::vector<TestPoint> &points) const;

		[[nodiscard]] bool OnlyBottom(const std::vector<TestPoint> &points) const;
	};

	struct Test
	{
		bool operator==(const Test &other) const
		{
			return id == other.id;
		}
		uint64_t id = 0;
		std::string name;
		bool executed = false;
		bool forcePoints = false;
		std::vector<TPIndex> testPointIndices;
		std::vector<NetIndex> netIndices;
	};

	class Board : public IBoard
	{
	public:
		static constexpr uint32_t N_PROBES = 8;

		Board() = default;

		void ReadFromFiles(const std::string &inPath) override;

		void Optimize() override;

		void SaveTo(const std::string &outPath) override;

	private:
		struct Configuration
		{
			std::array<Math::Point, N_PROBES> probes;
			std::array<TPIndex, N_PROBES> testPointIndices;
			std::array<bool, N_PROBES> used{false};
			TestIndex testIndex;

			static double Distance(const Configuration &a, const Configuration &b);

			static std::pair<Configuration, double> Distance(const std::vector<TestPoint> &points,
															 const std::vector<Net> &nets,
															 const std::vector<Test> &tests, const Configuration &a,
															 const TestIndex b);
		};

		void ReadPoints(const std::string &inPath);

		void ReadTests(const std::string &inPath);

		void OptimizeInner();

		Configuration FindNextTest();

		std::vector<TestPoint> m_testPoints;
		std::map<std::string, TPIndex> m_testPointsIndicesByName;
		std::vector<Net> m_nets;
		std::map<std::string, NetIndex> m_netsIndicesByName;
		std::vector<Test> m_tests;
		std::vector<Configuration> m_optimizedTests;

		double m_width = 0;
		double m_height = 0;
		Stats m_stats;
	};
} // namespace HLRouter
