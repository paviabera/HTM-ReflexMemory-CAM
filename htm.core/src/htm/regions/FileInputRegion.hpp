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
 * Declarations for FileInputRegion class
 *   (was VectorFileSensor)
 */

//----------------------------------------------------------------------

#ifndef NTA_FILE_INPUT_REGION_HPP
#define NTA_FILE_INPUT_REGION_HPP

//----------------------------------------------------------------------

#include <vector>

#include <htm/engine/RegionImpl.hpp>
#include <htm/ntypes/Array.hpp>
#include <htm/regions/VectorFile.hpp>
#include <htm/types/Types.hpp>
#include <htm/types/Serializable.hpp>
#include <htm/ntypes/Value.hpp>

namespace htm {

/**
 *  FileInputRegion is a sensor that reads in files containing lists of
 *  vectors and outputs these vectors in sequence.
 *
 *  @b Description
 *
 *  Three input file formats are supported:
 *      0 - unlabeled files with element count
 *      1 - labeled files with element count
 *      2 - unlabeled files without element count (default)
 *
 *  These input formats are described in more detail below.
 *
 *  The Sensor implements the execute() commands as specified in the nodeSpec.
 *
 *  @b Notes:
 *  The file format for an unlabeled file without element count is as follows:
 *  \verbatim
          e11 e12 e13 ... e1N
          e21 e22 e23 ... e2N
             :
          eM1 eM2 eM3 ... eMN
    \endverbatim
 *
 *  The file format for an unlabeled file with element count is as follows:
 *  \verbatim
          N
          e11 e12 e13 ... e1N
          e21 e22 e23 ... e2N
             :
          eM1 eM2 eM3 ... eMN
    \endverbatim
 *
 *  The format for a labeled file with element count is as follows:
 *  \verbatim
       N
             EL1 EL2 EL3     ELN
       VL1   e11 e12 e13 ... e1N
       VL2   e21 e22 e23 ... e2N
             :
       VLM   eM1 eM2 eM3 ... eMN
    \endverbatim
 *
 *  where ELi are string labels for each element in the vector and VLi are
 *  string labels for each vector. Strings are separated by whitespace. Strings
 *  with whitespace are not supported (e.g. no quoting of strings).
 *
 *  Whitespace between numbers is ignored.
 *  The full list of vectors is read into memory when the loadFile command
 *  is executed.
 *
 */

class FileInputRegion : public RegionImpl, Serializable {
public:
  //------ Static methods for plug-in API ------------------------------------

