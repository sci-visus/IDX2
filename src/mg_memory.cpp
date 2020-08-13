#include <string.h>
#include "mg_assert.h"
#include "mg_memory.h"

// TODO: some of these functions can be made inline

namespace idx2 {

i64
MemCopy(const buffer& Src, buffer* Dst) {
  mg_Assert(Dst->Data, "Copy to null");
  mg_Assert(Src.Data || Src.Bytes == 0, "Copy from null");
  mg_Assert(Dst->Bytes >= Src.Bytes, "Copy to a smaller buffer");
  memcpy(Dst->Data, Src.Data, size_t(Src.Bytes));
  return Src.Bytes;
}

i64
MemCopy(const buffer& Src, buffer* Dst, u64 Bytes) {
  mg_Assert(Dst->Data, "Copy to null");
  mg_Assert(Src.Data || Src.Bytes == 0, "Copy from null");
  mg_Assert(Dst->Bytes >= Src.Bytes, "Copy to a smaller buffer");
  memcpy(Dst->Data, Src.Data, size_t(Bytes));
  return Bytes;
}

buffer
operator+(const buffer& Buf, i64 Bytes) {
  return buffer{Buf.Data + Bytes, Buf.Bytes - Bytes};
}

void
ZeroBuf(buffer* Buf) {
  mg_Assert(Buf->Data);
  memset(Buf->Data, 0, size_t(Buf->Bytes));
}

mg_T(t) void
ZeroBufT(buffer_t<t>* Buf) {
  mg_Assert(Buf->Data);
  memset(Buf->Data, 0, Buf->Size * sizeof(t));
}

void
AllocBuf(buffer* Buf, i64 Bytes, allocator* Alloc) {
  Alloc->Alloc(Buf, Bytes);
}

void
CallocBuf(buffer* Buf, i64 Bytes, allocator* Alloc) {
  mg_Assert(!Buf->Data || Buf->Bytes == 0, "Buffer not freed before allocating new memory");
  if (Alloc == &Mallocator()) {
    Buf->Data = (byte*)calloc(size_t(Bytes), 1);
  }  else {
    AllocBuf(Buf, Bytes, Alloc);
    ZeroBuf(Buf);
  }
  mg_AbortIf(!(Buf->Data), "Out of memory");
  Buf->Bytes = Bytes;
  Buf->Alloc = Alloc;
}

void
DeallocBuf(buffer* Buf) {
  mg_Assert(Buf->Alloc);
  Buf->Alloc->Dealloc(Buf);
}

bool
mallocator::Alloc(buffer* Buf, i64 Bytes) {
  mg_Assert(!Buf->Data || Buf->Bytes == 0, "Buffer not freed before allocating new memory");
  Buf->Data = (byte*)malloc(size_t(Bytes));
  mg_AbortIf(!(Buf->Data), "Out of memory");
  Buf->Bytes = Bytes;
  Buf->Alloc = this;
  return true;
}

void
mallocator::Dealloc(buffer* Buf) {
  free(Buf->Data);
  Buf->Data = nullptr;
  Buf->Bytes = 0;
  Buf->Alloc = nullptr;
}

void
mallocator::DeallocAll() { /* empty */ }

linear_allocator::
linear_allocator() = default;

linear_allocator::
linear_allocator(const buffer& Buf) : Block(Buf) {}

bool linear_allocator::
Alloc(buffer* Buf, i64 Bytes) {
  if (CurrentBytes + Bytes <= Block.Bytes) {
    Buf->Data = Block.Data + CurrentBytes;
    Buf->Bytes = Bytes;
    Buf->Alloc = this;
    CurrentBytes += Bytes;
    return true;
  }
  return false;
}

void linear_allocator::
Dealloc(buffer* Buf) {
  if (Buf->Data + Buf->Bytes == Block.Data + CurrentBytes) {
    Buf->Data = nullptr;
    Buf->Bytes = 0;
    Buf->Alloc = nullptr;
    CurrentBytes -= Buf->Bytes;
  }
}

void linear_allocator::
DeallocAll() {
  CurrentBytes = 0;
}

bool linear_allocator::
Own(const buffer& Buf) const {
  return Block.Data <= Buf.Data && Buf.Data < Block.Data + CurrentBytes;
}

free_list_allocator::
free_list_allocator() = default;

free_list_allocator::
free_list_allocator(i64 MinBytesIn, i64 MaxBytesIn, allocator* ParentIn)
  : MinBytes(MinBytesIn)
  , MaxBytes(MaxBytesIn)
  , Parent(ParentIn) {}

free_list_allocator::
free_list_allocator(i64 Bytes, allocator* ParentIn)
  : free_list_allocator(Bytes, Bytes, ParentIn) {}

bool free_list_allocator::
Alloc(buffer* Buf, i64 Bytes) {
  mg_Assert(Parent);
  if (MinBytes <= Bytes && Bytes <= MaxBytes && Head) {
    Buf->Data = (byte*)Head;
    Buf->Bytes = Bytes;
    Buf->Alloc = this;
    Head = Head->Next;
    return true;
  }
  bool Result = Parent->Alloc(Buf, MaxBytes);
  Buf->Bytes = Bytes;
  Buf->Alloc = this;
  return Result;
}

void free_list_allocator::
Dealloc(buffer* Buf) {
  mg_Assert(Parent);
  if (MinBytes <= Buf->Bytes && Buf->Bytes <= MaxBytes) {
    Buf->Bytes = 0;
    Buf->Alloc = this;
    node* P = (node*)(Buf->Data);
    P->Next = Head;
    Head = P;
  } else {
    Parent->Dealloc(Buf);
  }
}

// NOTE: the client may want to call Parent->DeallocateAll() as well
void free_list_allocator::
DeallocAll() {
  mg_Assert(Parent);
  while (Head) {
    node* Next = Head->Next;
    buffer Buf((byte*)Head, MaxBytes, Parent);
    Parent->Dealloc(&Buf);
    Head = Next;
  }
}

fallback_allocator::
fallback_allocator() = default;

fallback_allocator::
fallback_allocator(owning_allocator* PrimaryIn, allocator* SecondaryIn)
  : Primary(PrimaryIn)
  , Secondary(SecondaryIn) {}

bool fallback_allocator::
Alloc(buffer* Buf, i64 Size) {
  bool Success = Primary->Alloc(Buf, Size);
  return Success ? Success : Secondary->Alloc(Buf, Size);
}

void fallback_allocator::
Dealloc(buffer* Buf) {
  if (Primary->Own(*Buf))
    return Primary->Dealloc(Buf);
  Secondary->Dealloc(Buf);
}

void fallback_allocator::
DeallocAll() {
  Primary->DeallocAll();
  Secondary->DeallocAll();
}

void
Clone(const buffer& Src, buffer* Dst, allocator* Alloc) {
  if (Dst->Data && Dst->Bytes != Src.Bytes)
    DeallocBuf(Dst);
  if (!Dst->Data && Dst->Bytes == 0)
    Alloc->Alloc(Dst, Src.Bytes);
  MemCopy(Src, Dst);
}

} // namespace idx2

