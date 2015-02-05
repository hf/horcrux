#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <hx/block.hpp>

#include <cstdlib>

TEST_CASE ("Block should initialize properly.", "[hx::Block]") {
  char data[1024];

  hx::Block<> a(data, 1024);

  REQUIRE (a.First() == data);
  REQUIRE (a.Last() == data + 1023);
  REQUIRE (a.Elements() == 1024);
  REQUIRE (a.Size() == 1024);
  REQUIRE (a.Data() == data);
  REQUIRE (a(1) == data + 1);
  REQUIRE (a.Width() == 1);

  hx::Block<> b(data, 2, 1024 / 2);

  REQUIRE (b.First() == data);
  REQUIRE (b.Last() == data + 1022);
  REQUIRE (b.Elements() == 1024 / 2);
  REQUIRE (b.Size() == 1024);
  REQUIRE (b.Data() == data);
  REQUIRE (b(1) == data + 2);
  REQUIRE (b.Width() == 2);
}

TEST_CASE ("Block should return proper subrange.", "[hx::Block]") {
  char data[1024];

  hx::Block<> a(data, 1024);

  hx::Block<> asub = a.Range(0, 1024);

  REQUIRE (asub.Elements() == 1024);
  REQUIRE (asub.Data() == data);
  REQUIRE (asub.Size() == 1024);
  REQUIRE (asub.First() == data);
  REQUIRE (asub.Last() == data + 1023);
  REQUIRE (asub(1) == data + 1);

  hx::Block<> bsub = a.Range(1, 1023);

  REQUIRE (bsub.Elements() == 1022);
  REQUIRE (bsub.Data() == data + 1);
  REQUIRE (bsub.Size() == 1022);
  REQUIRE (bsub.First() == data + 1);
  REQUIRE (bsub.Last() == (data + 1) + 1021);
  REQUIRE (bsub(1) == data + 2);

  hx::Block<> b(data, 2, 1024 / 2);

  hx::Block<> csub = b.Range(0, 1024 / 2);

  REQUIRE (csub.Elements() == 1024 / 2);
  REQUIRE (csub.Data() == data);
  REQUIRE (csub.Size() == 1024);
  REQUIRE (csub.First() == data);
  REQUIRE (csub.Last() == data + 1022);
  REQUIRE (csub(1) == data + 2);

  hx::Block<> dsub = b.Range(2, 511);

  REQUIRE (dsub.Elements() == 509);
  REQUIRE (dsub.Size() == 509 * 2);
  REQUIRE (dsub.First() == data + 4);
  REQUIRE (dsub.Last() == (data + 4) + 508 * 2);
  REQUIRE (dsub(1) == (data + 4) + 2);
}

TEST_CASE ("Block should extract proper sub-block.", "[hx::Block]") {
  char data[6 * 100];

  hx::Block<> a(data, 100, 6);

  hx::Block<> asub = a.Sub(0, 1);

  REQUIRE (asub.Elements() == 100);
  REQUIRE (asub.Size() == 100);
  REQUIRE (asub.First() == data);
  REQUIRE (asub.Last() == data + 99);

  hx::Block<> bsub = a.Sub(1, 2);

  REQUIRE (bsub.Elements() == 50);
  REQUIRE (bsub.Size() == 100);
  REQUIRE (bsub.First() == data + 100);
  REQUIRE (bsub.Last() == (data + 100) + 98);
}

TEST_CASE ("Block should recast properly.", "[hx::Block]") {
  char data[1024];

  hx::Block<> a(data, 2, 1024 / 2);

  hx::Block<> r = a.Recast(4);

  REQUIRE (r.Elements() == 1024 / 4);
  REQUIRE (r.Width() == 4);
  REQUIRE (r.Size() == 1024);
  REQUIRE (r.First() == data);
  REQUIRE (r.Last() == data + 1020);
}

TEST_CASE ("Block<char* const> should exchange properly.", "[hx::Block<char* const>]") {
  const size_t data_s = 1026;

  char orig_data[data_s];
  char data[data_s];

  srand(data_s);

  for (size_t i = 0; i < data_s; i++) {
    data[i] = orig_data[i] = rand() % 256;
  }

  hx::Block<> a(data, data_s / 2, 2);

  a.Exchange(0, 1);

  for (size_t i = 0; i < data_s / 2; i++) {
    REQUIRE (data[i] == orig_data[i + data_s / 2]);
  }

  for (size_t i = data_s / 2; i < data_s; i++) {
    REQUIRE (data[i] == orig_data[i - data_s / 2]);
  }
}

TEST_CASE ("Block<char* const> should replace contents properly.", "[hx::Block<char* const>]") {
  const size_t data_s = 1026;

  char orig_data[data_s];
  char data[data_s];

  srand(data_s);

  for (size_t i = 0; i < data_s; i++) {
    orig_data[i] = rand() % 256;
    data[i] = rand() % 256;
  }

  hx::Block<> a(data, data_s / 2, 2);
  hx::Block<> b(orig_data, data_s);

  a.Replace(b);

  for (size_t i = 0; i < data_s; i++) {
    REQUIRE (data[i] == orig_data[i]);
  }
}