  //    static const NTA_Spec * getSpec(const NTA_Byte * nodeType)
  //    {
  //      const char *description =
  //"FileInputRegion is a basic sensor for reading files containing vectors.\n"
  //"\n"
  //"FileInputRegion reads in a text file containing lists of numbers\n"
  //"and outputs these vectors in sequence. The output is updated\n"
  //"each time the sensor's compute() method is called. If\n"
  //"repeatCount is > 1, then each vector is repeated that many times\n"
  //"before moving to the next one. The sensor loops when the end of\n"
  //"the vector list is reached. The default file format\n"
  //"is as follows (assuming the sensor is configured with N outputs):\n"
  //"\n"
  //"  e11 e12 e13 ... e1N\n"
  //"  e21 e22 e23 ... e2N\n"
  //"    : \n"
  //"  eM1 eM2 eM3 ... eMN\n"
  //"\n"
  //"In this format the sensor ignores all whitespace in the file, including
  // newlines\n" "If the file contains an incorrect number of floats, the sensor
  // has no way\n" "of checking and will silently ignore the extra numbers at
  // the end of the file.\n"
  //"\n"
  //"The sensor can also read in comma-separated (CSV) files following the
  // format:\n"
  //"\n"
  //"  e11, e12, e13, ... ,e1N\n"
  //"  e21, e22, e23, ... ,e2N\n"
  //"    : \n"
  //"  eM1, eM2, eM3, ... ,eMN\n"
  //"\n"
  //"When reading CSV files the sensor expects that each line contains a new
  // vector\n" "Any line containing too few elements or any text will be
  // ignored. If there are\n" "more than N numbers on a line, the sensor retains
  // only the first N.\n"
  //;
  //
  //
  //      Spec* spec = new Spec("FileInputRegion", description, 0 /* flags */);
  //
  //      // ------ OUTPUTS
  //      nsb.addOutput("dataOut", "real", "This is FileInputRegion's only
  //      output. "
  //                    "It will be set to the next vector after each
  //                    compute.");
  //
  //      // ------ COMMANDS
  //      nsb.addCommand("loadFile",
  //                     "loadFile <filename> [file_format]\n"
  //                     "Reads vectors from the specified file, replacing any
  //                     vectors\n" "currently in the list. Position is set to
  //                     zero. \n" "Available file formats are: \n" "       0 #
  //                     Reads in unlabeled file with first number = element
  //                     count\n" "       1        # Reads in a labeled file
  //                     with first number = element count (deprecated)\n" " 2
  //                     # Reads in unlabeled file without element count
  //                     (default)\n" "       3        # Reads in a csv
  //                     file\n");
  //
  //      nsb.addCommand("appendFile",
  //             "appendFile <filename> [file_format]\n"
  //             "Reads vectors from the specified file, appending to current
  //             vector list.\n" "Position remains unchanged. Available file
  //             formats are: \n" "       0        # Reads in unlabeled file
  //             with first number = element count\n" "       1        # Reads
  //             in a labeled file with first number = element count
  //             (deprecated)\n" "       2        # Reads in unlabeled file
  //             without element count (default)\n" "       3        # Reads in
  //             a csv file\n");
  //
  //
  //      nsb.addCommand("dump", "Displays some debugging info.");
  //
  //      nsb.addCommand("saveFile",
  //        "saveFile filename [format [begin [end]]]\n"
  //        "Save the currently loaded vectors to a file. Typically used for
  //        debugging\n" "but may be used to convert between formats.\n");
  //
  //      // ------ PARAMETERS
  //
  //      nsb.addParameter("vectorCount",
  //                       "uint32",
  //                       "The number of vectors currently loaded in memory.",
  //                       1, /* elementCount */
  //                       "get",
  //                       "interval: [0, ...]",
  //                       "0" /* defaultValue */);
  //
  //      nsb.addParameter("position",
  //                       "int32",
  //                       "Set or get the current position within the list of
  //                       vectors in memory. Index of last output.  Will wrap.", 
  //                       1, /* elementCount */ 
  //                       "getset",
  //                       "interval: [0, ...]",
  //                       "0" /* defaultValue */);
  //
  //      nsb.addParameter("repeatCount",
  //                       "uint32",
  //                       "Set or get the current repeatCount. Each vector is
  //                       repeated\n" "repeatCount times before moving to the
  //                       next one.", 1, /* elementCount */ "getset",
  //                       "interval: [1, ...]",
  //                       "1" /* defaultValue */);
  //
  //      nsb.addParameter("recentFile",
  //                       "byteptr",
  //        "Name of the most recently file that is loaded or appended. Mostly
  //        to \n"
  //                       "support interactive use.\n",
  //                       1, /* elementCount */
  //                       "get");
  //
  //
  //      nsb.addParameter("scalingMode",
  //                       "byteptr",
  //        "During compute, each vector is adjusted as follows. If X is the
  //        data vector,\n" "S the scaling vector and O the offset vector, then
  //        the node's output\n" "                Y[i] = S[i]*(X[i] + O[i]).\n"
  //        "\n"
  //        "Scaling is applied according to scalingMode as follows:\n"
  //        "\n"
  //        "    If 'none', the vectors are unchanged, i.e. S[i]=1 and
  //        O[i]=0.\n" "    If 'standardForm', S[i] is 1/standard deviation(i)
  //        and O[i] = - mean(i)\n" "    If 'custom', each component is adjusted
  //        according to the vectors specified by the\n"
  //                       "setScale and setOffset commands.\n",
  //                       1, /* elementCount */
  //                       "all", /* access */
  //                       "", /* constraints */
  //                       "none" /* defaultValue */);
  //
  //      nsb.addParameter("scaleVector",
  //                       "real",
  //                       "Set or return the current scale vector S.\n",
  //                       0, /* elementCount */
  //                       "all",  /* access */
  //                       "", /* constraints */
  //                       "" /* defaultValue */);
  //
  //      nsb.addParameter("offsetVector",
  //                       "real",
  //                       "Set or return the current offset vector 0.\n",
  //                       0, /* elementCount */
  //                       "all", /* access */
  //                       "", /* constraints */
  //                       "" /* defaultValue */);
  //
  //
  //      nsb.addParameter("activeOutputCount",
  //                       "uint32",
  //                       "The number of active outputs of the node.",
  //                       1, /* elementCount */
  //                       "get", /* access */
  //                       "interval: [0, ...]");
  //
  //
  //      nsb.addParameter("maxOutputVectorCount",
  //                       "uint32",
  //        "The number of output vectors that can be generated by this
  //        sensor\n"
  //                       "under the current configuration.",
  //                       1, /* elementCount */
  //                       "get",
  //                       "interval: [0, ...]");
  //
  //
  //
  //    }

