#include "Board.h"

#include "../Common/Utils.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

using namespace std;
namespace Joiner
{
	void Board::ReadFromFiles(const std::string &inPath)
	{
		cout << "Reading points..." << endl;
		ReadPoints(inPath);
		cout << "Reading tests..." << endl;
		ReadTests(inPath);
		m_stats.numTestsStart = m_tests.size();
	}

	void Board::ReadPoints(const std::string &inPath)
	{
		ifstream fin(inPath + "/board.txt");
		if (!fin)
		{
			throw runtime_error(inPath + " is not a board folder");
		}
		string line;
		vector<string> tokens;
		uint64_t pointID = 0;
		uint64_t netID = 0;
		while (getline(fin, line))
		{
			if (line.ends_with('\r'))
				line.pop_back();
			splitLine(line, ' ', tokens);
			if (tokens[0] == ".point")
			{
				auto point = TestPoint();
				point.id = pointID++;
				point.name = tokens[1];
				point.top = tokens[2] == "T";
				point.validPositioning = tokens[2] == "T" || tokens[2] == "B";
				if (tokens[3].find(',') != string::npos)
				{
					tokens[3][tokens[3].find(',')] = '.';
				}
				if (tokens[4].find(',') != string::npos)
				{
					tokens[4][tokens[4].find(',')] = '.';
				}
				point.position = Math::Point{.x = stod(tokens[3]), .y = stod(tokens[4])};
				m_testPoints.emplace_back(point);
				m_testPointsIndicesByName[point.name] = m_testPoints.size() - 1;
			}
			else if (tokens[0] == ".net")
			{
				auto net = Net();
				const auto netSize = stoull(tokens[2]);
				const auto netName = tokens[1];
				net.id = netID++;
				net.name = netName;
				net.pointIndices.reserve(netSize);
				for (size_t i = 3; i < tokens.size(); i++)
				{
					const auto pointIdx = m_testPointsIndicesByName.at(tokens[i]);
					net.pointIndices.emplace_back(pointIdx);
					m_testPoints[pointIdx].netIndex = m_nets.size();
				}
				m_nets.emplace_back(net);
				m_netsByName[netName] = m_nets.size() - 1;
			}
		}
		fin.close();
		for (size_t i = 0; i < m_testPoints.size(); i++)
		{
			auto &point = m_testPoints[i];
			if (m_nets[point.netIndex].id == InvalidNetIndex)
			{
				std::cout << "WARNING! Net missing for point " << point.name << ". Creating one." << std::endl;
				auto net = Net();
				net.id = netID++;
				net.name = "net_fake_" + std::to_string(netID);
				net.pointIndices.emplace_back(i);
				m_nets.emplace_back(net);
				m_netsByName[net.name] = m_nets.size() - 1;
				point.netIndex = m_nets.size() - 1;
			}
		}
	}

	void Board::ReadTests(const std::string &inPath)
	{
		// size_t netID = m_nets.size();
		ifstream fin(inPath + "/test.txt");
		if (!fin)
		{
			throw runtime_error(inPath + " does not contain any tests");
		}
		string line;
		vector<string> tokens;
		uint64_t id = 0;
		while (getline(fin, line))
		{
			if (line.ends_with('\r'))
				line.pop_back();
			splitLine(line, ' ', tokens);
			if (tokens[0] == ".test")
			{
				auto test = Test();
				const auto testSize = stoull(tokens[2]);
				test.id = id++;
				test.name = tokens[1];
				set<NetIndex> nets;
				set<TPIndex> points;
				for (size_t i = 3; i < tokens.size(); i++)
				{
					const auto pointIdx = m_testPointsIndicesByName[tokens[i]];
					points.emplace(pointIdx);
				}
				test.testPointIndices = vector(points.cbegin(), points.cend());
				for (size_t i = 0; i < test.testPointIndices.size(); i++)
				{
					bool found = false;
					int64_t foundAt = -1;
					for (size_t j = i + 1; j < test.testPointIndices.size(); j++)
					{
						if (m_testPoints[test.testPointIndices[i]].netIndex ==
							m_testPoints[test.testPointIndices[j]].netIndex)
						{
							found = true;
							foundAt = static_cast<int64_t>(j);
							break;
						}
					}
					if (found)
					{
						test.forcePoints = true;
					}
					else
					{
						nets.emplace(m_testPoints[test.testPointIndices[i]].netIndex);
					}
				}
				test.netIndices = vector(nets.cbegin(), nets.cend());
				m_tests.emplace_back(test);
			}
		}
		fin.close();
		for (const auto &test : m_tests)
		{
			if (test.netIndices.size() > N_PROBES)
			{
				cout << "WARNING: test " << test.name << " will not be performed. It requires contacting "
					 << test.netIndices.size() << " nets." << endl;
			}
		}
	}

