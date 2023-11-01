#pragma once

#include <Arduino.h>

template<class T, unsigned int N>
class Queue 
{
private:
  int _front, _back, _count;
  T _data[N + 1];
public:
  Queue() 
  { 
    _front = 0;
    _back = 0;
    _count = 0;  
  }

  inline int count();
  inline int front();
  inline int back();
  bool push(const T &item);
  T peek();
  T pop();
  void clear();
};

template<class T, unsigned int N>
inline int Queue<T, N>::count() 
{
  return _count;
}

template<class T, unsigned int N>
inline int Queue<T, N>::front() 
{
  return _front;
}

template<class T, unsigned int N>
inline int Queue<T, N>::back() 
{
  return _back;
}

template<class T, unsigned int N>
bool Queue<T, N>::push(const T &item)
{
  if(_count < N) // Drops out when full
  {
    _data[_back++]=item;
    ++_count;
    // Check wrap around
    if (_back > N)
      _back -= (N + 1);

    return true;
  }

  return false;
}

template<class T, unsigned int N>
T Queue<T, N>::pop() 
{
  if(_count <= 0) return T(); // Returns empty
  else 
  {
    T result = _data[_front];
    _front++;
    --_count;
    // Check wrap around
    if (_front > N) 
      _front -= (N + 1);
    return result; 
  }
}

template<class T, unsigned int N>
T Queue<T, N>::peek() 
{
  if(_count <= 0) return T(); // Returns empty
  else return _data[_front];
}

template<class T, unsigned int N>
void Queue<T, N>::clear() 
{
  _front = _back;
  _count = 0;
}

