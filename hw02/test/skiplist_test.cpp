#include "gtest/gtest.h"
#include <string>
#include <skiplist/skiplist.h>
#include <cstdio>
#include <functional>

#include <ctime>
#include <cstdlib>

using namespace std;

TEST(SkipListTest, Empty) {
  srand(time(0));
  SkipList<int, string, 8> sk;
  ASSERT_EQ(nullptr, sk.Get(100));
  ASSERT_EQ(sk.cend(), sk.cbegin())  << "Begin iterator fails";
  ASSERT_EQ(sk.cend(), sk.cfind(10)) << "Find iterator fails";
}

TEST(SkipListTest, SimplePut) {
  SkipList<int, string, 8> sk;

  const std::string *pOld = sk.Put(10, "test");
  ASSERT_EQ(nullptr, pOld);

  pOld = sk[10];
  ASSERT_NE(nullptr, pOld)         << "Value found";
  ASSERT_EQ(string("test"), *pOld) << "Value is correct";

  pOld = sk.Get(10);
  ASSERT_NE(nullptr, pOld)         << "Value found";
  ASSERT_EQ(string("test"), *pOld) << "Value is correct";

  Iterator<int,std::string> it = sk.cbegin();
  ASSERT_NE(sk.cend(), it)              << "Iterator is not empty";
  ASSERT_EQ(10, it.key())               << "Iterator key is correct";
  ASSERT_EQ(string("test"), it.value()) << "Iterator value is correct";
  ASSERT_EQ(string("test"), *it)        << "Iterator value is correct";
}

TEST(SkipListTest, PutIfPresented) {
  SkipList<int, string, 8> sk;
  sk.Put(1, "Maine Coon");
  sk.Put(2, "Siamese cat");
  sk.Put(3, "Norwegian forest cat");
  sk.Put(4, "Sphynx cat");
  sk.Put(5, "Manul cat");

  string* old = sk.Put(5, "Pallas cat");
  ASSERT_NE(nullptr, old);
  printf("%s\n", old->c_str());
  delete old;
}

TEST(SkipListTest, IterateOver) {
  SkipList<int, string, 8> sk;

  sk.Put(5, "five");
  sk.Put(6, "six");
  sk.Put(1, "one");
  sk.Put(9, "nine");
  sk.Put(3, "three");

  for (auto p = sk.cbegin(); p != sk.cend(); p++)
    printf("%d -> %s\n", p.key(), p.value().c_str());

  printf("------------\n");
  auto f = sk.cfind(1);

  for (; f != sk.cend(); f++)
    printf("%d -> %s\n", f.key(), f.value().c_str());
}

TEST(SkipListTest, Delete) {
  SkipList<string, double, 8> sk;

  sk.Put("pi", 3.1415);
  sk.Put("e",  2.7183);
  sk.Put("ln2", 0.6931);

  double* pi = sk.Delete("pi");
  ASSERT_NE(nullptr, pi);
  printf("pi = %g\n", *pi);
  delete pi;
  ASSERT_EQ(sk.cfind("pi"), sk.cend());

  sk.Put("ln10",  2.30258);
  sk.Put("sqrt2", 1.4142);

  printf("----------\n");
  for (auto p = sk.cbegin(); p != sk.cend(); p++)
    printf("%s -> %g\n", p.key().c_str(), p.value());
}
