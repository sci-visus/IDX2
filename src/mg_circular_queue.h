#pragma once

#include "mg_common.h"

namespace mg {

mg_TI(t, N)
struct circular_queue {
  static_assert((N & (N - 1)) == 0);
  stack_array<t, N> Buffer;
  i16 Start = 0;
  i16 End = 0; // exclusive
  t& operator[](i16 Idx) {
    mg_Assert(Idx < Size(*this));
    return Buffer[(Start + Idx) & (N - 1)];
  }
};

mg_TI(t, N) i16
Size(const circular_queue<t, N>& Queue) {
  if (Queue.Start <= Queue.End)
    return Queue.End - Queue.Start;
  else
    return N - (Queue.Start - Queue.End);
}

mg_TI(t, N) bool
IsFull(const circular_queue<t, N>& Queue) {
  return Size(Queue) == N - 1;
}

mg_TI(t, N) void
PushBack(circular_queue<t, N>* Queue, const t& Val) {
  if (IsFull(*Queue))
    mg_Assert(false);
  Queue->Buffer[Queue->End++] = Val;
  Queue->End &= N - 1;
}

mg_TI(t, N) void
PopFront(circular_queue<t, N>* Queue, i16 Count = 1) {
  mg_Assert(Count <= Size(*Queue));
  Queue->Start += Count;
  Queue->Start &= N - 1;
}

mg_TI(t, N) void
PopBack(circular_queue<t, N>* Queue, i16 Count = 1) {
  mg_Assert(Count <= Size(*Queue));
  Queue->End -= Count;
  Queue->End &= N - 1;
}

mg_TI(t, N) void
Clear(circular_queue<t, N>* Queue) {
  Queue->Start = Queue->End = 0;
}

} // namespace mg
