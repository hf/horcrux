#pragma once

#include <cstddef>
#include <cassert>

#include <cu/block.hpp>

using namespace std;

namespace hx {
namespace Headers {

template <typename SPLIT, typename DATA = CU_BLOCK_DEFAULT_DATA_TYPE>
class Invertor {
private:
  const SPLIT split;

  inline size_t FieldWidth() const {
    return SPLIT::FieldType::WIDTH;
  }

public:
  typedef SPLIT SplitType;
  typedef DATA DataType;

  Invertor(const SPLIT& s) :
    split(s)
  {
    assert (split.Pieces() == split.Quorum());
  }

  Invertor(const Invertor<SPLIT, DATA>& other) :
    split(other.split)
  {
    assert (split.Pieces() == split.Quorum());
  }

  ~Invertor() {
    // No-op.
  }

  inline size_t OutputSize() const {
    return split.Pieces() * split.Quorum() * FieldWidth();
  }

  cu::Block<DATA> Invert(const cu::Block<DATA>&, const cu::Block<DATA>&, bool* const) const;
};

template <typename SPLIT, typename DATA>
cu::Block<DATA> Invertor<SPLIT, DATA>::Invert(const cu::Block<DATA>& hdrs, const cu::Block<DATA>& out, bool* const inverted) const {
  assert (hdrs.Size() == OutputSize());
  assert (out.Size() == OutputSize());

  cu::Block<DATA> headers = hdrs.Recast(split.Quorum() * FieldWidth());
  cu::Block<DATA> output = out.Recast(split.Quorum() * FieldWidth());

  assert (headers.Elements() == split.Pieces());

  char left_data[headers.Size()];
  char right_data[headers.Size()];

  cu::Block<DATA> left = cu::Block<DATA>(left_data, headers.Size()).Recast(split.Quorum() * FieldWidth());
  cu::Block<DATA> right = cu::Block<DATA>(right_data, headers.Size()).Recast(split.Quorum() * FieldWidth());

  assert (left.Elements() == split.Pieces());
  assert (right.Elements() == split.Pieces());

  left.Replace(headers);

  typename SPLIT::FieldType field;

  char one[FieldWidth()];

  field.Value(1, one);

  char intermediate[FieldWidth()];

  // identity matrix
  for (size_t i = 0; i < right.Elements(); i++) {
    cu::Block<DATA> row = right.Sub(i, FieldWidth());

    assert (row.Elements() == split.Quorum());

    for (size_t j = 0; j < row.Elements(); j++) {
      field.Value(0, row(j));
    }

    field.Value(1, row(i));
  }

  // reduce
  for (size_t k = 0; k < split.Pieces(); k++) {
    size_t pivot = k;

    for (size_t i = k; i < split.Pieces(); i++) {
      cu::Block<DATA> prow = left.Sub(pivot, FieldWidth());
      cu::Block<DATA> irow = left.Sub(i, FieldWidth());

      if (field.Compare(irow(k), prow(k)) > 0) {
        pivot = i;
      }
    }

    // TODO: Check if matrix is singular, set inverted = false and return out.

    right.Exchange(pivot, k);
    left.Exchange(pivot, k);

    cu::Block<DATA> lk = left.Sub(k, FieldWidth());
    cu::Block<DATA> rk = right.Sub(k, FieldWidth());

    for (size_t i = k + 1; i < split.Pieces(); i++) {
      cu::Block<DATA> li = left.Sub(i, FieldWidth());

      // left
      for (size_t j = k + 1; j < split.Quorum(); j++) {
        field.Mul(li(k), lk(j), intermediate);
        field.Div(intermediate, lk(k), intermediate);
        field.Sub(li(j), intermediate, li(j));
      }

      cu::Block<DATA> ri = right.Sub(i, FieldWidth());

      // right
      for (size_t j = 0; j < split.Quorum(); j++) {
        field.Mul(li(k), rk(j), intermediate);
        field.Div(intermediate, lk(k), intermediate);
        field.Sub(ri(j), intermediate, ri(j));
      }

      field.Value(0, li(k));
    }
  }

  for (long long int i = split.Pieces() - 1; i >= 0; i--) {
    cu::Block<DATA> li = left.Sub(i, FieldWidth());
    cu::Block<DATA> ri = right.Sub(i, FieldWidth());

    for (long long int j = split.Quorum() - 1; j >= 0; j--) {
      field.Div(ri(j), li(i), ri(j));
    }

    for (long long int j = split.Quorum() - 1; j >= 0; j--) {
      field.Div(li(j), li(i), li(j));
    }
  }

  char rem[FieldWidth()];

  for (long long int i = split.Pieces() - 2; i >= 0; i--) {
    cu::Block<DATA> li = left.Sub(i, FieldWidth());
    cu::Block<DATA> ri = right.Sub(i, FieldWidth());

    for (size_t k = i + 1; k < split.Quorum(); k++) {
      field.Value(0, rem);
      field.Add(rem, li(k), rem);

      cu::Block<DATA> lk = left.Sub(k, FieldWidth());
      cu::Block<DATA> rk = right.Sub(k, FieldWidth());

      for (long long int j = split.Quorum() - 1; j >= 0; j--) {
        field.Mul(rem, rk(j), intermediate);
        field.Sub(intermediate, ri(j), ri(j));
      }

      for (long long int j = split.Quorum() - 1; j >= 0; j--) {
        field.Mul(rem, lk(j), intermediate);
        field.Sub(intermediate, li(j), li(j));
      }
    }
  }

  field.Value(1, one);

  char zero[FieldWidth()];

  field.Value(0, zero);

  for (size_t i = 0; i < split.Pieces(); i++) {
    cu::Block<DATA> irow = left.Sub(i, FieldWidth());

    assert (field.Compare(irow(i), one) == 0);

    for (size_t j = 0; j < split.Pieces(); j++) {
      if (i != j) {
        assert (field.Compare(irow(j), zero) == 0);
      }
    }
  }

  output.Replace(right);

  if (inverted != NULL) {
    *inverted = true;
  }

  return output;
}

} // Headers
} // hx
