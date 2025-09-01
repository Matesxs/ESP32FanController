#pragma once

#include <cstdint>

template<typename T, size_t size>
class MovingAverage
{
public:
	MovingAverage()
	{
		memset(m_data, 0, sizeof(T) * size);
	}

	explicit MovingAverage(const T val)
	{
		Fill(val);
	}

	T Add(const T val)
	{
		m_sum += val;

		if (IsFull())
		{
			m_sum -= m_data[m_rear];
			m_front = (m_front + 1) % size;
		}
		else
			m_used++;

		m_data[m_rear] = val;
		m_rear = (m_rear + 1) % size;

		m_result = m_sum / m_used;

		m_updated = true;

		return m_result;
	}

	void Fill(const T val)
	{
		for (size_t i = 0; i < size; i++)
			Add(val);
	}

	T Get() const { return m_result; }
	T Sum() const { return m_sum; }
	T Stddev()
	{
		if (m_updated)
		{
			T sumQuadDev = static_cast<T>(0);
			for (size_t i = 0; i < m_used; i++)
				sumQuadDev += pow(m_data[(m_front + i) % size] - m_result, 2.0);
			m_stddev = sqrt(sumQuadDev / m_used);
			m_updated = false;
		}
		return m_stddev;
	}

	void Reset()
	{
		m_result = static_cast<T>(0);
		m_sum = static_cast<T>(0);
		m_stddev = static_cast<T>(0);

		m_used = 0;
		m_front = 0;
		m_rear = 0;
		m_updated = false;
	}

public:
	T operator()(const T val)
	{
		return Add(val);
	}

	explicit operator T() const { return m_result; }

private:
	[[nodiscard]] bool IsFull() const { return m_used >= size; }

private:
	T m_data[size];
	T m_result{ 0 };
	T m_sum{ 0 };
	T m_stddev{ 0 };
	bool m_updated = false;

	size_t m_used = 0;
	size_t m_front = 0;
	size_t m_rear = 0;
};
