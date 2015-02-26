#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <galois/field/rijndael.hpp>

#include <hx/splitter.hpp>
#include <hx/split.hpp>
#include <hx/headers/cauchy.hpp>

TEST_CASE ("Splitter should initialize properly.", "[hx::Splitter]") {
  hx::Split< galois::Field::Rijndael > split(10, 3);

  hx::Splitter< hx::Split< galois::Field::Rijndael > > splitter(split);

  REQUIRE (splitter.AlignBlock(1024) == 1024 + 2 * galois::Field::Rijndael::WIDTH);
  REQUIRE (splitter.OutputSize(1024) == 10 * (1024 / (3 * galois::Field::Rijndael::WIDTH)));
}

TEST_CASE ("Splitter should split.", "[hx::Splitter]") {
  hx::Split< galois::Field::Rijndael > split(10, 3);

  hx::Headers::Cauchy< hx::Split< galois::Field::Rijndael > > cauchy(split);

  const size_t hdata_s = cauchy.OutputSize();

  char hdata[hdata_s];

  hx::Block<> headers = cauchy.Generate(hx::Block<>(hdata, hdata_s));

  hx::Splitter< hx::Split< galois::Field::Rijndael > > splitter(split);

  const size_t data_s = splitter.AlignBlock(1024);
  const size_t out_s  = splitter.OutputSize(data_s);

  char data[data_s];
  char out[out_s];

  hx::Block<> result = splitter.Split(headers, hx::Block<>(data, data_s), hx::Block<>(out, out_s));

  REQUIRE (result.Elements() == split.Pieces());
  REQUIRE (result.Width() == splitter.PieceSize(data_s));
}
