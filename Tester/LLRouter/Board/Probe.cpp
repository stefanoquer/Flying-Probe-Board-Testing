#include "Probe.h"

#include "Board.h"
#include "../Collisions/Collision.h"

using namespace std;

namespace LLRouter
{
	Probe::Probe(const size_t id, const size_t nProbes, Math::Point position, const Board& board)
			: ID(id), top(ID < nProbes / 2), position(position)
	{
		m_zonePoints = vector<ZonePoint>(board.GetZonePoints());
		m_queue.reserve(board.GetZonePoints().size() + 1);
		nfzContacted.resize(board.GetNoFlyZones().size(), false);
		ntzContacted.resize(board.GetNoTouchZones().size(), false);
	}

	void Probe::PathFromPointToPoint(const Board& board)
	{
		currentPathSelection = PathSelection::eInvalid;
		pathResults.clear();
		vector<Math::Point> division;
		Math::Collision::SegmentDivision(Math::Segment{
				.start = position,
				.end = futurePosition
		}, board.GetZoneSegments(), top, division);
		pathResults.emplace_back(position);
		if constexpr (m_smart)
		{
			if (!division.empty() && division.size() < 5)
			{
				if (division.size() == 1)
				{
					currentPathSelection = PathSelection::eEasy;
				}
				else
				{
					currentPathSelection = PathSelection::eQuickFix;
				}
				for (const auto& p: division)
				{
					pathResults.emplace_back(p);
				}
			}
		}
		if(currentPathSelection != PathSelection::eInvalid) return; 
		Math::Collision::SegmentZoneCollision(Math::Segment{
						.start = position,
						.end = futurePosition },
				board.GetZoneSegments(), top, this);
		ZonePoint startPoint{
				.ID = static_cast<uint64_t>(-1),
				.zoneID = 0,
				.point = position,
				.ownerType = Zone::OwnerType::eNoFly,
				.isTop = top
		};
		ZonePoint endPoint{
				.ID = board.GetZonePoints().size(),
				.zoneID = 0,
				.point = futurePosition,
				.ownerType = Zone::OwnerType::eNoFly,
				.isTop = top
		};
		if (!division.empty())
		{
			startPoint.adjacentPoints[endPoint.ID] = std::move(division);
		}
		currentPathSelection = PathSelection::eDijkstra;
		m_pointsToContact.clear();
		for(size_t i = 0; i < m_zonePoints.size(); i++)
		{
			vector<bool> &zoneConsidered = (m_zonePoints[i].ownerType == Zone::OwnerType::eNoFly) ? nfzContacted : ntzContacted;
			if(zoneConsidered[m_zonePoints[i].zoneID])
			{
				m_pointsToContact.emplace_back(&m_zonePoints[i]);
			}
		}
		for(size_t i = 0; i < m_pointsToContact.size(); i++)
		{
			if(Math::Collision::GenericPathCollision(Math::Segment{position, m_pointsToContact[i]->point}, board.GetZoneSegments(), top) != Math::InvalidPoint)
			{
				startPoint.adjacentPoints[m_pointsToContact[i]->ID] = vector<Math::Point>{m_pointsToContact[i]->point};
			}
			if (Math::Collision::GenericPathCollision(Math::Segment{m_pointsToContact[i]->point, futurePosition}, board.GetZoneSegments(), top) != Math::InvalidPoint)
			{
				m_pointsToContact[i]->adjacentPoints[endPoint.ID] = vector<Math::Point>{endPoint.point};
			}
		}
		startPoint.prevPoint = ZonePoint::PointClassification::starting;
		endPoint.prevPoint = ZonePoint::PointClassification::end;
		PassAroundNoZone(startPoint, endPoint, board);
		// reset
		#pragma omp parallel for
			for (int i = 0; i < m_zonePoints.size(); ++i)
			{
				m_zonePoints[i].adjacentPoints.erase(endPoint.ID);
			}
	}

#define GET_POINT(POINT_ID) ((POINT_ID) == end.ID) ? end : m_zonePoints[(POINT_ID)]

