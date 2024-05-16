#pragma once

#include "../Common/Math.h"
#include <memory>
#include <string>
#include <vector>

struct ITestPoint;
struct INet;
struct ITest;

struct ITestPoint
{
	uint64_t id = 0;
	Math::Point position = Math::Point{.x = 0, .y = 0};
	std::string name;
	bool top = true;
	bool validPositioning = true;
	std::shared_ptr<INet> net = nullptr;
};

struct INet
{
	uint64_t id = 0;
	std::string name;
	std::vector<std::shared_ptr<ITestPoint>> points;

	[[nodiscard]] bool OnlyTop() const
	{
		for (const auto &point : points)
		{
			if (!point->top && point->validPositioning)
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] bool OnlyBottom() const
	{
		for (const auto &point : points)
		{
			if (point->top && point->validPositioning)
			{
				return false;
			}
		}
		return true;
	}
};

struct ITest
{
	bool operator==(const ITest &other) const
	{
		return id == other.id;
	}
	uint64_t id = 0;
	std::string name;
	bool executed = false;
	std::vector<std::shared_ptr<ITestPoint>> testPoints;
	std::vector<std::shared_ptr<INet>> nets;
};

class IBoard
{
public:
	IBoard() = default;

	virtual void ReadFromFiles(const std::string &inPath) = 0;

	virtual void Optimize() = 0;

	virtual void SaveTo(const std::string &outPath) = 0;

protected:
};