  static Spec *createSpec();
  size_t getNodeOutputElementCount(const std::string &outputName) const override;

  virtual UInt32 getParameterUInt32(const std::string &name, Int64 index = -1) const override;
  virtual Int32 getParameterInt32(const std::string &name, Int64 index = -1) const override;
  virtual std::string getParameterString(const std::string &name, Int64 index = -1) const override;
  virtual void getParameterArray(const std::string &name, Int64 index, Array &array) const override;
  virtual size_t getParameterArrayCount(const std::string &name, Int64 index) const override;

  virtual void setParameterUInt32(const std::string &name, Int64 index, UInt32 value) override;
  virtual void setParameterInt32(const std::string &name, Int64 index, Int32 value) override;
  virtual void setParameterString(const std::string &name, Int64 index, const std::string& value) override;
  virtual void setParameterArray(const std::string &name, Int64 index,const Array &array) override;

  void initialize() override;

  FileInputRegion(const ValueMap &params, Region *region);

  FileInputRegion(ArWrapper &wrapper, Region *region);

  virtual ~FileInputRegion();


	CerealAdapter;  // see Serializable.hpp
  // FOR Cereal Serialization
  template<class Archive>
  void save_ar(Archive& ar) const {
    ar(cereal::make_nvp("repeatCount_", repeatCount_));
    ar(cereal::make_nvp("iterations_", iterations_));
    ar(cereal::make_nvp("activeOutputCount_", activeOutputCount_));
    ar(cereal::make_nvp("curVector_", curVector_));
    ar(cereal::make_nvp("hasCategoryOut_", hasCategoryOut_));
    ar(cereal::make_nvp("hasResetOut_", hasResetOut_));
    ar(cereal::make_nvp("filename_", filename_));
    ar(cereal::make_nvp("scalingMode_", scalingMode_));
    ar(cereal::make_nvp("recentFile_", recentFile_));
    ar(cereal::make_nvp("vectorFile", vectorFile_));
  }

  // FOR Cereal Deserialization
  template<class Archive>
  void load_ar(Archive& ar) {
    ar(cereal::make_nvp("repeatCount_", repeatCount_));
    ar(cereal::make_nvp("iterations_", iterations_));
    ar(cereal::make_nvp("activeOutputCount_", activeOutputCount_));
    ar(cereal::make_nvp("curVector_", curVector_));
    ar(cereal::make_nvp("hasCategoryOut_", hasCategoryOut_));
    ar(cereal::make_nvp("hasResetOut_", hasResetOut_));
    ar(cereal::make_nvp("filename_", filename_));
    ar(cereal::make_nvp("scalingMode_", scalingMode_));
    ar(cereal::make_nvp("recentFile_", recentFile_));
    ar(cereal::make_nvp("vectorFile", vectorFile_));
  }	


  bool operator==(const RegionImpl &other) const override;
  inline bool operator!=(const FileInputRegion &other) const {
    return !operator==(other);
  }

  void compute() override;
  virtual std::string executeCommand(const std::vector<std::string> &args,
                                     Int64 index) override;

private:
  void closeFile();
  void openFile(const std::string &filename);

private:
  UInt32 repeatCount_; // Repeat count for output vectors
  UInt32 iterations_;  // Number of times compute() has been called
  int curVector_;      // The index of the vector that was just output
  UInt32 activeOutputCount_; // The number of elements in each input vector
  bool hasCategoryOut_;          // determine if a category output is needed
  bool hasResetOut_;             // determine if a reset output is needed
  htm::VectorFile vectorFile_; // Container class for the vectors

  // These are the instanceses of the Output buffers
  // They contain buffers shared with the Output objects.
  Array dataOut_;
  Array categoryOut_;
  Array resetOut_;
  std::string filename_; // Name of the output file

  std::string scalingMode_;
  std::string recentFile_; // The most recently loaded or appended file

  //------------------- Utility routines and debugging support

  // Seek to the n'th vector in the list. n should be between 0 and
  // numVectors-1. Logs a warning if n is outside those bounds.
  void seek(int n);

}; // end class FileInputRegion

//----------------------------------------------------------------------

} // end namespace htm

#endif // NTA_FILE_INPUT_REGION_HPP
