#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <galois/field/rijndael.hpp>
#include <cu/block.hpp>

#include <hx/split.hpp>
#include <hx/headers/cauchy.hpp>

template <typename FIELD>
static void CheckInverted(const cu::Block<>& a, const cu::Block<>& b) {
  REQUIRE (a.Elements() == b.Elements());
  REQUIRE (a.Width() == b.Width());

  FIELD field;

  char resultd[a.Size()];

  cu::Block<> result(resultd, a.Width(), a.Elements());

  char sum[FIELD::WIDTH];
  char intermediate[FIELD::WIDTH];

  for (size_t i = 0; i < result.Elements(); i++) {
    cu::Block<> ai = a.Sub(i, FIELD::WIDTH);
    cu::Block<> ri = result.Sub(i, FIELD::WIDTH);

    for (size_t j = 0; j < result.Elements(); j++) {
      field.Value(0, sum);

      for (size_t k = 0; k < result.Elements(); k++) {
        cu::Block<> bk = b.Sub(k, FIELD::WIDTH);

        field.Mul(ai(k), bk(j), intermediate);
        field.Add(sum, intermediate, sum);
      }

      field.Value(0, ri(j));
      field.Add(ri(j), sum, ri(j));
    }
  }

  char one[FIELD::WIDTH];
  char zero[FIELD::WIDTH];

  field.Value(1, one);
  field.Value(0, zero);

  for (size_t i = 0; i < result.Elements(); i++) {
    cu::Block<> ri = result.Sub(i, FIELD::WIDTH);

    for (size_t j = 0; j < ri.Elements(); j++) {
      if (i == j) {
        REQUIRE (field.Compare(ri(j), one) == 0);
      } else {
        REQUIRE (field.Compare(ri(j), zero) == 0);
      }
    }
  }
}

TEST_CASE ("Cauchy headers should initialize properly.", "[hx::Headers::Cauchy]") {
  REQUIRE (hx::Headers::Cauchy< hx::Split<galois::Field::Rijndael> >::WillConstructHeaders(hx::Split<galois::Field::Rijndael>(251, 5)) == false);

  hx::Headers::Cauchy< hx::Split<galois::Field::Rijndael> > a(hx::Split<galois::Field::Rijndael>(5, 3));

  REQUIRE (a.IsInvertible() == false);

  hx::Split<galois::Field::Rijndael> split = a.Split();

  REQUIRE (split.Pieces() == 5);
  REQUIRE (split.Quorum() == 3);
}

TEST_CASE ("Cauchy headers should generate properly.", "[hx::Headers::Cauchy]") {
  hx::Headers::Cauchy< hx::Split<galois::Field::Rijndael> > a(hx::Split<galois::Field::Rijndael>(5, 3));

  REQUIRE (a.OutputSize() == 5 * 3 * galois::Field::Rijndael::WIDTH);

  char ahd[a.OutputSize()];

  cu::Block<> aheaders = a.Generate(cu::Block<>(ahd, a.OutputSize()));

  REQUIRE (aheaders.Elements() == 5);
  REQUIRE (aheaders.Width() == 3 * galois::Field::Rijndael::WIDTH);
}

TEST_CASE ("Cauchy headers should invert properly.", "[hx::Headers::Cauchy]") {
  hx::Headers::Cauchy< hx::Split<galois::Field::Rijndael> > b(hx::Split<galois::Field::Rijndael>(5, 5));

  REQUIRE (b.IsInvertible() == true);

  char bhd[b.OutputSize()];

  cu::Block<> bheaders = b.Generate(cu::Block<>(bhd, b.OutputSize()));

  REQUIRE (bheaders.Elements() == 5);
  REQUIRE (bheaders.Width() == 5 * galois::Field::Rijndael::WIDTH);

  char ibhd[b.OutputSize()];

  cu::Block<> ibheaders = b.Invert(bheaders, cu::Block<>(ibhd, b.OutputSize()));

  CheckInverted< galois::Field::Rijndael >(bheaders, ibheaders);
}


