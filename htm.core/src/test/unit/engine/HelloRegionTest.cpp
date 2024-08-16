/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2013-2014, Numenta, Inc.
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

#include "gtest/gtest.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <htm/engine/Network.hpp>
#include <htm/engine/Region.hpp>
#include <htm/ntypes/Dimensions.hpp>
#include <htm/os/Path.hpp>

namespace testing {

using namespace htm;

static bool verbose = false;

TEST(HelloRegionTest, demo) {

  // Write test data
  if (!Path::exists("TestOutputDir"))
    Directory::create("TestOutputDir");
  std::string path = Path::join("TestOutputDir", "Data.csv");

  // enter some arbitrary test data.
  std::vector<std::vector<Real64>> testdata = {{0, 1.5f, 2.5f},
                                               {1, 1.5f, 2.5f},
                                               {2, 1.5f, 2.5f},
                                               {3, 1.5f, 2.5f}};
  size_t data_rows = testdata.size();
  size_t data_cols = testdata[0].size();
  std::ofstream f(path);
  for (auto row : testdata) {
    for (auto ele : row) {
      f << " " << ele;
    }
    f << std::endl;
  }
  f.close();


  // Create network
  Network net;


  std::string params = "{activeOutputCount: "+std::to_string(data_cols)+"}";

  // Add FileInputRegion region to network
  std::shared_ptr<Region> region =
      net.addRegion("region", "FileInputRegion", params);


  // Load data
  // This will read in all data into a vector of vectors.
  std::vector<std::string> loadFileArgs;
  loadFileArgs.push_back("loadFile");
  loadFileArgs.push_back(path);
  loadFileArgs.push_back("2"); //2=file format, space separated elements.
  region->executeCommand(loadFileArgs);

  // Initialize network
  net.initialize();

  // Compute
  net.run(1);  // This should fetch the first row into buffer

  // Get output
  const Array& outputArray = region->getOutputData("dataOut");
  EXPECT_TRUE(outputArray.getType() == NTA_BasicType_Real64);
  EXPECT_EQ(outputArray.getCount(), testdata[0].size());
  const Real64 *buffer = (const Real64 *)outputArray.getBuffer();
  for (size_t i = 0; i < outputArray.getCount(); i++)
    EXPECT_DOUBLE_EQ(buffer[i], testdata[0][i]);
  // At this point we have consumed the first buffer from FileInputRegion.

  // Serialize
  Network net2;
  {
    std::stringstream ss;
    net.save(ss, SerializableFormat::JSON);
	  if(verbose) std::cout << "Loading from stream. \n";
    if(verbose) std::cout << ss.str() << std::endl;
    ss.seekg(0);
    net2.load(ss, SerializableFormat::JSON);
  }

  // Note: this compares the structure (regions, links, etc) not data content or state.
  EXPECT_EQ(net, net2) << "Restored network should be the same as original.";

  std::shared_ptr<Region> region2 = net2.getRegion("region");
  const Array& outputArray2 = region2->getOutputData("dataOut");

  // fetch the data rows for both networks.
  net.run((int)data_rows-1);
  net2.run((int)data_rows-1);

  // The last row should be currently in the buffer
  if (verbose) std::cout << "outputArray =" << outputArray << std::endl;
  if (verbose) std::cout << "outputArray2=" << outputArray2 << std::endl;

  const Real64 *buffer2 = (const Real64 *)outputArray2.getBuffer();
  EXPECT_EQ(data_cols, outputArray.getCount());
  ASSERT_EQ(outputArray2.getCount(), outputArray.getCount());
  for (size_t i = 0; i < outputArray2.getCount(); i++) {
    EXPECT_NEAR(buffer[i], testdata[data_rows -1][i], 0.001);
	  EXPECT_NEAR(buffer[i], buffer2[i], 0.001);
	  if(verbose) std::cout << "testdata=" << testdata[data_rows -1][i]
              << ", buffer=" << buffer[i]
              << ", buffer2=" << buffer2[i] << std::endl;
  }

  // Cleanup
  Directory::removeTree("TestOutputDir", true);
}
} //end namespace