	void Probe::PassAroundNoZone(const ZonePoint& start, ZonePoint& end, const Board& board,
			bool shouldIgnoreContactedNoFly)
	{
		static const double STATIC_DISTANCE = board.GetWidth() + board.GetHeight();
		m_queue.clear();
		m_reverseSolutionDijkstra.clear();
		for (int64_t i = 0; i < m_zonePoints.size(); i++)
		{
			if (!shouldIgnoreContactedNoFly)
			{
				switch (m_zonePoints[i].ownerType)
				{
				case Zone::OwnerType::eNoFly:
					if (nfzContacted[m_zonePoints[i].zoneID])
					{
						m_queue.push(i);
					}
					break;
				case Zone::OwnerType::eNoTouch:
					if (ntzContacted[m_zonePoints[i].zoneID])
					{
						m_queue.push(i);
					}
					break;
				}
			}
			else
			{
				m_queue.push(i);
			}
			m_zonePoints[i].minimumDistance = std::numeric_limits<double>::max();
			m_zonePoints[i].prevPoint = ZonePoint::PointClassification::uninitialized;
		}
		m_queue.push(static_cast<int64_t>(end.ID));
		end.minimumDistance = std::numeric_limits<double>::max();
		end.prevPoint = ZonePoint::PointClassification::uninitialized;
		m_queue.setComparator([this, &end](const int64_t a, const int64_t b)
		{
			const auto& aPoint = GET_POINT(a);
			const auto& bPoint = GET_POINT(b);
			return bPoint.minimumDistance - aPoint.minimumDistance;
		});
		// First round
		for (const auto& [index, points]: start.adjacentPoints)
		{
			double distance = STATIC_DISTANCE * static_cast<double>(points.size());
			for (const auto& point: points)
			{
				distance += Math::Point::Distance(start.point, point);
			}
			auto& adjacentPoint = GET_POINT(index);
			adjacentPoint.minimumDistance = distance;
			adjacentPoint.prevPoint = ZonePoint::PointClassification::starting;
		}
		m_queue.sortAll();
		// Now the real fun starts
		while (!m_queue.empty())
		{
			auto index = m_queue.pop();
			const auto& point = GET_POINT(index);
			if (point.minimumDistance >= std::numeric_limits<double>::max()) continue;
			// Reached the end
			if (index == end.ID)
			{
				break;
			}
			for (const auto& [adjIndex, adjPoints]: point.adjacentPoints)
			{
				double distance =
						STATIC_DISTANCE * static_cast<double>(adjPoints.size()) + m_zonePoints[index].minimumDistance;
				distance += Math::Point::Distance(point.point, adjPoints.back());
				auto& adjPoint = GET_POINT(adjIndex);
				if (adjPoint.minimumDistance > distance)
				{
					auto adjPointIdx = m_queue.find(static_cast<int64_t>(adjPoint.ID));
					adjPoint.minimumDistance = distance;
					adjPoint.prevPointIndex = index;
					if (adjPointIdx < numeric_limits<int64_t>::max())
					{
						m_queue.update(adjPointIdx, static_cast<int64_t>(adjIndex));
					}
				}
			}
		}

		// Found solution, now let's go backwards!
		m_reverseSolutionDijkstra.emplace_back(end.prevPointIndex);
		for (auto& point = GET_POINT(m_reverseSolutionDijkstra.back());
		     point.prevPoint != ZonePoint::PointClassification::starting;
		     point = GET_POINT(m_reverseSolutionDijkstra.back()))
		{
			m_reverseSolutionDijkstra.emplace_back(point.prevPointIndex);
		}
		m_reverseSolutionDijkstra.pop_back();

		// Build forward solution
		for (auto index : m_reverseSolutionDijkstra)
		{
			auto p = GET_POINT(index);
			pathResults.emplace_back(p.point);
		}
		if(m_reverseSolutionDijkstra.empty())
		{
			pathResults.emplace_back(end.point);
		}
	}

#undef GET_POINT

	void Probe::CalculateXMinMax(size_t start)
	{
		xMax.first = xMin.first = -1;
		xMax.second = 0;
		xMin.second = std::numeric_limits<double>::max();
		for (auto i = start; i < pathResults.size(); i++)
		{
			if (pathResults[i].x < xMin.second)
			{
				xMin.second = pathResults[i].x;
				xMin.first = i;
			}
			if (pathResults[i].x > xMin.second)
			{
				xMin.second = pathResults[i].x;
				xMin.first = i;
			}
		}
	}

}