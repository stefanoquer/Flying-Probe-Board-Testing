#include "Board.h"
#include "../../Common/Utils.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>

using namespace std;

// #define PARALLEL

namespace LLRouter
{
	void Board::ReadFromFiles(const std::string &inPath)
	{
		ReadBoard(inPath);
		ReadTestPlan(inPath);
		Math::Point points[] = {{0, 0}, {0, m_height}, {m_width, 0}, {m_width, m_height}};
		m_probes.reserve(Probe::N_PROBES);
		for (size_t i = 0; i < Probe::N_PROBES; i++)
		{
			auto p = Probe(i, Probe::N_PROBES, points[i % Probe::N_PROBES / 2], *this);
			m_probes.emplace_back(p);
		}
	}

	void Board::Optimize()
	{
		cout << "Optimizing tests" << endl;
		auto startTime = chrono::high_resolution_clock::now();
		std::vector<TestConfiguration> tempResults;
		for (size_t i = 0; i < m_testPlan.size(); i++)
		{
			TestConfiguration futureConfig = m_testPlan[i];
			MoveToConfiguration(futureConfig, tempResults);
			if (!tempResults.empty())
				std::move(tempResults.begin() + 1, tempResults.end(), back_inserter(m_results));
		}
		auto endTime = chrono::high_resolution_clock::now();
	}

	void Board::MoveToConfiguration(const TestConfiguration &futureTestConfiguration,
									std::vector<TestConfiguration> &results)
	{
		results.clear();
#ifdef PARALLEL
#pragma omp parallel for
#endif // PARALLEL
		for (int64_t i = 0; i < futureTestConfiguration.size(); i++)
		{
			auto start = chrono::high_resolution_clock::now();
			m_probes[i].futurePosition = futureTestConfiguration[i];
			m_probes[i].PathFromPointToPoint(*this);
			m_probes[i].position = m_probes[i].futurePosition;
			auto end = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
			switch (m_probes[i].currentPathSelection)
			{
			case Probe::PathSelection::eEasy:
				m_probes[i].stats.easyPathTime += static_cast<double>(duration / 1000.0);
				m_probes[i].stats.nEasyPath++;
				break;
			case Probe::PathSelection::eQuickFix:
				m_probes[i].stats.quickFixPathTime += static_cast<double>(duration / 1000.0);
				m_probes[i].stats.nQuickFixPath++;
				break;
			case Probe::PathSelection::eDijkstra:
				m_probes[i].stats.dijkstraTime += static_cast<double>(duration / 1000.0);
				m_probes[i].stats.nDijkstra++;
				break;
			default:
				break;
			}
		}
		auto corrected = CorrectConfiguration();
		if (corrected.size() < 2)
		{
			// PANIC
			m_stats.panicCount++;
			// cout << "PANIC!" << endl;
		}
		else
		{
			results.insert(results.end(), corrected.cbegin(), corrected.cend());
		}
	}

