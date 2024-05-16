#pragma once

#include <vector>
#include <functional>
#include <algorithm>

template<class T>
class Heap
{
public:
	explicit Heap(size_t size = 1);
	~Heap();
	void reserve(size_t size);
	void resize(size_t size);
	inline void setComparator(std::function<int64_t(T a, T b)>& comparator) { m_comparator = comparator; }
	[[nodiscard]] inline size_t size() const noexcept { return m_data.size(); }
	[[nodiscard]] inline size_t capacity() const noexcept { return m_data.capacity(); }
	void push(T element);
	void HEAPmake();
	inline void clear() { resize(0); }
	void HEAPify(const size_t index) noexcept;
	inline T* data() { return m_data.data(); }
	T pop();
	void change(int64_t position, T value);

private:
	[[nodiscard]] size_t left(size_t index) const { return (index * 2 + 1); }
	[[nodiscard]] size_t right(size_t index) const { return (index * 2 + 2); }
	[[nodiscard]] size_t parent(size_t index) const { return ((index - 1) / 2); }

	void swap(size_t index1, size_t index2);

	std::vector<T> m_data;
	std::function<int64_t(T a, T b)> m_comparator;
	bool isHeap = false;
};

template<class T>
inline Heap<T>::Heap(size_t size)
{
	m_data = std::vector<T>(size);
}

template<class T>
inline Heap<T>::~Heap()
{
}

template<class T>
inline void Heap<T>::reserve(size_t size)
{
	m_data.reserve(size);
}

template<class T>
inline void Heap<T>::resize(size_t size)
{
	m_data.resize(size);
	isHeap = false;
}

template<class T>
inline void Heap<T>::push(T element)
{
	m_data.emplace_back(element);
	isHeap = false;
}

template<class T>
inline void Heap<T>::HEAPmake() 
{ 
	//return std::make_heap(m_data.begin(), m_data.end(), [&](T a, T b) {return m_comparator(a, b) > 0; });
	for (int64_t i = size() / 2 - 1; i >= 0; --i)
	{
		HEAPify(i);
	}
	isHeap = true;
}

template<class T>
inline void Heap<T>::HEAPify(const size_t index) noexcept
{
	auto l = left(index);
	auto r = right(index);
	auto largest = index;
	if ((l < size()) && (m_comparator(m_data[l], m_data[index]) > 0))
	{
		largest = l;
	}
	if ((r < size()) && (m_comparator(m_data[r], m_data[largest]) > 0))
	{
		largest = r;
	}
	if (largest != index)
	{
		swap(index, largest);
		HEAPify(largest);
	}
}

template<class T>
inline T Heap<T>::pop()
{
	T result = m_data[0];
	std::pop_heap(&m_data[0], &m_data[m_data.size()]);
	m_data.pop_back();
	return result;
}

// assume value has already changed
template<class T>
inline void Heap<T>::change(int64_t position, T value)
{
	auto old = m_data[position];
	m_data[position] = value;
	if(value < old)
	{
		HEAPify(position);
	}
	else
	{
		while(position > 1 && m_data[parent(position)] < m_data[position])
		{
			swap(position, parent(position));
			position = parent(position);
		}
	}
}

template<class T>
inline void Heap<T>::swap(size_t index1, size_t index2)
{
	T temp = std::move(m_data[index1]);
	m_data[index1] = std::move(m_data[index2]);
	m_data[index2] = std::move(temp);
}
