// Under PUBLIC DOMAIN.

#pragma once

#include <cstddef>
#include <cassert>

#include <vector>

using namespace std;

#include <cu/block.hpp>

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

  void Split(const cu::Block<>& hdrs, const cu::Block<>& in, const std::vector<char*>& outputs) const {
    assert (outputs.size() == split.Pieces());

    #ifndef NDEBUG
    for (size_t i = 0; i < outputs.size(); i++) {
      assert (outputs[i] != NULL);

      volatile char touch = *(outputs[i] + OutputSize(in.Size()) - 1);
      assert (touch >= 0 || touch <= 0);
    }
    #endif

    // check headers are [pieces x quorum] sized
    assert (hdrs.Size() == split.Pieces() * split.Quorum() * FieldWidth());

    // check at least one sequence of data
    assert (in.Size() >= split.Quorum() * FieldWidth());

    cu::Block<> headers = hdrs.Recast(split.Quorum() * FieldWidth());
    cu::Block<> input = in.Recast(split.Quorum() * FieldWidth());

    assert (headers.Elements() == split.Pieces());
    assert (input.Elements() >= 1);

    typename SPLIT::FieldType field;

    char intermediate[FieldWidth()];
    char ck[FieldWidth()];

    for (size_t i = 0; i < split.Pieces(); i++) {
      cu::Block<> header = headers.Sub(i, FieldWidth());
      cu::Block<> piece(outputs[i], FieldWidth(), PieceSize(in.Size()) / FieldWidth());

      assert (header.Elements()   == split.Quorum());
      assert (piece.Elements() == PieceSize(input.Size()) / FieldWidth());

      assert (input.Elements() == piece.Elements());

      for (size_t k = 0; k < piece.Elements(); k++) {
        cu::Block<> sequence = input.Sub(k, FieldWidth());

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

  }

  cu::Block<> Split(const cu::Block<>& hdrs, const cu::Block<>& in, const cu::Block<>& out) const {
    assert (out.Size() == OutputSize(in.Size()));

    cu::Block<> output = out.Recast(PieceSize(in.Size()));

    assert (output.Elements() == split.Pieces());

    std::vector<char*> outputs(output.Elements());

    for (size_t i = 0; i < output.Elements(); i++) {
      outputs[i] = output(i);
    }

    Split(hdrs, in, outputs);

    return output;
  }
};

} // hx
