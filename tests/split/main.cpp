#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <galois/field/rijndael.hpp>

#include <hx/split.hpp>

TEST_CASE ("Split should be initialized correctly.", "[hx::Split]") {
  hx::Split< galois::Field::Rijndael > a(3, 2);

  REQUIRE (a.Pieces() == 3);
  REQUIRE (a.Quorum() == 2);

  hx::Split< galois::Field::Rijndael > b = a;

  REQUIRE (b.Pieces() == 3);
  REQUIRE (b.Quorum() == 2);

  hx::Split< galois::Field::Rijndael > c(10);

  REQUIRE (c.Pieces() == 10);
  REQUIRE (c.Quorum() == 10);
}