	// TODO: REWRITE
	std::vector<Board::TestConfiguration> Board::CorrectConfiguration()
	{
		vector<Board::TestConfiguration> resultTop;
		vector<Board::TestConfiguration> resultBottom;
		vector<Board::TestConfiguration> bestResultTop;
		vector<Board::TestConfiguration> bestResultBottom;
		vector<Board::TestConfiguration> bestResult;
		vector<size_t> configurationPosition(Probe::N_PROBES, 0);
		vector<vector<size_t>> diffsTop;
		vector<vector<size_t>> diffsBottom;
		diffsTop.emplace_back(configurationPosition);
		diffsBottom.emplace_back(configurationPosition);
		// Push the initial state to solution
		TestConfiguration tempResult;
		for (size_t i = 0; i < tempResult.size(); i++)
		{
			tempResult[i] = m_probes[i].pathResults[0];
		}
		resultTop.emplace_back(tempResult);
		resultBottom.emplace_back(tempResult);
		// Find best corrected configuration
		CorrectConfigurationBest(resultTop, diffsTop, configurationPosition, 0, N_PROBES / 2);
		CorrectConfigurationBest(resultBottom, diffsBottom, configurationPosition, N_PROBES / 2, N_PROBES);

		// Top
		if (resultTop.size() < 2 || resultBottom.size() < 2)
			return bestResult;
		auto currentDiff = diffsTop[0];
		bestResultTop.emplace_back(resultTop[0]);
		// Try to merge movements
		for (size_t i = 1; i < diffsTop.size(); ++i)
		{
			const auto &nextDiff = diffsTop[i];
			// if the maximum difference of position is 1:
			bool ok = true;
			for (size_t j = 0; j < currentDiff.size() / 2; ++j)
			{
				if (nextDiff[j] - currentDiff[j] > 1)
				{
					ok = false;
					break;
				}
			}
			if (!ok)
			{
				currentDiff = diffsTop[i - 1];
				bestResultTop.emplace_back(resultTop[i - 1]);
			}
		}
		bestResultTop.emplace_back(resultTop.back());

		// Bottom
		currentDiff = diffsBottom[0];
		bestResultBottom.emplace_back(resultBottom[0]);
		// Try to merge movements
		for (size_t i = 1; i < diffsBottom.size(); ++i)
		{
			const auto &nextDiff = diffsBottom[i];
			// if the maximum difference of position is 1:
			bool ok = true;
			for (size_t j = currentDiff.size() / 2; j < currentDiff.size(); ++j)
			{
				if (nextDiff[j] - currentDiff[j] > 1)
				{
					ok = false;
					break;
				}
			}
			if (!ok)
			{
				currentDiff = diffsBottom[i - 1];
				bestResultBottom.emplace_back(resultBottom[i - 1]);
			}
		}
		bestResultBottom.emplace_back(resultBottom.back());

		// Merge
		bestResult.resize(max(bestResultTop.size(), bestResultBottom.size()));
		for (size_t i = 0; i < bestResult.size(); ++i)
		{
			if (i < bestResultTop.size())
			{
				for (auto j = 0; j < bestResultTop[i].size() / 2; ++j)
				{
					bestResult[i][j] = bestResultTop[i][j];
				}
			}
			else
			{
				for (auto j = 0; j < bestResultTop[i].size() / 2; ++j)
				{
					bestResult[i][j] = bestResultTop.back()[j];
				}
			}
			if (i < bestResultBottom.size())
			{
				for (auto j = bestResultBottom[i].size() / 2; j < bestResultBottom[i].size(); ++j)
				{
					bestResult[i][j] = bestResultBottom[i][j];
				}
			}
			else
			{
				for (auto j = bestResultBottom[i].size() / 2; j < bestResultBottom[i].size(); ++j)
				{
					bestResult[i][j] = bestResultBottom.back()[j];
				}
			}
		}

		std::fill(configurationPosition.begin(), configurationPosition.end(), 0);
		return bestResult;
	}

	// TODO: REWRITE
	bool Board::CorrectConfigurationBest(std::vector<Board::TestConfiguration> &result,
										 std::vector<std::vector<size_t>> &diffs,
										 std::vector<size_t> &configurationPosition, size_t left, size_t right)
	{
		// Check if finished
		bool finished = true;
		for (size_t i = left; i < right; ++i)
		{
			if (configurationPosition[i] < m_probes[i].pathResults.size() - 1)
			{
				finished = false;
				break;
			}
		}
		// Termination condition
		// Temp result = result
		if (finished)
		{
			// Check final configuration
			if (std::vector<bool> ok; CheckConfiguration(result.back(), ok))
			{
				// worst case scenario: we have this result being valid
				return true;
			}
		}
		// Let's try to build the best solution by trying all solution
		for (size_t i = left; i < right; ++i)
		{
			if (configurationPosition[i] < m_probes[i].pathResults.size() - 1)
			{
				configurationPosition[i]++;
				// Fill probe configuration
				TestConfiguration temp;
				std::vector<size_t> tempDiff(Probe::N_PROBES, 0);
				for (size_t j = 0; j < configurationPosition.size(); ++j)
				{
					temp[j] = m_probes[j].pathResults[configurationPosition[j]];
					tempDiff[j] = configurationPosition[j];
				}
				result.emplace_back(temp);
				diffs.emplace_back(tempDiff);
				if (std::vector<bool> ok; CheckConfiguration(result.back(), ok))
				{
					if (CorrectConfigurationBest(result, diffs, configurationPosition, left, right))
						return true;
				}
				// Backtrack
				configurationPosition[i]--;
				result.pop_back();
				diffs.pop_back();
			}
		}
		return false;
	}

