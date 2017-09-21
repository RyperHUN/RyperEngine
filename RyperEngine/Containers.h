#pragma once
#include <vector>

template <typename T, int MAX_SIZE = 10>
class CircularVector
{
	std::vector<T> rays;
	int index = 0;
public:
	const int maxCapacity = MAX_SIZE;
	CircularVector()
	{
		rays.reserve(MAX_SIZE);
	}
	void push(T ray)
	{
		if (maxCapacity != rays.size())
			rays.push_back(ray);
		else
		{
			rays[index++] = ray;
			index = index % maxCapacity;
		}
	}
	const std::vector<T>& GetRays() const
	{
		return rays;
	}
};