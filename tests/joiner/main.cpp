#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <galois/field/rijndael.hpp>
#include <cu/block.hpp>

#include <hx/split.hpp>
#include <hx/headers/cauchy.hpp>
#include <hx/splitter.hpp>
#include <hx/joiner.hpp>

TEST_CASE ("Joiner should initialize properly.", "[hx::Joiner]") {
  hx::Joiner< galois::Field::Rijndael > joiner(10);

  REQUIRE (joiner.Quorum() == 10);
  REQUIRE (joiner.Pieces() == 10);

  REQUIRE (joiner.OutputSize(10 * galois::Field::Rijndael::WIDTH * 3) == (10 * galois::Field::Rijndael::WIDTH * 3));
}

TEST_CASE ("Joiner should join previously split data.", "[hx::Joiner]") {
  hx::Split< galois::Field::Rijndael > split(10, 3);

  hx::Headers::Cauchy< hx::Split< galois::Field::Rijndael > > cauchy(split);

  const size_t hdata_s = cauchy.OutputSize();

  char hdata[hdata_s];

  cu::Block<> headers = cauchy.Generate(cu::Block<>(hdata, hdata_s));

  hx::Splitter< hx::Split< galois::Field::Rijndael > > splitter(split);

  const size_t data_s = splitter.AlignBlock(1024);
  const size_t out_s  = splitter.OutputSize(data_s);

  char data[data_s];
  char out[out_s];

  galois::Field::Rijndael field;

  for (size_t i = 0; i < data_s; i += galois::Field::Rijndael::WIDTH) {
    field.Value(i % galois::Field::Rijndael::ORDER, data + i);
  }

  cu::Block<> pieces = splitter.Split(headers, cu::Block<>(data, data_s), cu::Block<>(out, out_s));

  hx::Joiner< galois::Field::Rijndael > joiner(split.Quorum());

  hx::Headers::Cauchy< hx::Split< galois::Field::Rijndael > > joiner_cauchy(hx::Split< galois::Field::Rijndael >(split.Quorum(), split.Quorum()));

  char ihdata[joiner_cauchy.OutputSize()];

  cu::Block<> joiner_inverted_headers = joiner_cauchy.Invert(headers.Range(0, split.Quorum()), cu::Block<>(ihdata, joiner_cauchy.OutputSize()));
  cu::Block<> joiner_pieces = pieces.Range(0, split.Quorum());

  REQUIRE (joiner_pieces.Size() == data_s);

  const size_t jdata_s = joiner.OutputSize(joiner_pieces);

  char jdata[jdata_s];

  cu::Block<> joined = joiner.Join(joiner_inverted_headers, joiner_pieces, cu::Block<>(jdata, jdata_s));

  REQUIRE (joined.Elements() == data_s / galois::Field::Rijndael::WIDTH);
  REQUIRE (joined.Width() == galois::Field::Rijndael::WIDTH);

  for (size_t i = 0; i < joined.Elements(); i++) {
    REQUIRE (field.Compare(joined(i), data + i * galois::Field::Rijndael::WIDTH) == 0);
  }
}