	void Board::Optimize()
	{
		cout << "Optimizing tests..." << endl;
		const auto start = chrono::high_resolution_clock::now();
		OptimizeInner();
		size_t testSize;
		const auto end = chrono::high_resolution_clock::now();
		m_stats.secDuration =
			static_cast<float>(chrono::duration_cast<chrono::milliseconds>(end - start).count()) / 1000.0f;
		m_stats.numTestsReduced = m_optimizedTests.size();
		m_stats.percReduction =
			static_cast<float>(m_stats.numTestsReduced) / static_cast<float>(m_stats.numTestsStart) * 100.0f;
		for (const auto &t : m_tests)
		{
			if (!t.executed)
			{
				cout << "NOT EXECUTED! Test: id: " << t.id << ", name: " << t.name << endl;
			}
		}
	}

	void Board::SaveTo(const string &outPath)
	{
		cout << "Writing tests..." << endl;
		ofstream fout(outPath + "/joined.csv");
		for (const auto &t : m_optimizedTests)
		{
			fout << t.ToCSVString(m_testPoints, m_nets);
		}
		fout.close();
		// trace for each joined test
		fout = ofstream(outPath + "/testTrace.txt");
		for (const auto &t : m_optimizedTests)
		{
			fout << t.name;
			for (const auto &joined : t.joinedTests)
			{
				fout << " " << m_tests[joined].name;
			}
			fout << endl;
		}
		fout.close();
		// Print stats
		fout = ofstream(outPath + "/statsJoin.txt");
		fout << m_stats.ToString() << endl;
		fout.close();
	}

	void Board::OptimizeInner()
	{
		// FORCED POINTS
		auto sizeOffsets = GetSizeOffsets(true);
		for (int64_t i = 0; i < m_tests.size(); i++)
		{
			if (m_tests[i].executed)
				continue;
			if (!m_tests[i].forcePoints)
				break;
			int32_t currentOffset = static_cast<int32_t>(sizeOffsets.size() - 1);
			auto &currentSolution = m_tests[i];
			currentSolution.executed = true;
			vector<TPIndex> currentPoints(currentSolution.testPointIndices.cbegin(),
										  currentSolution.testPointIndices.cend());
			for (int32_t count = 0; currentOffset > 0; count++)
			{
				while (currentOffset > 0 && sizeOffsets[currentOffset] + count >= sizeOffsets[0])
				{
					currentOffset--;
					while (currentOffset > 0 && (sizeOffsets[currentOffset] == sizeOffsets[currentOffset - 1] ||
												 sizeOffsets[currentOffset] == sizeOffsets[0]))
						currentOffset--;
					count = 0;
				}
				if (currentOffset < sizeOffsets.size() - 2 &&
					sizeOffsets[currentOffset] + count >= sizeOffsets[currentOffset + 1])
					continue;
				const size_t nextTestIndex = sizeOffsets[currentOffset] + count;
				auto &nextTest = m_tests[nextTestIndex];
				if (nextTest.executed)
					continue;
				if (!nextTest.forcePoints)
					break;
				if (currentOffset == 0)
					break;
				currentPoints.resize(currentSolution.testPointIndices.size());
				for (const auto &point : nextTest.testPointIndices)
				{
					if (find(currentPoints.cbegin(), currentPoints.cend(), point) == currentPoints.cend())
					{
						currentPoints.emplace_back(point);
					}
				}
				if (OkPointsPositions(currentPoints))
				{
					currentSolution.joinedTests.emplace_back(nextTestIndex);
					nextTest.executed = true;
					currentSolution.testPointIndices = vector(currentPoints.cbegin(), currentPoints.cend());
				}
			}
			m_optimizedTests.emplace_back(currentSolution);
		}
		// FREE POINTS
		sizeOffsets = GetSizeOffsets(false);
		for (int64_t i = 0; i < m_tests.size(); i++)
		{
			if (m_tests[i].executed)
				continue;
			if (m_tests[i].forcePoints)
				break;
			int32_t currentOffset = (int32_t)sizeOffsets.size() - 1;
			auto &currentSolution = m_tests[i];
			currentSolution.executed = true;
			vector<NetIndex> currentNets(currentSolution.netIndices.cbegin(), currentSolution.netIndices.cend());
			for (int32_t count = 0; currentOffset > 0; count++)
			{
				while (currentOffset > 0 && sizeOffsets[currentOffset] + count >= sizeOffsets[0])
				{
					currentOffset--;
					while (currentOffset > 0 && (sizeOffsets[currentOffset] == sizeOffsets[currentOffset - 1] ||
												 sizeOffsets[currentOffset] == sizeOffsets[0]))
						currentOffset--;
					count = 0;
				}
				if (currentOffset < sizeOffsets.size() - 2 &&
					sizeOffsets[currentOffset] + count >= sizeOffsets[currentOffset + 1])
					continue;
				const auto nextTestIndex = sizeOffsets[currentOffset] + count;
				auto &nextTest = m_tests[nextTestIndex];
				if (nextTest.executed)
					continue;
				if (nextTest.forcePoints)
					break;
				if (currentOffset == 0)
					break;
				currentNets.resize(currentSolution.netIndices.size());
				for (const auto &net : nextTest.netIndices)
				{
					if (find(currentNets.cbegin(), currentNets.cend(), net) == currentNets.cend())
					{
						currentNets.emplace_back(net);
					}
				}
				if (OkNetPositions(currentNets))
				{
					currentSolution.joinedTests.emplace_back(nextTestIndex);
					nextTest.executed = true;
					currentSolution.netIndices = vector(currentNets.cbegin(), currentNets.cend());
				}
			}
			m_optimizedTests.emplace_back(currentSolution);
		}
	}

