#pragma once

#include <cstddef>
#include <cassert>

using namespace std;

#include "../block.hpp"
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

  Block<> Generate(const Block<>& out) const {
    assert (out.Size() == OutputSize());

    Block<> output = out.Recast(split.Quorum() * FieldWidth());

    typename SPLIT::FieldType field;

    char one[FieldWidth()];

    field.Value(1, one);

    for (size_t i = 0; i < output.Elements(); i++) {
      Block<> row = output.Sub(i, FieldWidth());

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

  Block<> Invert(const Block<>& hdrs, const Block<>& out) const {
    Invertor<SPLIT> invertor(split);

    bool inverted = true;

    Block<> result = invertor.Invert(hdrs, out, &inverted);

    assert (inverted);

    return result;
  }
};

} // Headers
} // hx
