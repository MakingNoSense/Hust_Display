#include "buffer.h"

CircularBuffer::CircularBuffer(int size) : _size(size), _head(0), _full(false) {
  _buffer = new float[size];
}

void CircularBuffer::add(float value) {
  _buffer[_head] = value;
  _head = (_head + 1) % _size;

  if (_head == 0) {
    _full = true;
  }
}

float CircularBuffer::get_mean() {
  int count = _full ? _size : _head;
  float sum = 0.0;
  for(int i = 0; i < count; i++) {
    sum += _buffer[i];
  }

  return sum / count;
}