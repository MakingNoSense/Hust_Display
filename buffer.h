#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

class CircularBuffer {
  public:
    CircularBuffer(int size);
    void add(float data);
    float get_mean();

  private:
    int _size;
    int _head;
    float *_buffer;
    bool _full;
};

#endif