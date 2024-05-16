#pragma once

#include "../../Interfaces/IBoard.h"
#include "Zones.h"
#include "Probe.h"

namespace LLRouter
{
	struct BoardStats
	{
		double cycleTime = 0;
		double totalTime = 0;
		uint64_t panicCount = 0;
		uint64_t noCorrectionCount = 0;
		uint64_t correctionCount = 0;
		uint64_t numConfigurations = 0;
	};

	class Board : public IBoard
	{
		using TestConfiguration = std::array<Math::Point, Probe::N_PROBES>;
	public:
		static constexpr uint8_t N_PROBES = 8;

		Board() = default;

		void ReadFromFiles(const std::string& inPath) final;

		void Optimize() final;

		void SaveTo(const std::string& outPath) final;

		[[nodiscard]] const std::vector<ZonePoint>& GetZonePoints() const;

		[[nodiscard]] const std::vector<Zone>& GetNoFlyZones() const;

		[[nodiscard]] const std::vector<Zone>& GetNoTouchZones() const;

		[[nodiscard]] const std::vector<ZoneSegment>& GetZoneSegments() const;

		[[nodiscard]] double GetWidth() const;

		[[nodiscard]] double GetHeight() const;

	private:

		void ReadBoard(const std::string& inPath);

		void ReadTestPlan(const std::string& inPath);

		void MoveToConfiguration(const TestConfiguration &futureTestConfiguration, std::vector<TestConfiguration> &results);

		std::vector<TestConfiguration> CorrectConfiguration();

		bool CorrectConfigurationBest(std::vector<Board::TestConfiguration> &result, std::vector<std::vector<size_t>> &diffs, std::vector<size_t> &configurationPosition, size_t left, size_t right);

		bool CheckConfiguration(const TestConfiguration &config, std::vector<bool> &results) const;

		double m_width = 0;
		double m_height = 0;
		std::vector<Zone> m_noFlyZones;
		std::vector<Zone> m_noTouchZones;
		std::vector<ZoneSegment> m_zoneSegments;
		std::vector<ZonePoint> m_zonePoints;

		std::vector<TestConfiguration> m_testPlan;
		std::vector<TestConfiguration> m_results;

		ProbeConfiguration m_probes;
		BoardStats m_stats;
	};
}