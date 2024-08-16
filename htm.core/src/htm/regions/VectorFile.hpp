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
 * Simple class for reading and processing data files
 */

//----------------------------------------------------------------------

#ifndef NTA_VECTOR_FILE_HPP
#define NTA_VECTOR_FILE_HPP

//----------------------------------------------------------------------

#include <fstream>
#include <sstream>
#include <htm/types/Types.hpp>
#include <htm/types/Serializable.hpp>
#include <vector>

namespace htm {
/**
 *  VectorFile is a simple container class for lists of numerical vectors. Its
 * only purpose is to support the needs of the FileInputRegion. Key features of
 *  interest are its ability to read in different text file formats and its
 *  ability to dynamically scale its outputs.
 */
class VectorFile : public Serializable {
public:
  VectorFile();
  virtual ~VectorFile();

  static Int32 maxFormat() { return 6; }

  /// Read in vectors from the given filename. All vectors are expected to
  /// have the same size (i.e. same number of elements).
  /// If a list already exists, new vectors are expected to have the same size
  /// and will be appended to the end of the list
  /// appendFile will NOT change the scaling vectors as long as the
  /// expectedElementCount is the same as previously stored vectors. The
  /// The fileFormat number corresponds to the file formats in FileInputRegion:
  ///           0        # Reads in unlabeled file with first number = element count
  ///           1        # Reads in a labeled file with first number = element count
  ///           2        # Reads in unlabeled file without element count
  ///           3        # Reads in a csv file
  ///           4        # Reads in a little-endian float32 binary file
  ///           5        # Reads in a big-endian float32 binary file
  ///           6        # Reads in a big-endian IDX binary file
  void appendFile(const std::string &fileName, Size expectedElementCount,
                  UInt32 fileFormat);

  /// Retrieve i'th vector, apply scaling and copy result into output
  /// output must have size of at least 'count' elements
  void getScaledVector(const UInt i, Real64 *out, UInt offset, Size count);

  /// Retrieve the i'th vector and copy into output without scaling
  /// output must have size at least 'count' elements
  void getRawVector(const UInt i, Real64 *out, UInt offset, Size count);

  /// Return the number of stored vectors
  size_t vectorCount() const { return fileVectors_.size(); }

  /// Return the size of each vector (number of elements per vector)
  size_t getElementCount() const;

  /// Set the scale and offset vectors to correspond to standard form
  /// Sets the offset component of each element to be -mean
  /// Sets the scale component of each element to be 1/stddev
  void setStandardScaling();

  /// Reset scaling to have no effect (unitary scaling vector and zero offset
  /// vector) If nElements > 0, also resize the scaling vector to have that many
  /// elements, otherwise leave it as-is
  void resetScaling(UInt nElements = 0);

  /// Get the scaling and offset values for element e
  void getScaling(const UInt e, Real64 &scale, Real64 &offset) const;

  /// Set the scale value for element e
  void setScale(const UInt e, const Real64 scale);

  /// Set the offset value for element e
  void setOffset(const UInt e, const Real64 offset);

  /// Clear the set of vectors and labels, including scale and offset vectors,
  /// release all memory, and set numElements back to zero.
  void clear(bool clearScaling = true);

  // Return true iff a labeled file was read in
  inline bool isLabeled() const {
    return (!(elementLabels_.empty() || vectorLabels_.empty()));
  }

  /// Save the scale and offset vectors to this stream
  void saveState(std::ostream &str) const;

  /// Initialize the scaling and offset vectors from this stream
  /// If vectorCount() > 0, it is an error if numElements()
  /// does not match the data in the stream
  void readState(std::istream &state);

  /// Save vectors, unscaled, to a file with the specified format.
  void saveVectors(std::ostream &out, Size nColumns, UInt32 fileFormat,
                   Int64 begin, Int64 end, const char *lineEndings = nullptr) const;

	CerealAdapter;  // See Serializable.hpp
  template<class Archive>
	void save_ar(Archive& ar) const { 
	  UInt32 format = (isLabeled())?1:2;     // format (1 if labled, 2 if not)
		size_t nRows = fileVectors_.size();
		Size nCols = scaleVector_.size();
		std::stringstream ss;
	  saveVectors(ss, nCols, format, 0, fileVectors_.size(), nullptr);
		std::string data = ss.str();
    ar(cereal::make_nvp("format", format),
		   cereal::make_nvp("nRows", nRows),
			 cereal::make_nvp("nCols", nCols),
		   cereal::make_nvp("scaleVector", scaleVector_),
		   cereal::make_nvp("offsetVector", offsetVector_),
			 cereal::make_nvp("data", data));
	}
  template<class Archive>
	void load_ar(Archive& ar) { 
	  UInt32 format;     // format (1 if labled, 2 if not)
		size_t nRows;
		size_t nCols;
		std::string data;
    ar( format, nRows,  nCols, scaleVector_, offsetVector_, data);
		std::stringstream ss(data);
	  loadVectors(ss, nRows, nCols, format);
	}
	

private:
  std::vector<Real64 *> fileVectors_; // list of vectors
  std::vector<bool> own_;           // memory ownership flags
  std::vector<Real64> scaleVector_;   // the scaling vector
  std::vector<Real64> offsetVector_;  // the offset vector

  std::vector<std::string> elementLabels_; // string denoting the meaning of each element
  std::vector<std::string> vectorLabels_; // a string label for each vector

  //------------------- Utility routines
  void appendCSVFile(std::istream &inFile, Size expectedElementCount);

  /// Read vectors from a binary file.
  void appendFloat64File(const std::string &filename, Size expectedElements);

  /// Read vectors from a binary IDX file.
  void appendIDXFile(const std::string &filename, int expectedElements);
  void loadVectors(std::istream &f, size_t nRows, size_t nCols, int format);
}; // end class VectorFile

//----------------------------------------------------------------------

} // namespace htm

#endif // NTA_VECTOR_FILE_HPP
