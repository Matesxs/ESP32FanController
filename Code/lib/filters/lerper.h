#pragma once

#include <Arduino.h>

template<class T>
class Lerper
{
public:
  explicit Lerper(const T state = 0, const T target = 0) :
    m_state(state),
    m_start_state(0),
    m_target_state(target),
    m_duration(0),
    m_progress(0) {}

  void set(const T state, const T target, const uint32_t duration)
  {
    m_state = state;
    set(target, duration);
  }

  void set(const T target, const uint32_t duration)
  {
    m_start_state = m_state;
    m_target_state = target;
    m_duration = duration;
    m_progress = 0;
    m_prev = millis();
  }

  T getTarget() const
  {
    return m_target_state;
  }

  T update()
  {
    const uint32_t currTime = millis();
    T state = update(currTime - m_prev);
    m_prev = currTime;
    return state;
  }

  T update(const uint32_t ticks)
  {
    if (m_progress >= m_duration)
    {
      m_state = m_target_state;
      return m_target_state;
    }
    
    m_progress += ticks;
    m_progress = min(m_duration, m_progress);

    m_state = m_start_state + static_cast<T>((static_cast<double>(m_progress) / m_duration) * (static_cast<double>(m_target_state) - static_cast<double>(m_start_state)));

    if (m_progress >= m_duration)
      m_state = m_target_state;

    return m_state;
  }

  bool finished() 
  {
    return m_state == m_target_state;
  }

private:
  uint32_t m_prev;
  T m_state;
  T m_start_state;
  T m_target_state;
  uint32_t m_duration;
  uint32_t m_progress;
};