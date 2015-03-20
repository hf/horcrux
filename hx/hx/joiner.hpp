#pragma once

#include <cstddef>
#include <cassert>

#include <cu/block.hpp>

using namespace std;

namespace hx {

template <typename FIELD>
class Joiner {
private:
  size_t quorum;

  inline size_t FieldWidth() const {
    return FIELD::WIDTH;
  }

public:
  typedef FIELD FieldType;

  Joiner(size_t q) :
    quorum(q)
  {
    assert (quorum > 0);
  }

  Joiner(const Joiner<FIELD>& o) :
    quorum(o.quorum)
  {
    // No-op.
  }

  ~Joiner() {
    // No-op.
  }

  inline size_t Pieces() const {
    return Quorum();
  }

  inline size_t Quorum() const {
    return quorum;
  }

  inline size_t OutputSize(size_t pieces_size) const {
    assert (pieces_size % (Quorum() * FieldWidth()) == 0);
    assert (pieces_size / (Quorum() * FieldWidth()) > 0);

    return pieces_size;
  }

  inline size_t OutputSize(const cu::Block<>& pieces) {
    return OutputSize(pieces.Size());
  }

  cu::Block<> Join(const cu::Block<>& hdrs, const cu::Block<>& pcs, const cu::Block<>& out) const {
    assert (hdrs.Size() == Pieces() * Quorum() * FieldWidth());
    assert (pcs.Size() % (Quorum() * FieldWidth()) == 0);
    assert (pcs.Size() == out.Size());

    cu::Block<> pieces = pcs.Recast(pcs.Size() / (Quorum() * FieldWidth()));
    cu::Block<> headers = hdrs.Recast(Quorum() * FieldWidth());
    cu::Block<> output = out.Recast(FieldWidth());

    assert (pieces.Elements() == headers.Elements());

    FIELD field;

    char intermediate[FieldWidth()];

    for (size_t j = 0; j < output.Elements(); j++) {
      size_t i = j % Quorum();
      size_t k = j / Quorum();

      cu::Block<> header = headers.Sub(i, FieldWidth());

      field.Value(0, output(j));

      assert (header.Elements() == pieces.Elements());

      for (size_t c = 0; c < pieces.Elements(); c++) {
        // TODO: Refactor this to use speedier SubGet method,
        // also for header(c)
        cu::Block<> piece = pieces.Sub(c, FieldWidth());

        field.Mul(header(c), piece(k), intermediate);
        field.Add(output(j), intermediate, output(j));
      }
    }

    return output;
  }
};

} // hx
