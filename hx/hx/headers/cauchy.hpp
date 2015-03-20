#pragma once

#include <cstddef>
#include <cassert>

#include <cu/block.hpp>

using namespace std;

#include "invertor.hpp"

namespace hx {
namespace Headers {

template <class SPLIT>
class Cauchy {
private:
  const SPLIT split;

  inline size_t FieldWidth() const {
    return SPLIT::FieldType::WIDTH;
  }

public:
  typedef SPLIT SplitType;

  static inline bool WillConstructHeaders(const SPLIT& split) {
    return (split.Pieces() + split.Quorum()) < SPLIT::FieldType::ORDER;
  }

  Cauchy(const SPLIT& s) :
    split(s)
  {
    assert (WillConstructHeaders(split));
  }

  ~Cauchy() {
    // No-op.
  }

  inline SPLIT Split() const {
    return split;
  }

  inline size_t OutputSize() const {
    return split.Pieces() * split.Quorum() * FieldWidth();
  }

  cu::Block<> Generate(const cu::Block<>& out) const {
    assert (out.Size() == OutputSize());

    cu::Block<> output = out.Recast(split.Quorum() * FieldWidth());

    typename SPLIT::FieldType field;

    char one[FieldWidth()];

    field.Value(1, one);

    for (size_t i = 0; i < output.Elements(); i++) {
      cu::Block<> row = output.Sub(i, FieldWidth());

      for (size_t j = 0; j < row.Elements(); j++) {
        field.Value(i + j + 1, row(j));
        field.Div(one, row(j), row(j));
      }
    }

    return output;
  }

  inline bool IsInvertible() const {
    return split.Pieces() == split.Quorum();
  }

  cu::Block<> Invert(const cu::Block<>& hdrs, const cu::Block<>& out) const {
    Invertor<SPLIT> invertor(split);

    bool inverted = true;

    cu::Block<> result = invertor.Invert(hdrs, out, &inverted);

    assert (inverted);

    return result;
  }
};

} // Headers
} // hx