	bool Board::CheckConfiguration(const Board::TestConfiguration &config, std::vector<bool> &results) const
	{
		// setup
		bool ok = true;
		results.clear();
		results.reserve(config.size());
		results.emplace_back(true);
		// for each probe
		for (size_t i = 1; i < config.size(); ++i)
		{
			// If the configuration is invalid = p[i + 1].x > p[i].x AND both the probes are on TOP or on BOTTOM
			if ((config[i].x < config[i - 1].x) && m_probes[i].top == m_probes[i - 1].top)
			{
				results.emplace_back(false);
				ok = false;
			}
			else
			{
				results.emplace_back(true);
			}
		}
		return ok;
	}

	void Board::SaveTo(const std::string &outPath)
	{
		// TODO
		cout << "Saving positionings" << endl;
		ofstream fout(outPath + "/route.txt");
		for (size_t i = 0; i < m_results.size(); i++)
		{
			fout << i;
			for (const auto &point : m_results[i])
			{
				fout << " " << point.x << "," << point.y;
			}
			fout << endl;
		}
		fout.close();
	}

	void Board::ReadBoard(const string &inPath)
	{
		string line;
		std::vector<string> tokens;
		double xMax = 0;
		double yMax = 0;
		ifstream fin(inPath + "/board.txt");
		if (!fin)
		{
			throw runtime_error("Folder " + inPath + " is not a board directory");
		}
		cout << "Reading board" << endl;
		while (getline(fin, line))
		{
			splitLine(line, ' ', tokens);
			if (tokens[0] == ".width")
			{
				m_width = stod(tokens[1]);
			}
			else if (tokens[0] == ".height")
			{
				m_height = stod(tokens[1]);
			}
			else if (tokens[0] == ".point")
			{
				xMax = stod(tokens[3]);
				yMax = stod(tokens[4]);
			}
		}
		if (m_width == 0)
		{
			m_width = xMax;
		}
		if (m_height == 0)
		{
			m_height = yMax;
		}
		// TODO
	}

	void Board::ReadTestPlan(const string &inPath)
	{
		string line;
		std::vector<string> tokens;
		ifstream fin(inPath + "/path.csv");
		if (!fin)
		{
			throw runtime_error("Folder " + inPath + " does not contain path.csv");
		}
		cout << "Reading tests" << endl;
		while (getline(fin, line))
		{
			splitLine(line, ';', tokens);
			TestConfiguration configuration;
			for (size_t i = 1; i < tokens.size(); i++)
			{
				string xStr = tokens[i].substr(0, tokens[i].find_first_of(','));
				string yStr = tokens[i].substr(tokens[i].find_first_of(',') + 1);
				configuration[i - 1] = Math::Point{.x = stod(xStr), .y = stod(yStr)};
			}
			m_testPlan.emplace_back(configuration);
		}
	}

	const std::vector<ZonePoint> &Board::GetZonePoints() const
	{
		return m_zonePoints;
	}

	const std::vector<Zone> &Board::GetNoFlyZones() const
	{
		return m_noFlyZones;
	}

	const std::vector<Zone> &Board::GetNoTouchZones() const
	{
		return m_noTouchZones;
	}

	const std::vector<ZoneSegment> &Board::GetZoneSegments() const
	{
		return m_zoneSegments;
	}

	double Board::GetWidth() const
	{
		return m_width;
	}

	double Board::GetHeight() const
	{
		return m_height;
	}
} // namespace LLRouter