	std::vector<uint64_t> Board::GetSizeOffsets(bool reverse)
	{
		vector<uint64_t> result;
		uint64_t currentSize = 8;
		// Get iterator to free points
		if (reverse)
		{
			uint64_t beginFreePoints;
			// Sort for force points in the beginning
			sort(m_tests.begin(), m_tests.end(),
				 [](const auto &first, const auto &second) { return first.forcePoints > second.forcePoints; });
			for (beginFreePoints = 0; beginFreePoints < m_tests.size(); beginFreePoints++)
				if (!m_tests[beginFreePoints].forcePoints)
					break;
			// Sort for size (join the largest groups first)
			stable_sort(m_tests.begin(), m_tests.begin() + static_cast<int64_t>(beginFreePoints),
						[](const auto &first, const auto &second) {
							return first.testPointIndices.size() > second.testPointIndices.size();
						});
			result.resize(currentSize, beginFreePoints);
			for (int64_t i = (int64_t)beginFreePoints - 1; i > 0; i--)
			{
				result[m_tests[i].testPointIndices.size()] = i;
			}
			result[0] = beginFreePoints;
		}
		else
		{
			uint64_t endFreePoints;
			// sort for free points in the beginning
			sort(m_tests.begin(), m_tests.end(),
				 [](const auto &first, const auto &second) { return first.forcePoints < second.forcePoints; });
			for (endFreePoints = m_tests.size() - 1; endFreePoints > 0; endFreePoints--)
				if (!m_tests[endFreePoints].forcePoints)
					break;
			endFreePoints++;
			// Sort for size (join the largest groups first)
			stable_sort(m_tests.begin(), m_tests.begin() + static_cast<int64_t>(endFreePoints),
						[](const auto &first, const auto &second) {
							return first.testPointIndices.size() > second.testPointIndices.size();
						});
			result.resize(currentSize, endFreePoints);
			for (int64_t i = (int64_t)endFreePoints - 1; i > 0; i--)
			{
				result[m_tests[i].testPointIndices.size()] = i;
			}
			result[0] = endFreePoints;
		}
		return result;
	}

	bool Board::OkPointsPositions(const std::vector<TPIndex> &points) const
	{
		uint64_t topPoints = 0;
		uint64_t bottomPoints = 0;
		for (const auto &p : points)
		{
			m_testPoints[p].top ? topPoints++ : bottomPoints++;
		}
		return (topPoints <= N_PROBES / 2) && (bottomPoints <= N_PROBES / 2);
	}

	bool Board::OkNetPositions(const std::vector<NetIndex> &nets) const
	{
		uint64_t onlyUp = 0;
		uint64_t onlyDown = 0;
		uint64_t netUpDown = 0;
		for (const auto &net : nets)
		{
			if (m_nets[net].OnlyTop(m_testPoints))
			{
				onlyUp++;
			}
			else if (m_nets[net].OnlyBottom(m_testPoints))
			{
				onlyDown++;
			}
			else
			{
				netUpDown++;
			}
		}
		return (onlyUp <= Board::N_PROBES / 2) && (onlyDown <= Board::N_PROBES / 2) &&
			   (onlyUp + onlyDown + netUpDown <= Board::N_PROBES);
	}

