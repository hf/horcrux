#pragma once

#include <cstddef>
#include <cassert>

using namespace std;

namespace hx {

#define BLOCK_DEFAULT_DATA_TYPE char* const

template <typename DATA = BLOCK_DEFAULT_DATA_TYPE>
class Block {
private:
  size_t width;
  size_t elements;
  DATA data;

public:
  Block(DATA d, size_t w, size_t e) :
    data(d),
    width(w),
    elements(e)
  {
    assert (elements > 0);
    assert (width > 0);
    assert (data != NULL);
  }

  Block(DATA d, size_t e) :
    data(d),
    width(1),
    elements(e)
  {
    assert (elements > 0);
    assert (data != NULL);
  }

  Block(const Block<DATA>& o) :
    data(o.data),
    width(o.width),
    elements(o.elements)
  {
    // No-op.
  }

  ~Block() {
    // No-op.
  }

  inline size_t Width() const {
    return width;
  }

  inline size_t Elements() const {
    return elements;
  }

  inline size_t Size() const {
    return elements * width;
  }

  inline DATA Data() const {
    return data;
  }

  inline Block<DATA> Range(size_t f, size_t t) const {
    assert (t >= f);
    assert ((t - f) <= Elements());

    return Block<DATA>(Get(f), width, t - f);
  }

  inline DATA Get(size_t i) const {
    return (*this)(i);
  }

  inline DATA operator()(size_t i) const {
    assert (i < Elements());

    return Data() + i * width;
  }

  inline Block<DATA> Sub(size_t i) const {
    return Sub(i, 1);
  }

  inline DATA SubGet(size_t i, size_t subwidth, size_t j) const {
    // TODO: Refactor this without creating aditional Block<> objects
    return Sub(i, subwidth).Get(j);
  }

  inline Block<DATA> Sub(size_t i, size_t subwidth) const {
    assert (subwidth > 0);
    assert (i < Elements());

    return Block<DATA>((*this)(i), subwidth, width / subwidth);
  }

  inline DATA First() const {
    return (*this)(0);
  }

  inline DATA Last() const {
    return (*this)(Elements() - 1);
  }

  inline Block<DATA> Recast(size_t width) const {
    assert (width > 0);

    return Block<DATA>(Data(), width, Size() / width);
  }

  inline void Exchange(size_t, size_t);
  inline void Replace(const Block<DATA>&);
};

template<>
void Block<>::Exchange(size_t i, size_t j) {
  assert (i < Elements());
  assert (j < Elements());

  if (i == j) {
    return;
  }

  BLOCK_DEFAULT_DATA_TYPE idata = Get(i);
  BLOCK_DEFAULT_DATA_TYPE jdata = Get(j);

  for (size_t k = 0; k < Width(); k++) {
    char t = idata[k];
    idata[k] = jdata[k];
    jdata[k] = t;
  }
}

template<>
void Block<>::Replace(const Block<>& other) {
  assert (other.Size() <= Size());

  for (size_t i = 0; i < other.Size(); i++) {
    *(Data() + i) = *(other.Data() + i);
  }
}

} // hx
