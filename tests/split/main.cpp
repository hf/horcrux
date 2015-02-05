#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <hx/split.hpp>
#include <hx/field/rijndael.hpp>

TEST_CASE ("Split should be initialized correctly.", "[hx::Split]") {
  hx::Split< hx::Field::Rijndael > a(3, 2);

  REQUIRE (a.Pieces() == 3);
  REQUIRE (a.Quorum() == 2);

  hx::Split< hx::Field::Rijndael > b = a;

  REQUIRE (b.Pieces() == 3);
  REQUIRE (b.Quorum() == 2);

  hx::Split< hx::Field::Rijndael > c(10);

  REQUIRE (c.Pieces() == 10);
  REQUIRE (c.Quorum() == 10);
}
