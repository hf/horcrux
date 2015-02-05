#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <hx/field/rijndael.hpp>

#include <cstddef>

using namespace std;

TEST_CASE ("Rijndael field should be defined properly.", "[hx::Field::Rijndael]") {
  REQUIRE (hx::Field::Rijndael::ORDER == 256);
  REQUIRE (hx::Field::Rijndael::WIDTH == 1);
}

TEST_CASE ("Rijndael field should add / subtract correctly.", "[hx::Field::Rijndael]") {
  hx::Field::Rijndael field;

  for (size_t i = 0; i <= 0xFF; i++) {
    for (size_t j = 0; j <= 0xFF; j++) {
      unsigned char a = i & 0xFF;
      unsigned char b = j & 0xFF;
      unsigned char r = 0;

      REQUIRE (field.Add((char*) &a, (char*) &b, (char*) &r) == (char*) &r);
      REQUIRE (field.Sub((char*) &r, (char*) &b, (char*) &r) == (char*) &r);

      REQUIRE (r == a);

      field.Add((char*) &a, (char*) &b, (char*) &r);
      field.Sub((char*) &r, (char*) &a, (char*) &r);

      REQUIRE (r == b);
    }
  }
}

TEST_CASE ("Rijndael field should multiply and behave correctly with multiplicative inverses", "[hx::Field::Rijndael]") {
  hx::Field::Rijndael field;

  REQUIRE (hx::Field::Rijndael::INVERSES[0] == 0);

  for (size_t i = 1; i <= 0xFF; i++) {
    unsigned char result = i & 0xFF;
    unsigned char inv = hx::Field::Rijndael::INVERSES[i];

    REQUIRE (field.Mul((char*) &result, (char*) &inv, (char*) &result) == (char*) &result);

    REQUIRE (result == 1);
  }
}