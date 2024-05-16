#include "Board.h"

#include "../Common/Utils.h"
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

using namespace std;
namespace HLRouter
{

	void Board::ReadFromFiles(const std::string &inPath)
	{
		cout << "Reading points..." << endl;
		ReadPoints(inPath);
		cout << "Reading tests..." << endl;
		ReadTests(inPath);
		m_stats.numPositionings = m_tests.size();
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
				TestPoint point;
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
				if (point.position.x > m_width)
				{
					m_width = point.position.x;
				}
				if (point.position.y > m_height)
				{
					m_height = point.position.y;
				}
			}
			else if (tokens[0] == ".net")
			{
				Net net;
				const auto netSize = stoull(tokens[2]);
				const auto netName = tokens[1];
				net.id = netID++;
				net.name = netName;
				net.pointIndices.reserve(netSize);
				for (size_t i = 3; i < tokens.size(); i++)
				{
					const auto pointIndex = m_testPointsIndicesByName.at(tokens[i]);
					net.pointIndices.emplace_back(pointIndex);
					m_testPoints[pointIndex].netIndex = m_nets.size();
				}
				m_nets.emplace_back(net);
				m_netsIndicesByName[netName] = m_nets.size() - 1;
			}
		}
		fin.close();
	}

	void Board::ReadTests(const std::string &inPath)
	{
		ifstream fin(inPath + "/joined.csv");
		if (!fin)
		{
			throw runtime_error(inPath + " is not a board folder");
		}
		string line;
		vector<string> tokens;
		while (getline(fin, line))
		{
			splitLine(line, ';', tokens);
			Test test;
			test.id = stoull(tokens[0]);
			test.forcePoints = static_cast<bool>(stoull(tokens[1]));
			for (size_t i = 2; i < N_PROBES; i++)
			{
				if (tokens[i].empty())
					break;
				if (test.forcePoints)
				{
					test.testPointIndices.emplace_back(m_testPointsIndicesByName.at(tokens[i]));
				}
				else
				{
					test.netIndices.emplace_back(m_netsIndicesByName.at(tokens[i]));
				}
			}
			/*for (size_t i = 2; i < N_PROBES + 2; i++)
			{
				if (!tokens[i].empty())
				{
					test->nets.emplace_back(m_netsByName.at(tokens[i]));
				}
			}
			if (tokens.size() > N_PROBES + 2)
			{
				for (size_t i = N_PROBES + 2; i < N_PROBES + 2 + N_PROBES; i++)
				{
					if (!tokens[i].empty())
					{
						test->testPoints.emplace_back(m_testPoints[stoull(tokens[i])]);
					}
				}
			}*/
			m_tests.emplace_back(test);
		}
		/*for (const auto& test: m_tests)
		{
			if (test->forcePoints)
			{
				auto end = std::min(test->testPoints.begin() + N_PROBES / 2, test->testPoints.end());
				sort(test->testPoints.begin(), end, [](const auto& a, const auto& b)
				{
					if (a->top == b->top)
					{
						return a->position.x < b->position.x;
					}
					if (a->top)
					{
						return true;
					}
					return false;
				});
			}
		}*/
		fin.close();
	}

	void Board::Optimize()
	{
		cout << "Finding a path..." << endl;
		m_optimizedTests.emplace_back(Configuration{.probes =
														{
															// top
															Math::Point{0, 0},				// TL
															Math::Point{0, m_height},		// BL
															Math::Point{m_width, 0},		// TR
															Math::Point{m_width, m_height}, // BR
																							// bottom
															Math::Point{0, 0},				// TL
															Math::Point{0, m_height},		// BL
															Math::Point{m_width, 0},		// TR
															Math::Point{m_width, m_height}	// BR
														},
													.testIndex = InvalidTestIndex});
		const auto start = chrono::high_resolution_clock::now();
		OptimizeInner();
		const auto stop = chrono::high_resolution_clock::now();
		m_stats.duration =
			static_cast<float>(chrono::duration_cast<chrono::milliseconds>(stop - start).count()) / 1000.0f;
		m_stats.totDistance = 0;
		for (size_t i = 1; i < m_optimizedTests.size() - 1; i++)
		{
			const auto distance = Configuration::Distance(m_optimizedTests[i], m_optimizedTests[i + 1]);
			m_stats.totDistance += distance;
			if (distance < m_stats.minDistance)
			{
				m_stats.minDistance = distance;
			}
			if (distance > m_stats.maxDistance)
			{
				m_stats.maxDistance = distance;
			}
		}
	}

	void Board::OptimizeInner()
	{
		size_t i = 0;
		while (!all_of(m_tests.cbegin(), m_tests.cend(), [](const auto &a) { return a.executed; }))
		{
			auto conf = FindNextTest();
			m_tests[conf.testIndex].executed = true;
			m_optimizedTests.emplace_back(conf);
			if (i % 100 == 0)
			{
				cout << "Found path between " << i << " tests out of " << m_tests.size() << endl;
			}
			i++;
		}
	}

	void Board::SaveTo(const std::string &outPath)
	{
		ofstream fout(outPath + "/path.csv");
		for (const auto &conf : m_optimizedTests)
		{
			if (conf.testIndex == InvalidTestIndex)
				fout << "begin";
			else
				fout << m_tests[conf.testIndex].id;
			for (const auto &point : conf.probes)
			{
				fout << ";" << point.x << "," << point.y;
			}
			fout << endl;
		}
		fout.close();
		fout = ofstream(outPath + "/tracePoints.csv");
		size_t i = 0;
		for (const auto &conf : m_optimizedTests)
		{
			/*if (i > 0 && any_of(conf.used.cbegin(), conf.used.cend(), [](const auto& u)
			{ return !u; }))
			{
				//cout << "WARNING! IMPOSSIBLE ROUTING! Test " << i << " not performed!" << endl;
			}*/
			i++;
			if (conf.testIndex == InvalidTestIndex)
				fout << "begin";
			else
				fout << m_tests[conf.testIndex].id;
			std::string pointName;
			for (size_t j = 0; j < conf.testPointIndices.size(); j++)
			{
				const auto point = conf.testPointIndices[j];
				if (j == conf.testPointIndices.size() / 2)
				{
					pointName.clear();
				}
				if (j < conf.testPointIndices.size() / 2 && pointName.empty())
				{
					for (auto k = static_cast<int64_t>(j); k >= 0; k--)
					{
						if (pointName.empty() && conf.testPointIndices[k] != InvalidTPIndex)
						{
							pointName = m_testPoints[conf.testPointIndices[k]].name;
						}
						else if (!pointName.empty())
							break;
					}
					for (auto k = static_cast<int64_t>(j); k < conf.testPointIndices.size() / 2; k++)
					{
						if (pointName.empty() && conf.testPointIndices[k] != InvalidTPIndex)
						{
							pointName = m_testPoints[conf.testPointIndices[k]].name;
						}
						else if (!pointName.empty())
							break;
					}
				}
				else
				{
					for (auto k = static_cast<int64_t>(j); k >= conf.testPointIndices.size() / 2; k--)
					{
						if (pointName.empty() && conf.testPointIndices[k] != InvalidTPIndex)
						{
							pointName = m_testPoints[conf.testPointIndices[k]].name;
						}
						else if (!pointName.empty())
							break;
					}
					for (auto k = static_cast<int64_t>(j); k < conf.testPointIndices.size(); k++)
					{
						if (pointName.empty() && conf.testPointIndices[k] != InvalidTPIndex)
						{
							pointName = m_testPoints[conf.testPointIndices[k]].name;
						}
						else if (!pointName.empty())
							break;
					}
				}
				if (conf.testPointIndices[j] != InvalidTPIndex)
				{
					pointName = m_testPoints[conf.testPointIndices[j]].name;
				}
				fout << ";" << pointName;
			}
			fout << endl;
		}
		fout.close();
		fout = ofstream(outPath + "/statsHLRouter.txt");
		fout << m_stats.ToString();
		fout.close();
	}

	Board::Configuration Board::FindNextTest()
	{
		Configuration bestConfiguration = m_optimizedTests.back();
		bestConfiguration.testIndex = InvalidTestIndex;
		atomic<double> bestDistance = DBL_MAX;
#pragma omp parallel for
		for (int64_t testIndex = 0; testIndex < m_tests.size(); testIndex++)
		{
			const auto &test = m_tests[testIndex];
			if (test.executed)
				continue;
			auto [configuration, distance] =
				Configuration::Distance(m_testPoints, m_nets, m_tests, m_optimizedTests.back(), testIndex);
#pragma omp critical
			if (distance < bestDistance.load())
			{
				bestConfiguration = configuration;
				bestConfiguration.testIndex = testIndex;
				bestDistance = distance;
			}
		}
		return bestConfiguration;
	}

	double Board::Configuration::Distance(const Board::Configuration &a, const Board::Configuration &b)
	{
		double maxDistance = 0;
		for (size_t i = 0; i < Board::N_PROBES; i++)
		{
			double currentDistance = Math::Point::Distance(a.probes[i], b.probes[i]);
			if (currentDistance > maxDistance)
			{
				maxDistance = currentDistance;
			}
		}
		if (maxDistance == 0)
		{
			return 0;
		}
		return maxDistance;
	}

	std::pair<Board::Configuration, double> Board::Configuration::Distance(const std::vector<TestPoint> &points,
																		   const std::vector<Net> &nets,
																		   const std::vector<Test> &tests,
																		   const Board::Configuration &config,
																		   const TestIndex testIndex)
	{
		Board::Configuration result = config;
		result.used.fill(false);
		unordered_set<NetIndex> completedNets;
		// For skipping the force points in top/bottom manner
		size_t countOnlyTop = 0;
		size_t countOnlyBottom = 0;
		if (tests[testIndex].forcePoints)
		{

			size_t indexTop = 0;
			size_t indexBottom = config.probes.size() / 2;
			for (const auto pointIdx : tests[testIndex].testPointIndices)
			{
				// APPROXIMATION! Could be improved in a second passage, should be relatively easy to try all the
				// configurations
				if (points[pointIdx].top)
				{
					result.testPointIndices[indexTop++] = pointIdx;
				}
				else
				{
					result.testPointIndices[indexBottom++] = pointIdx;
				}
			}
		}
		else
		{
			// First only top and only bottom
			for (const auto netIdx : tests[testIndex].netIndices)
			{
				double bestDistance = DBL_MAX;
				TPIndex bestPoint = InvalidTPIndex;
				size_t bestProbe = 0;
				bool found = false;
				if (nets[netIdx].OnlyTop(points))
				{
					countOnlyTop++;
					for (size_t i = 0; i < config.probes.size() / 2; i++)
					{
						if (result.used[i])
							continue;
						auto [point, distance] = Net::DistancePointNetSide(nets, points, result.probes[i],
																		   (i < result.probes.size() / 2), netIdx);
						if (distance < bestDistance)
						{
							bestDistance = distance;
							bestPoint = point;
							bestProbe = i;
							found = true;
						}
					}
				}
				else if (nets[netIdx].OnlyBottom(points))
				{
					countOnlyBottom++;
					for (size_t i = config.probes.size() / 2; i < config.probes.size(); i++)
					{
						if (result.used[i])
							continue;
						auto [point, distance] = Net::DistancePointNetSide(nets, points, result.probes[i],
																		   (i < result.probes.size() / 2), netIdx);
						if (distance < bestDistance)
						{
							bestDistance = distance;
							bestPoint = point;
							bestProbe = i;
							found = true;
						}
					}
				}
				if (found)
				{
					result.testPointIndices[bestProbe] = bestPoint;
					result.used[bestProbe] = true;
				}
			}
			for (const auto netIdx : tests[testIndex].netIndices)
			{
				if (nets[netIdx].OnlyBottom(points) || nets[netIdx].OnlyTop(points))
					continue;
				double bestDistance = DBL_MAX;
				TPIndex bestPoint = InvalidTPIndex;
				size_t bestProbe = 0;
				for (size_t i = 0; i < config.probes.size(); i++)
				{
					if (result.used[i])
						continue;
					auto [point, distance] = Net::DistancePointNetSide(nets, points, result.probes[i],
																	   (i < result.probes.size() / 2), netIdx);
					if (distance < bestDistance)
					{
						bestDistance = distance;
						bestPoint = point;
						bestProbe = i;
					}
				}
				result.testPointIndices[bestProbe] = bestPoint;
				result.used[bestProbe] = true;
			}
		}
		sort(result.testPointIndices.begin(), result.testPointIndices.begin() + result.probes.size() / 2,
			 [&points](const auto a, const auto b) {
				 return (a != InvalidTPIndex && b != InvalidTPIndex) &&
						((points[a].position.x < points[b].position.x) ||
						 (points[a].position.x == points[b].position.x && points[a].position.y < points[b].position.y));
			 });
		sort(result.testPointIndices.begin() + result.testPointIndices.size() / 2, result.testPointIndices.end(),
			 [&points](const auto a, const auto b) {
				 return (a != InvalidTPIndex && b != InvalidTPIndex) &&
						((points[a].position.x < points[b].position.x) ||
						 (points[a].position.x == points[b].position.x && points[a].position.y < points[b].position.y));
			 });

		for (size_t i = 0; i < result.probes.size(); i++)
		{
			if (result.testPointIndices[i] != InvalidTPIndex)
			{
				result.probes[i] = points[result.testPointIndices[i]].position;
			}
		}
		double maxDistance = Distance(config, result);
		return make_pair(result, maxDistance);
	}

	std::string Stats::ToString() const
	{
		ostringstream out;
		out << "Minimum distance: " << minDistance << endl;
		out << "Maximum distance: " << maxDistance << endl;
		out << "Total distance: " << totDistance << endl;
		out << "Total number of positionings: " << numPositionings << endl;
		out << "Average distance between positionings: " << totDistance / (double)numPositionings << endl;
		out << "---------------------------------------" << endl;
		out << "Computation duration: " << duration << " s" << endl;
		return out.str();
	}

	std::pair<std::pair<TPIndex, TPIndex>, double> Net::DistanceNets(const std::vector<Net> &nets,
																	 const std::vector<TestPoint> &testPoints,
																	 const NetIndex n1, const NetIndex n2)
	{
		TPIndex bestP1 = InvalidTPIndex;
		TPIndex bestP2 = InvalidTPIndex;

		double maxDistance = DBL_MAX;
		for (const auto tp1 : nets[n1].pointIndices)
		{
			for (const auto tp2 : nets[n2].pointIndices)
			{
				double distance = Math::Point::GDistance(testPoints[tp1].position, testPoints[tp2].position);
				if (distance < maxDistance)
				{
					bestP1 = tp1;
					bestP2 = tp2;
					maxDistance = distance;
				}
			}
		}
		return make_pair(make_pair(bestP1, bestP2), maxDistance);
	}

	std::pair<TPIndex, double> Net::DistancePointNet(const std::vector<Net> &nets,
													 const std::vector<TestPoint> &testPoints, const TPIndex p,
													 const NetIndex n)
	{
		TPIndex bestP = InvalidTPIndex;
		double bestDistance = DBL_MAX;
		for (const auto p2 : nets[n].pointIndices)
		{
			double distance = TestPoint::Distance(testPoints[p], testPoints[p2]);
			if (distance < bestDistance)
			{
				bestP = p2;
				bestDistance = distance;
			}
		}
		return make_pair(bestP, bestDistance);
	}

	std::pair<TPIndex, double> Net::DistancePointNetSide(const std::vector<Net> &nets,
														 const std::vector<TestPoint> &testPoints, const Math::Point p,
														 bool top, const NetIndex n)
	{
		TPIndex bestP;
		double bestDistance = DBL_MAX;
		for (const auto p2 : nets[n].pointIndices)
		{
			if (!testPoints[p2].validPositioning)
				continue;
			double distance = TestPoint::Distance(p, top, testPoints[p2]);
			if (distance < bestDistance)
			{
				bestP = p2;
				bestDistance = distance;
			}
		}
		return make_pair(bestP, bestDistance);
	}

	double TestPoint::Distance(const TestPoint &tp1, const TestPoint &tp2)
	{
		return Distance(tp1.position, tp1.top, tp2);
	}

	double TestPoint::Distance(const Math::Point p1, bool topP1, const TestPoint &tp2)
	{
		if (topP1 != tp2.top)
			return DBL_MAX;
		return Math::Point::Distance(p1, tp2.position);
	}
	bool Net::OnlyTop(const std::vector<TestPoint> &points) const
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
	bool Net::OnlyBottom(const std::vector<TestPoint> &points) const
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
} // namespace HLRouter
