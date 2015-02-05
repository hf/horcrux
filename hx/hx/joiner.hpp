#pragma once

#include <cstddef>
#include <cassert>

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

  inline size_t OutputSize(const Block<>& pieces) {
    return OutputSize(pieces.Size());
  }

  Block<> Join(const Block<>& hdrs, const Block<>& pcs, const Block<>& out) const {
    assert (hdrs.Size() == Pieces() * Quorum() * FieldWidth());
    assert (pcs.Size() % (Quorum() * FieldWidth()) == 0);
    assert (pcs.Size() == out.Size());

    Block<> pieces = pcs.Recast(pcs.Size() / (Quorum() * FieldWidth()));
    Block<> headers = hdrs.Recast(Quorum() * FieldWidth());
    Block<> output = out.Recast(FieldWidth());

    assert (pieces.Elements() == headers.Elements());

    FIELD field;

    char intermediate[FieldWidth()];

    for (size_t j = 0; j < output.Elements(); j++) {
      size_t i = j % Quorum();
      size_t k = j / Quorum();

      Block<> header = headers.Sub(i, FieldWidth());

      field.Value(0, output(j));

      assert (header.Elements() == pieces.Elements());

      for (size_t c = 0; c < pieces.Elements(); c++) {
        // TODO: Refactor this to use speedier SubGet method,
        // also for header(c)
        Block<> piece = pieces.Sub(c, FieldWidth());

        field.Mul(header(c), piece(k), intermediate);
        field.Add(output(j), intermediate, output(j));
      }
    }

    return output;
  }
};

} // hx
