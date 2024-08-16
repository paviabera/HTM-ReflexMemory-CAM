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

/** @file
 * Implementation for FileInputRegion class
 *   (was VectorFileSensor )
 */

#include <cstring> // strlen
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>

#include <htm/engine/Output.hpp>
#include <htm/engine/Input.hpp>
#include <htm/engine/Region.hpp>
#include <htm/engine/Spec.hpp>
#include <htm/regions/FileInputRegion.hpp>
#include <htm/utils/Log.hpp>

using namespace std;
namespace htm {

// Helper function
UInt32 toUInt32(const std::string& s) {
  std::istringstream ss(s);
  UInt32 i;
  ss >> i;
  return i;
}

//----------------------------------------------------------------------------

FileInputRegion::FileInputRegion(const ValueMap &params, Region *region)
    : RegionImpl(region),

      iterations_(0), curVector_(-1),
      dataOut_(NTA_BasicType_Real64), categoryOut_(NTA_BasicType_Real32),
      resetOut_(NTA_BasicType_Real32), filename_(""),
      recentFile_("") {
  repeatCount_ = params.getScalarT<UInt32>("repeatCount", 1);
  activeOutputCount_ = params.getScalarT<UInt32>("activeOutputCount", 0);
  hasCategoryOut_ = params.getScalarT<UInt32>("hasCategoryOut", 0) == 1;
  hasResetOut_ = params.getScalarT<UInt32>("hasResetOut", 0) == 1;
  scalingMode_ = params.getString("scalingMode", "none");
  if (scalingMode_ != "standardForm")
    scalingMode_ = "none";

  if (params.contains("inputFile"))
    filename_ = params.getString("inputFile", "");
}

FileInputRegion::FileInputRegion(ArWrapper &wrapper, Region *region) 
    : RegionImpl(region), repeatCount_(1), iterations_(0), curVector_(-1),
      activeOutputCount_(0), hasCategoryOut_(false), hasResetOut_(false),
      dataOut_(NTA_BasicType_Real64), categoryOut_(NTA_BasicType_Real32),
      resetOut_(NTA_BasicType_Real32), filename_(""), scalingMode_("none"),
      recentFile_("") {
  cereal_adapter_load(wrapper);
}


void FileInputRegion::initialize() {
  // NOTE: this is not called after deserialize.
  NTA_CHECK(region_ != nullptr);
  dataOut_ = region_->getOutput("dataOut")->getData();

  if (activeOutputCount_ == 0)
    activeOutputCount_ = (UInt32)dataOut_.getCount();
  else if (dataOut_.getCount() != activeOutputCount_) {
    NTA_THROW << "FileInputRegion::init - wrong output size: "
              << dataOut_.getCount() << " should be: " << activeOutputCount_
              << ". Are activeOutputCount parameter and dimensions both specified?";
  }
}

FileInputRegion::~FileInputRegion() {}

//----------------------------------------------------------------------------

void FileInputRegion::compute() {
  // It's not necessarily an error to have no outputs. In this case we just
  // return
  dataOut_ = region_->getOutput("dataOut")->getData();
  if (dataOut_.getCount() == 0)
    return;

  // Don't write if there is no open file.
  if (recentFile_ == "") {
    NTA_WARN << "FileInputRegion compute() called, but there is no open file";
    return;
  }

  NTA_CHECK(vectorFile_.vectorCount() > 0)
      << "FileInputRegion::compute - no data vectors in memory."
      << "Perhaps no data file has been loaded using the 'loadFile'"
      << " execute command.";

  if (iterations_ % repeatCount_ == 0) {
    // Get index to next vector and copy scaled vector to our output
    curVector_++;
    curVector_ %= vectorFile_.vectorCount();
  }

  Real64 *out = (Real64 *)dataOut_.getBuffer();

  Size count = dataOut_.getCount();
  UInt offset = 0;

  if (hasCategoryOut_) {
    categoryOut_ = region_->getOutput("categoryOut")->getData();
    Real64 *categoryOut = reinterpret_cast<Real64 *>(categoryOut_.getBuffer());
    vectorFile_.getRawVector((htm::UInt)curVector_, categoryOut, offset, 1);
    offset++;

    // trace facility
    NTA_DEBUG << "compute " << region_->getOutput("categoryOut") << std::endl;
  }

  if (hasResetOut_) {
    resetOut_ = region_->getOutput("resetOut")->getData();
    Real64 *resetOut = reinterpret_cast<Real64 *>(resetOut_.getBuffer());
    vectorFile_.getRawVector((htm::UInt)curVector_, resetOut, offset, 1);
    offset++;

    // trace facility
    NTA_DEBUG << "compute " << *region_->getOutput("reset") << std::endl;
  }

  vectorFile_.getScaledVector((htm::UInt)curVector_, out, offset, count);

  // trace facility
  NTA_DEBUG << "compute " << *region_->getOutput("dataOut") << std::endl;
  iterations_++;
}

//--------------------------------------------------------------------------------
inline const char *checkExtensions(const std::string &filename,
                                   const char *const *extensions) {
  while (*extensions) {
    const char *ext = *extensions;
    if (filename.rfind(ext) == (filename.size() - ::strlen(ext)))
      return ext;

    ++extensions;
  }
  return nullptr;
}

//--------------------------------------------------------------------------------
/// Execute a FileInputRegion specific command
std::string FileInputRegion::executeCommand(const std::vector<std::string> &args, Int64 index)

{
  UInt32 argCount = (UInt32)args.size();
  // Get the first argument (command string)
  NTA_CHECK(argCount > 0) << "FileInputRegion: No command name";
  string command = args[0];

  // Process each command
  if ((command == "loadFile") || (command == "appendFile")) {
    NTA_CHECK(argCount > 1)
        << "FileInputRegion: no filename specified for " << command;

    UInt32 labeled = 2; // Default format is 2

    // string filename = ReadStringFromBuffer(*buf2);
    string filename(args[1]);
    cout << "In FileInputRegion " << filename << endl;

    if (argCount == 3) {
      labeled = toUInt32(args[2]);
    } else {
      // Check for some common extensions.
      const char *csvExtensions[] = {".csv", ".CSV", nullptr};
      if (checkExtensions(filename, csvExtensions)) {
        cout << "Reading CSV file" << endl;
        labeled = 3; // CSV format.
      }
    }

    // Detect binary file format and set labeled flag to read little endian
    // binary file
    if (filename.substr(filename.size() - 3, 3) == "bin") {
      cout << "Reading binary file" << endl;
      labeled = 4;
    }

    if (labeled > (UInt32)VectorFile::maxFormat())
      NTA_THROW << "FileInputRegion: unknown file format '" << labeled << "'";

    // Read in new set of vectors
    // If the command is loadFile, we clear the list first and reset the
    // position to the beginning
    if (command == "loadFile")
      vectorFile_.clear(false);

    if (activeOutputCount_ == 0) { 
      // vector width not specified so use the region dimensions.
      activeOutputCount_ = static_cast<UInt32>(dim_.getCount());
    }


    // Timer t(true);

    UInt32 elementCount = activeOutputCount_;
    if (hasCategoryOut_)
      elementCount++;
    if (hasResetOut_)
      elementCount++;

    vectorFile_.appendFile(filename, elementCount, labeled);
    cout << "Read " << vectorFile_.vectorCount() << " vectors" << endl;
    // << "  in " << t.getValue() << " seconds" << endl;

    vectorFile_.resetScaling(activeOutputCount_); // clear scaling
    if (scalingMode_ == "standardForm")
      vectorFile_.setStandardScaling();

    if (command == "loadFile")
      seek(0);


    recentFile_ = filename;
  }

  else if (command == "dump") {
    std::string message;
    message = "FileInputRegion isLabeled = " + to_string(vectorFile_.isLabeled())
      + ", repeatCount = " + to_string(repeatCount_)
      + ", vectorWidth = " + to_string(vectorFile_.getElementCount())
      + ", vectorCount = " + to_string(vectorFile_.vectorCount())
      + ", iterations = " + to_string(iterations_)
      + "\n";
    return message;
  }

  else if (command == "saveFile") {
    NTA_CHECK(argCount > 1)
        << "FileInputRegion: no filename specified for " << command;

    Int32 format = 2; // Default format is 2
    Int64 begin = 0, end = 0;
    bool hasEnd = false;

    string filename(args[1]);

    if (argCount > 2) {
      format = toUInt32(args[2]);
      if ((format < 0) || (format > VectorFile::maxFormat()))
        NTA_THROW << "VectorFileSensor: unknown file format '" << format << "'";
    }

    if (argCount > 3) {
      begin = toUInt32(args[3]);
    }

    if (argCount > 4) {
      end = toUInt32(args[4]);
      hasEnd = true;
    }

    NTA_CHECK(argCount <= 5) << "FileInputRegion: too many arguments";

    std::ofstream f(filename.c_str());
    if (!hasEnd)
      end = vectorFile_.vectorCount() - 1;
    vectorFile_.saveVectors(f, dataOut_.getCount(), format, begin, end);
  }

  else {
    NTA_THROW << "FileInputRegion: Unknown execute command: '" << command
              << "' sent!";
  }

  return "";
}

//----------------------------------------------------------------------
/**
 * Position to a vector within the vector file.
 * A negative number for n will position n from the end (wrapping backward). 
 */
void FileInputRegion::seek(int n) {
  // Set curVector_ to be one before the vector we want and reset iterations
  // On the next compute() it will advance to the vector we want and output it.
  iterations_ = 0;
  curVector_ = n - 1;

  // circular-buffer, reached one end of vector/line, wrap around.
  if (curVector_ < 0) // make n >= 0.
    curVector_ += static_cast<int>(vectorFile_.vectorCount()); 
  NTA_ASSERT(curVector_ >= 0);
  curVector_ %= vectorFile_.vectorCount();
}

size_t FileInputRegion::getNodeOutputElementCount(const std::string &outputName) const {
  NTA_CHECK(outputName == "dataOut") << "Invalid output name: " << outputName;
  return activeOutputCount_;
}


Spec *FileInputRegion::createSpec() {
  auto ns = new Spec;
  ns->name = "FileInputRegion";
  ns->description =
      "FileInputRegion is a basic sensor for reading files containing "
      "vectors.\n"
      "\n"
      "FileInputRegion reads in a text file containing lists of numbers\n"
      "and outputs these vectors in sequence. The output is updated\n"
      "each time the sensor's compute() method is called. If\n"
      "repeatCount is > 1, then each vector is repeated that many times\n"
      "before moving to the next one. The sensor loops when the end of\n"
      "the vector list is reached. The default file format\n"
      "is as follows (assuming the sensor is configured with N outputs):\n"
      "\n"
      "  e11 e12 e13 ... e1N\n"
      "  e21 e22 e23 ... e2N\n"
      "    : \n"
      "  eM1 eM2 eM3 ... eMN\n"
      "\n"
      "In this format the sensor ignores all whitespace in the file, including "
      "newlines\n"
      "If the file contains an incorrect number of floats, the sensor has no "
      "way\n"
      "of checking and will silently ignore the extra numbers at the end of "
      "the file.\n"
      "\n"
      "The sensor can also read in comma-separated (CSV) files following the "
      "format:\n"
      "\n"
      "  e11, e12, e13, ... ,e1N\n"
      "  e21, e22, e23, ... ,e2N\n"
      "    : \n"
      "  eM1, eM2, eM3, ... ,eMN\n"
      "\n"
      "When reading CSV files the sensor expects that each line contains a new "
      "vector\n"
      "Any line containing too few elements or any text will be ignored. If "
      "there are\n"
      "more than N numbers on a line, the sensor retains only the first N.\n";

  ns->outputs.add("dataOut",
                  OutputSpec("Data read from file", NTA_BasicType_Real64,
                             0,    // count
                             true, // isRegionLevel
                             true  // isDefaultOutput
                             ));

  ns->outputs.add( "categoryOut",
				          OutputSpec("The current category encoded as a float (represent a whole number)",
					                 NTA_BasicType_Real32,
					                 1,    // count
					                 false, // isRegionLevel
					                 false // isDefaultOutput
					                 ));

  ns->outputs.add("resetOut",
                  OutputSpec("Sequence reset signal: 0 - do nothing, otherwise "
                             "start a new sequence",
                             NTA_BasicType_Real32,
                             1,    // count
                             false, // isRegionLevel
                             false // isDefaultOutput
                             ));

  ns->parameters.add( "vectorCount",
			      ParameterSpec("The number of vectors currently loaded in memory.",
			                 NTA_BasicType_UInt32,
			                 1,                    // elementCount
			                 "interval: [0, ...]", // constraints
			                 "0",                  // defaultValue
			                 ParameterSpec::ReadOnlyAccess));

  ns->parameters.add("position",
                   ParameterSpec("Set or get the current position within the "
                             "list of vectors in memory. Index of vector last output. "
                             "Before anything is output, position is -1. Setting a position "
                             "will position just prior to the requested vector so that "
                             "the requested value will be the next output from the next "
                             "call to compute(). If the requested position is outside the "
                             "range of the vector set, it will wrap to be inside the range.",
                             NTA_BasicType_Int32,
                             1,                    // elementCount
                             "interval: [0, ...]", // constraints
                             "-1",                  // defaultValue (before first value).
                             ParameterSpec::ReadWriteAccess));

  ns->parameters.add( "repeatCount",
			      ParameterSpec(
					          "Set or get the current repeatCount. Each vector is repeated\n"
					          "repeatCount times before moving to the next one.",
					          NTA_BasicType_UInt32,
					          1,                    // elementCount
					          "interval: [1, ...]", // constraints
					          "1",                  // defaultValue
					          ParameterSpec::ReadWriteAccess));

  ns->parameters.add("recentFile",
                   ParameterSpec("Writes output vectors to this file on each "
                                   "compute. Will append to any\n"
                                   "existing data in the file. This parameter "
                                   "must be set at runtime before\n"
                                   "the first compute is called. Throws an "
                                   "exception if it is not set or\n"
                                   "the file cannot be written to.\n",
                               NTA_BasicType_Byte,
                               0,  // elementCount
                               "", // constraints
                               "", // defaultValue
                               ParameterSpec::ReadOnlyAccess));

  ns->parameters.add( "scalingMode",
			       ParameterSpec("During compute, each vector is adjusted as follows. If X is the "
						          "data vector,\n"
						          "S the scaling vector and O the offset vector, then the node's output\n"
						          "                Y[i] = S[i]*(X[i] + O[i]).\n"
						          "\n"
						          "Scaling is applied according to scalingMode as follows:\n"
						          "\n"
						          "    If 'none', the vectors are unchanged, i.e. S[i]=1 and O[i]=0.\n"
						          "    If 'standardForm', S[i] is 1/standard deviation(i) and O[i] = - mean(i)\n"
						          "    If 'custom', each component is adjusted according to the \n"
						          "       vectors specified by the setScale and setOffset commands.\n",
					            NTA_BasicType_Byte,
					            0,      // elementCount
					            "",     // constraints
					            "none", // defaultValue
					            ParameterSpec::ReadWriteAccess));

  ns->parameters.add( "scaleVector",
				   ParameterSpec("Set or return the current scale vector S.\n",
				                 NTA_BasicType_Real32,
				                 0,  // elementCount
				                 "", // constraints
				                 "", // defaultValue
				                 ParameterSpec::ReadWriteAccess));

  ns->parameters.add( "offsetVector",
			       ParameterSpec("Set or return the current offset vector 0.\n",
			                     NTA_BasicType_Real32,
			                     0,  // elementCount
			                     "", // constraints
			                     "", // defaultValue
			                     ParameterSpec::ReadWriteAccess));

  ns->parameters.add("activeOutputCount",
                   ParameterSpec("The number of active outputs of the node.",
                                 NTA_BasicType_UInt32,
                                 1,                    // elementCount
                                 "interval: [0, ...]", // constraints
                                 "",                   // default Value
                                 ParameterSpec::CreateAccess));

  ns->parameters.add( "maxOutputVectorCount",
			       ParameterSpec( "The number of output vectors that can be generated by this sensor\n"
						          "under the current configuration.",
						          NTA_BasicType_UInt32,
						          1,                    // elementCount
						          "interval: [0, ...]", // constraints
						          "0",                  // defaultValue
						          ParameterSpec::ReadOnlyAccess));

  ns->parameters.add("hasCategoryOut",
                   ParameterSpec( "Category info is present in data file.",
                                  NTA_BasicType_UInt32,
                                  1,              // elementCount
                                  "enum: [0, 1]", // constraints
                                  "0",            // defaultValue
                                  ParameterSpec::ReadWriteAccess));

  ns->parameters.add( "hasResetOut",
				   ParameterSpec("New sequence reset signal is present in data file.",
				                 NTA_BasicType_UInt32,
				                 1,              // elementCount
				                 "enum: [0, 1]", // constraints
				                 "0",            // defaultValue
				                 ParameterSpec::ReadWriteAccess));

  ns->commands.add( "loadFile",
      CommandSpec(
        "loadFile <filename> [file_format]\n"
        "Reads vectors from the specified file, replacing any vectors\n"
        "currently in the list. Position is set to zero. \n"
        "Available file formats are: \n"
        " 0 - Reads in unlabeled file with first number = element count\n"
        " 1 - Reads in a labeled file with first number = element count (deprecated)\n"
        " 2 - Reads in unlabeled file without element count (default)\n"
        " 3 - Reads in a csv file\n"));

  ns->commands.add( "appendFile",
      CommandSpec(
        "appendFile <filename> [file_format]\n"
        "Reads vectors from the specified file, appending to current vector list.\n"
        "Position remains unchanged. Available file formats are: \n"
        " 0 - Reads in unlabeled file with first number = element count\n"
        " 1 - Reads in a labeled file with first number = element count (deprecated)\n"
        " 2 - Reads in unlabeled file without element count (default)\n"
        " 3 - Reads in a csv file\n"));

  ns->commands.add( "saveFile",
       CommandSpec("saveFile filename [format [begin [end]]]\n"
                              "Save the currently loaded vectors to a file. "
                              "Typically used for debugging\n"
                              "but may be used to convert between formats.\n"));

  ns->commands.add("dump",
       CommandSpec("Displays some debugging info."));

  return ns;
}


//--------------------------------------------------------------------------------
UInt32 FileInputRegion::getParameterUInt32(const std::string &name, Int64 index) const {
  if (name == "vectorCount") {
    return (UInt32)vectorFile_.vectorCount();
  } else if (name == "repeatCount") {
    return repeatCount_;
  } else if (name == "activeOutputCount") {
    return activeOutputCount_;
  } else if (name == "maxOutputVectorCount") {
    return (UInt32)vectorFile_.vectorCount() * repeatCount_;
  } else if (name == "hasCategoryOut") {
    return (UInt32)hasCategoryOut_;
  } else if (name == "hasResetOut") {
    return (UInt32)hasResetOut_;
  } else {
    return RegionImpl::getParameterUInt32(name, index);
  }
}

Int32 FileInputRegion::getParameterInt32(const std::string &name, Int64 index) const {
  if (name == "position") {
    if (vectorFile_.vectorCount() == 0) return -1;
    return curVector_;
  } else {
    return RegionImpl::getParameterInt32(name, index);
  }
}
std::string FileInputRegion::getParameterString(const std::string &name, Int64 index) const {
  if (name == "scalingMode") {
    return scalingMode_;
  } else if (name == "recentFile") {
    return recentFile_;
  } else {
    return RegionImpl::getParameterString(name, index);
  }
}


void FileInputRegion::getParameterArray(const std::string &name, Int64 index, Array &a) const {
  if (a.getCount() != dataOut_.getCount())
    NTA_THROW << "getParameterArray(), array size is: " << a.getCount()
              << "instead of : " << dataOut_.getCount();

  Real64 *buf = (Real64 *)a.getBuffer();
  Real64 dummy;
  if (name == "scaleVector") {
    Real64 *buf = (Real64 *)a.getBuffer();
    for (UInt i = 0; i < vectorFile_.getElementCount(); i++) {
      vectorFile_.getScaling(i, buf[i], dummy);
    }
  } else if (name == "offsetVector") {

    for (UInt i = 0; i < vectorFile_.getElementCount(); i++) {
      vectorFile_.getScaling(i, dummy, buf[i]);
    }
  } else {
    NTA_THROW << "FileInputRegion::getParameterArray(), unknown parameter: "
              << name;
  }
}

//--------------------------------------------------------------------------------
void FileInputRegion::setParameterUInt32(const std::string &name, Int64 index, UInt32 value) {
  const char *where = "setParameterUInt32() FileInputRegion, parameter ";

  if (name == "repeatCount") {
    NTA_CHECK(value > 0)
        << where << "repeatCount: " << value
        << " - Should be a positive non-zero integer";

    repeatCount_ = value;
  }
  else if (name == "hasCategoryOut") {
    hasCategoryOut_ = (value == 1);
  }
  else if (name == "hasResetOut") {
    hasResetOut_ = (value == 1);
  } else {
	RegionImpl::setParameterUInt32(name, index, value);
  }
}

void FileInputRegion::setParameterInt32(const std::string &name, Int64 index, Int32 value) {
  const char *where = "setParameterInt32() FileInputRegion, parameter ";
  if (name == "position") {
    if (vectorFile_.vectorCount() == 0) return; // not yet initialized.
    NTA_CHECK(value >= 0 && value < static_cast<Int32>(vectorFile_.vectorCount()))
      << where << "'position'." << " Requested position is out of range. [ 0 to "
      << (vectorFile_.vectorCount() - 1) << "]";
    seek(value);
  } else {
	  RegionImpl::setParameterInt32(name, index, value);
  }
}

void FileInputRegion::setParameterString(const std::string &name, Int64 index, const std::string &value) {
  const char *where = "setParameterString() FileInputRegion ";
  if (name == "scalingMode") {
    if (value == "none")
      vectorFile_.resetScaling();
    else if (value == "standardForm")
      vectorFile_.setStandardScaling();
    else if (value != "custom") // Do nothing if set to custom
      NTA_THROW << where << " Unknown scaling mode: " << value;
    scalingMode_ = value;
  } else {
	  RegionImpl::setParameterString(name, index, value);
  }
}



void FileInputRegion::setParameterArray(const std::string &name, Int64 index,
                                         const Array &a) {
  NTA_CHECK (a.getCount() == dataOut_.getCount())
         << "setParameterArray(), array size is: " << a.getCount()
         << "instead of : " << dataOut_.getCount();

  Real *buf = (Real *)a.getBuffer();
  if (name == "scaleVector") {
    for (UInt i = 0; i < vectorFile_.getElementCount(); i++) {
      vectorFile_.setScale(i, buf[i]);
    }
  } else if (name == "offsetVector") {

    for (UInt i = 0; i < vectorFile_.getElementCount(); i++) {
      vectorFile_.setOffset(i, buf[i]);
    }
  } else {
    NTA_THROW << "FileInputRegion::setParameterArray(), unknown parameter: " << name;
  }

  scalingMode_ = "custom";
}

size_t FileInputRegion::getParameterArrayCount(const std::string &name, Int64 index) const {
  NTA_CHECK (name == "scaleVector" || name == "offsetVector")
     << "FileInputRegion::getParameterArrayCount(), unknown array parameter: "<< name;
  return dataOut_.getCount();
}



bool FileInputRegion::operator==(const RegionImpl &o) const {
  if (o.getType() != "FileInputRegion") return false;
  FileInputRegion &other = (FileInputRegion &)o;
  if (repeatCount_ != other.repeatCount_) return false;
  if (activeOutputCount_ != other.activeOutputCount_) return false;
  if (curVector_ != other.curVector_) return false;
  if (iterations_ != other.iterations_) return false;
  if (hasCategoryOut_ != other.hasCategoryOut_) return false;
  if (hasResetOut_ != other.hasResetOut_) return false;
  if (filename_ != other.filename_) return false;
  if (scalingMode_ != other.scalingMode_) return false;
  if (recentFile_ != other.recentFile_) return false;

  return true;
}

} // end namespace htm
