#pragma once

#include "lerper.h"

template<class T>
class Fader
{
private:
  bool active = false;
  double m_progres = 0.0;
  T &from;
  T &to;
  Lerper<double> m_lerper;

public:  
  Fader(T &fromState, T &toState)
    :from(fromState)
    ,to(toState)
  {
    m_lerper.setReference(&m_progres);
  }

  bool start(uint32_t duration)
  {
    active = true;
    m_lerper.set(1.0, duration, 0.0);
    return true;
  }

  bool isActive() { return active; }
  
  bool fade()
  {
    if(!active) 
      return false;

    if(m_lerper.finished())
    {
      from.setValues(to);
      active = false;
      return false;
    }

    m_lerper.update();

    from.fade(to, m_progres);
    return true;
  }
};