#pragma once

#include <Arduino.h>

template<class T>
class Lerper
{
public:
  Lerper() :
    m_reference(NULL),
    m_start_state(0),
    m_target_state(0),
    m_duration(0),
    m_progress(0) {}

  bool setReference(T* reference)
  {
    if (finished())
    {
      m_reference = reference;
      return true;
    }

    return false;
  }

  void set(T target, uint32_t duration, T ref_overwrite)
  {
    set(target, duration);
    (*m_reference) = ref_overwrite;
  }

  void set(T target, uint32_t duration)
  {
    if (m_reference == NULL) return;

    m_start_state = *m_reference;
    m_target_state = target;
    m_duration = duration;
    m_progress = 0;
    m_prev = millis();
  }

  void update()
  {
    uint32_t currTime = millis();
    update(currTime - m_prev);
    m_prev = currTime;
  }

  void update(uint32_t ticks)
  {
    if (m_reference == NULL) return;
    if (m_progress == m_duration) return;
    m_progress += ticks;
    m_progress = min(m_duration, m_progress);
    (*m_reference) = static_cast<T>(m_progress * (m_target_state - m_start_state) / m_duration + m_start_state);
  }

  bool finished() 
  {
    if (m_reference == NULL) return true;
    return (*m_reference) == m_target_state; 
  }

private:
  uint32_t m_prev;
  T* m_reference;
  T m_start_state;
  T m_target_state;
  uint32_t m_duration;
  uint32_t m_progress;
};