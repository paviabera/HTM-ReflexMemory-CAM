/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2013, Numenta, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero Public License for more details.
 *
 * You should have received a copy of the GNU Affero Public License
 * along with this program.  If not, see http://www.gnu.org/licenses.
 * --------------------------------------------------------------------- */

/**
 * @file
 */
#include <gtest/gtest.h>
#include <htm/utils/Random.hpp>

#include <htm/os/Env.hpp>
#include <htm/utils/Log.hpp>
#include <htm/os/Timer.hpp>

#include <fstream>
#include <sstream>
#include <vector>

namespace testing {

using namespace htm;
using namespace std;

TEST(RandomTest, Seeding) {
  {
  Random r;
  ASSERT_TRUE(r.getSeed() != 0) << "Should initialize with randomized seed";

  auto x = r.getUInt32();
  ASSERT_NE(x,  0u);
  }

  // test getSeed
  {
    Random r(98765);
    ASSERT_EQ(98765U, r.getSeed());
  }

  { // seed & equals
  Random r1(1);
  Random r2(2);
  ASSERT_NE(r1.getUInt32(), r2.getUInt32());
  ASSERT_NE(r1, r2) << "Randoms with different seed must be different";

  Random r1b(1);
  ASSERT_NE(r1, r1b) << "different steps";
  auto x = r1b.getUInt32();
  ASSERT_EQ(r1, r1b) << "Randoms with same seed must be the same." << x;
  }

  { //MAX_INT seed
  Random r(-1);
  ASSERT_EQ(r(), 419326371u);
  }

  for(int i=0; i< 10; i++) {
    ASSERT_NE(Random(0).getSeed(), Random(0).getSeed()) << "Randomly seeded generators should not be identical!";
  }

}


TEST(RandomTest, CopyConstructor) {
  // test copy constructor.
  Random r1(289436);
  for (int i = 0; i < 100; i++)
    r1.getUInt32();
  Random r2(r1); //copy

  UInt32 v1, v2;
  for (int i = 0; i < 100; i++) {
    v1 = r1.getUInt32();
    v2 = r2.getUInt32();
    ASSERT_EQ(v1, v2) << "copy constructor";
  }
}


TEST(RandomTest, OperatorEquals) {
  // test operator=
  Random r1(289436);
  for (int i = 0; i < 100; i++)
    r1.getUInt32();
  Random r2(86726008);
  for (int i = 0; i < 100; i++)
    r2.getUInt32();

  r2 = r1;
  NTA_ASSERT(r1 == r2);

  UInt32 v1, v2;
  for (int i = 0; i < 100; i++) {
    v1 = r1.getUInt32();
    v2 = r2.getUInt32();
    ASSERT_EQ(v1, v2) << "operator=";
  }
}


TEST(RandomTest, testSerialization) { // test serialization/deserialization using Cereal
  const UInt SEED = 862973u;
  Random r1(SEED);
  ASSERT_EQ(r1.getSeed(), SEED) << "RNG seed not set properly";

  //burn-in
  for (int i = 0; i < 100; i++) r1.getUInt32();
  EXPECT_EQ(r1.getUInt32(), 2276275187u) << "Before serialization must be same";

  // serialize
  std::stringstream ss;
  r1.save(ss);

  // deserialize into r2
  Random r2;
  r2.load(ss);

  // r1 and r2 should be identical
  EXPECT_EQ(r1, r2) << "load from serialization";
  EXPECT_EQ(r2.getUInt32(), 3537119063u) << "Deserialized is not deterministic";
  EXPECT_EQ(r1.getUInt32(), 3537119063u); //move the same number of steps
  EXPECT_EQ(r1.getSeed(), r2.getSeed());
  EXPECT_EQ(r2.getSeed(), SEED) << "Rng deserialized seed is not the same!";

  for (int i = 0; i < 100; i++) {
    EXPECT_EQ(r1.getUInt32(), r2.getUInt32()) << "serialization";
  }

}


TEST(RandomTest, ReturnInCorrectRange) {
  // make sure that we are returning values in the correct range
  // @todo perform statistical tests
  Random r;
  UInt64 seed = r.getSeed();
  ASSERT_TRUE(seed != 0) << "seed not zero";
  int i;
  UInt32 max32 = 10000000;
  for (i = 0; i < 2000; i++) {
    UInt32 i32 = r.getUInt32(max32);
    ASSERT_TRUE(i32 < max32) << "UInt32";
    Real64 r64 = r.getReal64();
    ASSERT_TRUE(r64 >= 0.0 && r64 < 1.0) << "Real64";
  }
}


TEST(RandomTest, getUInt32) {
  // tests for getUInt32
  Random r1(1);
  EXPECT_EQ(1791095845u, r1.getUInt32())
      << "check getUInt64, seed 1, first call";
  EXPECT_EQ(4282876139u, r1.getUInt32())
      << "check getUInt64, seed 1, second call";

  Random r2(2);
  EXPECT_EQ(1872583848u, r2.getUInt32())
      << "check getUInt64, seed 2, first call";
  EXPECT_EQ(794921487u, r2.getUInt32())
      << "check getUInt64, seed 2, second call";

  Random r3(7464235991977222558);
  EXPECT_EQ(1606095383u, r3.getUInt32())
      << "check getUInt64, big seed, first call";
  EXPECT_EQ(59943411u, r3.getUInt32())
      << "check getUInt64, big seed, second call";
}


TEST(RandomTest, getReal64) {
  // tests for getReal64
  Random r1(1);
  EXPECT_DOUBLE_EQ(0.41702199853421701, r1.getReal64());
  EXPECT_DOUBLE_EQ(0.99718480836534518, r1.getReal64());

  Random r2(2);
  EXPECT_DOUBLE_EQ(0.43599490272719293, r2.getReal64());
  EXPECT_DOUBLE_EQ(0.18508208151559394, r2.getReal64());

  Random r3(7464235991977222558);
  EXPECT_DOUBLE_EQ(0.37394822188046489, r3.getReal64());
  EXPECT_DOUBLE_EQ(0.013956662969187988, r3.getReal64());
}


TEST(RandomTest, Sampling) {
  // tests for sampling

  const vector<UInt> population = {1u, 2u, 3u, 4u};
  Random r(17);

  {
    // choose some elements
    auto  choices = r.sample<UInt>(population, 2);
    EXPECT_EQ(3u, choices[0]) << "check sample 0";
    EXPECT_EQ(2u, choices[1]) << "check sample 1";
  }

  {
    // choose all elements
    vector<UInt> choices = r.sample<UInt>(population, 4);

    EXPECT_EQ(1u, choices[0]) << "check sample 0";
    EXPECT_EQ(2u, choices[1]) << "check sample 1";
    EXPECT_EQ(4u, choices[2]) << "check sample 2";
    EXPECT_EQ(3u, choices[3]) << "check sample 3";
  }

  //check population list remained unmodified
  ASSERT_EQ(1u, population[0]) << "check sample p 0";
  ASSERT_EQ(2u, population[1]) << "check sample p 1";
  ASSERT_EQ(3u, population[2]) << "check sample p 2";
  ASSERT_EQ(4u, population[3]) << "check sample p 3";

  {
    // nChoices > nPopulation
    EXPECT_THROW(r.sample<UInt>(population, 5), Exception) << "checking for exception from population too small";
  }
}


TEST(RandomTest, Shuffling) {
  // tests for shuffling
  Random r(1);
  UInt32 arr[] = {1u, 2u, 3u, 4u};
  const UInt32 exp[] = {4u, 1u, 3u, 2u};

  ASSERT_NO_THROW(r.shuffle(std::begin(arr), std::end(arr)));

  EXPECT_EQ(exp[0], arr[0]) << "check shuffle 0";
  EXPECT_EQ(exp[1], arr[1]) << "check shuffle 1";
  EXPECT_EQ(exp[2], arr[2]) << "check shuffle 2";
  EXPECT_EQ(exp[3], arr[3]) << "check shuffle 3";
}


/**
 * Test operator '=='
 */
TEST(RandomTest, testEqualsOperator) {
  Random r1(42), r2(42), r3(3);
  ASSERT_EQ(r1, r2);
  ASSERT_NE(r1, r3);
  ASSERT_NE(r2, r3);

  UInt32 v1, v2;
  v1 = r1.getUInt32();
  ASSERT_NE(r1, r2) << "one step diff";
  v2 = r2.getUInt32();
  ASSERT_EQ(r1, r2) << "synchronized steps";
  ASSERT_EQ(v1, v2);
}


TEST(RandomTest, testGetUIntSpeed) {
 Random r1(42);
 UInt32 rnd;
 const int RUNS = 10000000;
 for(int i=0; i<RUNS; i++) {
   rnd = r1.getUInt32(10000); //get random int [0..1M)
 }
 EXPECT_EQ(rnd, 9278u);
}

} //-ns
