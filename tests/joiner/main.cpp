#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <hx/split.hpp>
#include <hx/field/rijndael.hpp>
#include <hx/headers/cauchy.hpp>
#include <hx/splitter.hpp>
#include <hx/joiner.hpp>
#include <hx/block.hpp>

TEST_CASE ("Joiner should initialize properly.", "[hx::Joiner]") {
  hx::Joiner< hx::Field::Rijndael > joiner(10);

  REQUIRE (joiner.Quorum() == 10);
  REQUIRE (joiner.Pieces() == 10);

  REQUIRE (joiner.OutputSize(10 * hx::Field::Rijndael::WIDTH * 3) == (10 * hx::Field::Rijndael::WIDTH * 3));
}

TEST_CASE ("Joiner should join previously split data.", "[hx::Joiner]") {
  hx::Split< hx::Field::Rijndael > split(10, 3);

  hx::Headers::Cauchy< hx::Split< hx::Field::Rijndael > > cauchy(split);

  const size_t hdata_s = cauchy.OutputSize();

  char hdata[hdata_s];

  hx::Block<> headers = cauchy.Generate(hx::Block<>(hdata, hdata_s));

  hx::Splitter< hx::Split< hx::Field::Rijndael > > splitter(split);

  const size_t data_s = splitter.AlignBlock(1024);
  const size_t out_s  = splitter.OutputSize(data_s);

  char data[data_s];
  char out[out_s];

  hx::Field::Rijndael field;

  for (size_t i = 0; i < data_s; i += hx::Field::Rijndael::WIDTH) {
    field.Value(i % hx::Field::Rijndael::ORDER, data + i);
  }

  hx::Block<> pieces = splitter.Split(headers, hx::Block<>(data, data_s), hx::Block<>(out, out_s));

  hx::Joiner< hx::Field::Rijndael > joiner(split.Quorum());

  hx::Headers::Cauchy< hx::Split< hx::Field::Rijndael > > joiner_cauchy(hx::Split< hx::Field::Rijndael >(split.Quorum(), split.Quorum()));

  char ihdata[joiner_cauchy.OutputSize()];

  hx::Block<> joiner_inverted_headers = joiner_cauchy.Invert(headers.Range(0, split.Quorum()), hx::Block<>(ihdata, joiner_cauchy.OutputSize()));
  hx::Block<> joiner_pieces = pieces.Range(0, split.Quorum());

  REQUIRE (joiner_pieces.Size() == data_s);

  const size_t jdata_s = joiner.OutputSize(joiner_pieces);

  char jdata[jdata_s];

  hx::Block<> joined = joiner.Join(joiner_inverted_headers, joiner_pieces, hx::Block<>(jdata, jdata_s));

  REQUIRE (joined.Elements() == data_s / hx::Field::Rijndael::WIDTH);
  REQUIRE (joined.Width() == hx::Field::Rijndael::WIDTH);

  for (size_t i = 0; i < joined.Elements(); i++) {
    REQUIRE (field.Compare(joined(i), data + i * hx::Field::Rijndael::WIDTH) == 0);
  }
}
