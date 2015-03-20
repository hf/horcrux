// Under PUBLIC DOMAIN.

#pragma once

#include <cstddef>
#include <cassert>

using namespace std;

namespace hx {

template <class FIELD>
class Split {
private:
  const size_t pieces;
  const size_t quorum;

  inline void Check() const {
    assert (pieces > 0);
    assert (quorum > 0);
    assert (quorum <= pieces);
  }

public:
  typedef FIELD FieldType;

  Split(size_t n, size_t m) :
    pieces(n),
    quorum(m)
  {
    Check();
  }

  Split(size_t n) :
    pieces(n),
    quorum(n)
  {
    Check();
  }

  Split(const Split<FIELD>& o) :
    pieces(o.pieces),
    quorum(o.quorum)
  {
    Check();
  }

  ~Split() {
    // No-op.
  }

  inline size_t Pieces() const {
    return pieces;
  }

  inline size_t Quorum() const {
    return quorum;
  }

};

} // hx
