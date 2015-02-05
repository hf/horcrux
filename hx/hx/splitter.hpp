#pragma once

#include <cstddef>
#include <cassert>

using namespace std;

#include "block.hpp"

namespace hx {

template <class SPLIT>
class Splitter {
private:
  SPLIT split;

inline size_t FieldWidth() const {
  return SPLIT::FieldType::WIDTH;
}

public:
  typedef SPLIT SplitType;

  Splitter(const SPLIT& s) :
    split(s)
  {
    // No-op.
  }

  ~Splitter() {
    // No-op.
  }

  inline size_t PieceSize(const size_t block) const {
    return block / (split.Quorum() * FieldWidth());
  }

  inline size_t AlignBlock(const size_t block) const {
    const size_t retval = block + split.Quorum() * FieldWidth() - (block % (split.Quorum() * FieldWidth()));

    assert (retval % (split.Quorum() * FieldWidth()) == 0);

    return retval;
  }

  inline size_t OutputSize(const size_t block) const {
    return split.Pieces() * PieceSize(block);
  }

  Block<> Split(const Block<>& hdrs, const Block<>& in, const Block<>& out) const {
    // check headers are [pieces x quorum] sized
    assert (hdrs.Size() == split.Pieces() * split.Quorum() * FieldWidth());

    // check at least one sequence of data
    assert (in.Size() >= split.Quorum() * FieldWidth());

    // check that output can hold everything
    assert (out.Size() == OutputSize(in.Size()));

    Block<> headers = hdrs.Recast(split.Quorum() * FieldWidth());
    Block<> input = in.Recast(split.Quorum() * FieldWidth());
    Block<> output = out.Recast(PieceSize(input.Size()));

    assert (headers.Elements() == split.Pieces());
    assert (input.Elements() >= 1);
    assert (output.Elements() == split.Pieces());

    typename SPLIT::FieldType field;

    char intermediate[FieldWidth()];
    char ck[FieldWidth()];

    for (size_t i = 0; i < split.Pieces(); i++) {
      Block<> header = headers.Sub(i, FieldWidth());
      Block<> piece = output.Sub(i, FieldWidth());

      assert (header.Elements()   == split.Quorum());
      assert (piece.Elements() == PieceSize(input.Size()) / FieldWidth());

      assert (input.Elements() == piece.Elements());

      for (size_t k = 0; k < piece.Elements(); k++) {
        Block<> sequence = input.Sub(k, FieldWidth());

        assert (sequence.Elements() == split.Quorum());

        field.Value(0, ck);

        // dot product
        for (size_t j = 0; j < split.Quorum(); j++) {
          field.Mul(header(j), sequence(j), intermediate);
          field.Add(intermediate, ck, ck);
        }

        field.Value(0, piece(k));
        field.Add(piece(k), ck, piece(k));
      }
    }

    return output;
  }
};

} // hx
