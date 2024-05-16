#pragma once

#include "Heap.h"

template<class T>
class PriorityQueue
{
public:
	explicit PriorityQueue() = default;
	explicit PriorityQueue(size_t size) { m_heap.resize(size); }
	inline void reserve(size_t size) { m_heap.reserve(size); }
	inline void resize(size_t size) { m_heap.resize(size); }
	inline size_t size() { return m_heap.size(); }
	inline size_t capacity() { return m_heap.capacity(); }
	inline bool empty() { return m_heap.size() == 0; }
	inline void sortAll() { return m_heap.HEAPmake(); }
	inline void setComparator(std::function<int64_t(T a, T b)> comparator) { m_heap.setComparator(comparator); }
	inline void clear() { m_heap.clear(); }
	inline T* data() { return m_heap.data(); }
	inline std::vector<T>& asVector() { return m_heap.asVector(); }
	void push(T element);
	T pop();
	void update(size_t position, T value);
	size_t find(T element);
private:
	Heap<T> m_heap;
};

template<class T>
inline void PriorityQueue<T>::push(T element)
{
	m_heap.push(element);
}

template<class T>
inline T PriorityQueue<T>::pop()
{
	return m_heap.pop();
}

template<class T>
inline void PriorityQueue<T>::update(size_t position, T value)
{
	m_heap.change(position, value);
}

template<class T>
inline size_t PriorityQueue<T>::find(T element)
{
	for (size_t i = 0; i < size(); ++i)
	{
		if (m_heap.data()[i] == element)
		{
			return i;
		}
	}
	return INT64_MAX;
}
