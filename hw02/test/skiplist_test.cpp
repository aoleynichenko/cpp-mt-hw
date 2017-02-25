#include "gtest/gtest.h"
#include <string>
#include <skiplist/skiplist.h>
#include <cstdio>

using namespace std;

SkipList<int, string, 8> sk;

TEST(SkipListTest, Empty) {
  ASSERT_EQ(nullptr, sk.Get(100));
  ASSERT_EQ(sk.cend(), sk.cbegin())  << "Begin iterator fails";
  ASSERT_EQ(sk.cend(), sk.cfind(10)) << "Find iterator fails";
}

TEST(SkipListTest, SimplePut) {
  SkipList<int, string, 8> sk;

  sk.Put(11, "dynamic_cast is bullshit");
  sk.Put(12, "const_cast is bullshit");
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

TEST(SkipListTest, IterateOver) {
  sk.Put(5, "five");
  sk.Put(6, "six");
  sk.Put(1, "one");
  sk.Put(9, "nine");
  sk.Put(3, "three");

  for (auto p = sk.cbegin(); p != sk.cend(); p++)
    printf("%d -> %s\n", p.key(), p.value().c_str());
}