	std::vector<Test> Board::MergeSmallerTests()
	{
		vector<Test> result;
		for (size_t i = 0; i < m_optimizedTests.size(); i++)
		{
			auto &biggerTest = m_optimizedTests[i];
			if (biggerTest.joined)
				continue;
			for (int j = 0; j < m_optimizedTests.size(); j++)
			{
				auto &smallerTest = m_optimizedTests[j];
				if (smallerTest.joined)
					continue;
				if (biggerTest.forcePoints != smallerTest.forcePoints)
					continue;
				if (biggerTest == smallerTest)
					continue;
				if (biggerTest.Contains(smallerTest, !biggerTest.forcePoints))
				{
					biggerTest = Test::JoinTests(m_tests, i, j);
				}
			}
			result.emplace_back(biggerTest);
		}
		return result;
	}

	std::string Test::ToCSVString(const std::vector<TestPoint> &vPoints, const std::vector<Net> &vNets) const
	{
		ostringstream sout;
		// sout << id << ";";
		sout << id << ";" << (forcePoints ? 1 : 0) << ";";
		for (size_t i = 0; i < Board::N_PROBES; i++)
		{
			if (forcePoints)
			{
				if (i < testPointIndices.size())
				{
					sout << vPoints[testPointIndices[i]].name;
				}
			}
			else
			{
				if (i < netIndices.size())
				{
					sout << vNets[netIndices[i]].name;
				}
			}
			if (i < Board::N_PROBES - 1)
			{
				sout << ";";
			}
		}
		sout << endl;
		return sout.str();
	}

	bool Test::Contains(const Test &other, bool useNets)
	{
		bool result;
		if (useNets)
		{
			result = all_of(other.netIndices.cbegin(), other.netIndices.cend(), [&](const auto &net) {
				return std::find(netIndices.cbegin(), netIndices.cend(), net) != netIndices.cend();
			});
		}
		else
		{
			result = all_of(other.testPointIndices.cbegin(), other.testPointIndices.cend(), [&](const auto &point) {
				return std::find(testPointIndices.cbegin(), testPointIndices.cend(), point) != testPointIndices.cend();
			});
		}
		return result;
	}

	Test Test::JoinTests(std::vector<Test> tests, const TestIndex testIndex1, const TestIndex testIndex2)
	{
		auto &test1 = tests[testIndex1];
		auto &test2 = tests[testIndex2];
		auto result = Test();
		result.id = test1.id;
		result.name = test1.name;
		result.forcePoints = test1.forcePoints | test2.forcePoints;
		vector<NetIndex> totalNets(test1.netIndices.cbegin(), test1.netIndices.cend());
		vector<TPIndex> totalTestPoints(test1.testPointIndices.cbegin(), test1.testPointIndices.cend());
		vector<TestIndex> totalTests(test1.joinedTests.cbegin(), test1.joinedTests.cend());
		for (const auto netIdx : test2.netIndices)
		{
			if (find(totalNets.cbegin(), totalNets.cend(), netIdx) == totalNets.cend())
			{
				totalNets.emplace_back(netIdx);
			}
		}
		for (const auto pointIdx : test2.testPointIndices)
		{
			if (find(totalTestPoints.cbegin(), totalTestPoints.cend(), pointIdx) == totalTestPoints.cend())
			{
				totalTestPoints.emplace_back(pointIdx);
			}
		}
		for (const auto testIdx : test2.joinedTests)
		{
			if (find(totalTests.cbegin(), totalTests.cend(), testIdx) == totalTests.cend())
			{
				totalTests.emplace_back(testIdx);
			}
		}
		totalTests.emplace_back(testIndex2);
		result.testPointIndices = std::move(totalTestPoints);
		result.netIndices = std::move(totalNets);
		result.joinedTests = std::move(totalTests);
		result.executed = true;
		test1.joined = true;
		test2.joined = true;
		return result;
	}

	// STATS
	std::string Stats::ToString() const
	{
		ostringstream out;
		out << "Process duration: " << secDuration << " s" << endl;
		out << "Number of tests before join: " << numTestsStart << endl;
		out << "Number of tests after join: " << numTestsReduced << endl;
		out << "Reduction percentage: " << percReduction << "%" << endl;
		return out.str();
	}
} // namespace Joiner
