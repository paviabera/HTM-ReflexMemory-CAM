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
 * Implementation of the ArrayBase class
 */

#include <cstdlib>  // for size_t
#include <cstring>  // for memcpy, memcmp
#include <iostream> // for ostream
#include <sstream>  // for stringstream
#include <vector>

#include <htm/ntypes/ArrayBase.hpp>
#include <htm/ntypes/Value.hpp>

#include <htm/utils/Log.hpp>

namespace htm {


/**
 * This makes a deep copy of the buffer so this class will own the buffer.
 */
ArrayBase::ArrayBase(NTA_BasicType type, const void *buffer, size_t count) {
  if (!BasicType::isValid(type)) {
    NTA_THROW << "Invalid NTA_BasicType " << type << " used in array constructor";
  }
  type_ = type;
  allocateBuffer(count);
  if (has_buffer()) {
    if (type == NTA_BasicType_Str) {
      std::string *ptr1 = reinterpret_cast<std::string *>(getBuffer());
      const std::string *ptr2 = reinterpret_cast<const std::string *>(buffer);
      for (size_t i = 0; i < count; i++) {
        ptr1[i] = ptr2[i];
      }
    } else {
      // Warning: for NTA_BasicType_Bool, if the buffer came from the internal buffer of a vector
      //          the element size is not known for sure. It might be optimized to store them as bits.
      std::memcpy(reinterpret_cast<char *>(getBuffer()), reinterpret_cast<const char *>(buffer),
                  count * BasicType::getSize(type));
    }
  }
}

/**
 * constructor for Array object containing an SDR.
 * The SDR is copied. Array is the owner of the copy.
 */
ArrayBase::ArrayBase(const SDR &sdr) {
  type_ = NTA_BasicType_SDR;
  auto dim = sdr.dimensions;
  allocateBuffer(dim);
  if (has_buffer()) {
    std::memcpy(reinterpret_cast<char *>(getBuffer()), reinterpret_cast<char *>(sdr.getDense().data()), count_);
  }
  // sdr.setDenseInplace();
}

/**
 * Caller does not provide a buffer --
 * Nupic will either provide a buffer via setBuffer or
 * ask the ArrayBase to allocate a buffer via allocateBuffer.
 */
ArrayBase::ArrayBase(NTA_BasicType type) {
  if (!BasicType::isValid(type)) {
    NTA_THROW << "Invalid NTA_BasicType " << type << " used in array constructor";
  }
  type_ = type;
  releaseBuffer();
}

/**
 * The destructor will result in the shared_ptr being deleted.
 * If this is the last reference to the pointer, and this class owns the buffer,
 * the pointer will be deleted...making sure it will not leak.
 */
ArrayBase::~ArrayBase() {}

/**
 * Ask ArrayBase to allocate its buffer.  This class owns the buffer.
 * If there was already a buffer allocated, it will be released.
 * The buffer will be deleted when the last copy of this class has been deleted.
 */
char *ArrayBase::allocateBuffer(size_t count) {
  // Note that you can allocate a buffer of size zero.
  // The C++ spec (5.3.4/7) requires such a new request to return
  // a non-NULL value which is safe to delete.  This allows us to
  // disambiguate uninitialized ArrayBases and ArrayBases initialized with
  // size zero.
  count_ = count;
  if (type_ == NTA_BasicType_SDR) {
    std::vector<UInt> dimension;
    dimension.push_back((UInt)count);
    allocateBuffer(dimension);
  } else if (type_ == NTA_BasicType_Str) {
    //Need to allocate and delete std::string such that it can initialize.
    char *s = reinterpret_cast<char *>(new std::string[count_]);
    buffer_.reset(s, StrDeleter());
  } else {
    std::shared_ptr<char> sp(new char[count_ * BasicType::getSize(type_)], std::default_delete<char[]>());
    buffer_ = sp;
  }
  return buffer_.get();
}

char *ArrayBase::allocateBuffer(const std::vector<UInt> &dimensions) { // only for SDR
  NTA_CHECK(type_ == NTA_BasicType_SDR) << "Dimensions can only be set on the SDR payload";
  SDR *sdr = new SDR(dimensions);
  std::shared_ptr<char> sp(reinterpret_cast<char *>(sdr));
  buffer_ = sp;
  count_ = sdr->size;
  return buffer_.get();
}

/**
 * Will fill the buffer with 0's.
 */
void ArrayBase::zeroBuffer() {
  if (has_buffer()) {
    if (type_ == NTA_BasicType_SDR) {
      getSDR().zero();
    } else if (type_ == NTA_BasicType_Str) {
      std::string *ptr = reinterpret_cast<std::string *>(buffer_.get());
      for (size_t i = 0; i < count_; i++)
        ptr[i] = "";
    } else
      std::memset(buffer_.get(), 0, count_ * BasicType::getSize(type_));
  }
}

/**
 * Internal function
 * Use the given pointer as the buffer.
 * The caller is responsible to delete the buffer.
 * This class will NOT own the buffer so when this class and all copies
 * of this class are deleted the buffer will NOT be deleted.
 * NOTE: A crash condition WILL exists if this class is used
 *       after the object pointed to has gone out of scope. No protections.
 * This allows external buffers to be carried in the Array structure.
 */
void ArrayBase::setBuffer(void *buffer, size_t count) {
  NTA_CHECK(type_ != NTA_BasicType_SDR);
  count_ = count;
  buffer_ = std::shared_ptr<char>(reinterpret_cast<char *>(buffer), nonDeleter());
}
void ArrayBase::setBuffer(SDR &sdr) {
  type_ = NTA_BasicType_SDR;
  buffer_ = std::shared_ptr<char>(reinterpret_cast<char *>(&sdr), nonDeleter());
  count_ = sdr.size;
}



void ArrayBase::releaseBuffer() {
  buffer_.reset();
  count_ = 0;
}

void *ArrayBase::getBuffer() {
  if (has_buffer()) {
    if (type_ == NTA_BasicType_SDR) {
      return getSDR().getDense().data();
    }
    return buffer_.get();
  }
  return nullptr;
}

const void *ArrayBase::getBuffer() const {
  if (has_buffer()) {
    if (buffer_ != nullptr && type_ == NTA_BasicType_SDR) {
      return getSDR().getDense().data();
    }
    return buffer_.get();
  }
  return nullptr;
}

SDR& ArrayBase::getSDR() {
  NTA_CHECK(type_ == NTA_BasicType_SDR) << "Does not contain an SDR object";
  if (buffer_ == nullptr) {
    std::vector<UInt> zeroDim;
    zeroDim.push_back(0u);
    allocateBuffer(zeroDim);  // Create an empty SDR object.
  }
  SDR& sdr = *(reinterpret_cast<SDR *>(buffer_.get()));
  sdr.setDense(sdr.getDense()); // cleanup cache
  return sdr;
}
const SDR& ArrayBase::getSDR() const {
  NTA_CHECK(type_ == NTA_BasicType_SDR) << "Does not contain an SDR object";
  if (buffer_ == nullptr)
    // this is const, cannot create an empty SDR.
    NTA_THROW << "getSDR: SDR pointer is null";
  SDR& sdr = *(reinterpret_cast<SDR *>(buffer_.get()));
  sdr.setDense(sdr.getDense()); // cleanup cache
  return sdr;
}

/**
 * number of elements of the given type in the buffer.
 */
size_t ArrayBase::getCount() const {
  if (has_buffer() && type_ == NTA_BasicType_SDR) {
    return (reinterpret_cast<SDR *>(buffer_.get()))->size;
  }
  return count_;
};




/**
 * Return the NTA_BasicType of the current contents.
 */
NTA_BasicType ArrayBase::getType() const { return type_; };

/**
 * Return true if a buffer has been allocated.
 */
bool ArrayBase::has_buffer() const { return (buffer_.get() != nullptr); }

/**
 * Convert the buffer contents of the current ArrayBase into
 * the type of the incoming ArrayBase 'a'. Applying an offset if specified.
 * This may be called multiple times to set values of different offsets.
 * If there is not enough room in the destination buffer a new one is created.
 * After allocating the buffer, zero it to clear zero values (if converting
 * from Sparse to Dense).
 *
 * For Fan-In condition, be sure there is enough space in the buffer before
 * the first conversion to avoid loosing data during re-allocation. Then do
 * them in order so that the largest index is last.
 *
 * Be careful when using this with destination of SDR...it will remove
 * dimensions if buffer is  not big enough.
 *
 * args:
 *    a         - Destination buffer
 *    offset    - Index used as starting index. (defaults to 0)
 *    maxsize   - Total size of destination buffer (if 0, use source size)
 *                This is used to allocate destination buffer size (in counts).
 */
void ArrayBase::convertInto(ArrayBase &a, size_t offset, size_t maxsize) const {
  if (maxsize == 0)
    maxsize = getCount() + offset;
  if (maxsize > a.getCount()) {
    a.allocateBuffer(maxsize);
    a.zeroBuffer();
  }
	// TODO:  Comment this out until we are sure that it is not needed.
  //if (offset == 0) {
  //  // This could be the first buffer of a Fan-In set.
  //  if (a.getCount() != maxsize)
  //    a.setCount(maxsize);
  //}
  NTA_CHECK(getCount() + offset <= maxsize);
  char *toPtr =  reinterpret_cast<char *>(a.getBuffer()); // char* so it has size
  if (offset)
    toPtr += (offset * BasicType::getSize(a.getType()));
  const void *fromPtr = getBuffer();
  BasicType::convertArray(toPtr, a.type_, fromPtr, type_, getCount());
}

bool ArrayBase::isInstance(const ArrayBase &a) const {
  if (a.buffer_ == nullptr || buffer_ == nullptr)
    return false;
  return (buffer_.get() == a.buffer_.get());
}


///////////////////////////////////////////////////////////////////////////////
//    Compare operators
///////////////////////////////////////////////////////////////////////////////
// Compare contents of two ArrayBase objects
// Note: An Array and an ArrayRef could be the same if type, count, and buffer
// contents are the same.
bool operator==(const ArrayBase &lhs, const ArrayBase &rhs) {
  if (lhs.getType() != rhs.getType() || lhs.getCount() != rhs.getCount())
    return false;
  if (lhs.getCount() == 0u)
    return true;
  if (lhs.getType() == NTA_BasicType_SDR) {
    return (lhs.getSDR() == rhs.getSDR());
  }
  if (lhs.getType() == NTA_BasicType_Str) {
    const std::string *ptr1 = reinterpret_cast<const std::string *>(lhs.getBuffer());
    const std::string *ptr2 = reinterpret_cast<const std::string *>(rhs.getBuffer());
    for (size_t i = 0; i < lhs.getCount(); i++) {
      if (ptr1[i] != ptr2[i])
        return false;
    }
    return true;
  }
  return (std::memcmp(lhs.getBuffer(), rhs.getBuffer(), lhs.getCount() * BasicType::getSize(lhs.getType())) == 0);
}

template<typename T>
static bool compare_array_0_and_non0s_helper_(T ptr, const Byte *v, size_t size) {
    for (size_t i = 0; i < size; i++) {
      if (((v[i]!=0) && (((T)ptr)[i] == 0)) 
       || ((v[i]==0) && (((T)ptr)[i] != 0)))
        return false;
    }
    return true;
}

// Compare contents of a ArrayBase object and a vector of type Byte.  Actually
// we are only interested in 0 and non-zero values in this compare.
static bool compare_array_0_and_non0s_(const ArrayBase &a_side, const std::vector<htm::Byte> &v_side) {
  
  if (a_side.getCount() != v_side.size()) return false;
  size_t ele_size = BasicType::getSize(a_side.getType());
  size_t size = a_side.getCount();
  const void *a_ptr = a_side.getBuffer();
  const Byte *v_ptr = &v_side[0];
  switch (ele_size) {
  default:
  case 1: return compare_array_0_and_non0s_helper_(reinterpret_cast<const Byte*  >(a_ptr), v_ptr, size);
  case 2: return compare_array_0_and_non0s_helper_(reinterpret_cast<const UInt16*>(a_ptr), v_ptr, size);
  case 4: return compare_array_0_and_non0s_helper_(reinterpret_cast<const UInt32*>(a_ptr), v_ptr, size);
  case 8: return compare_array_0_and_non0s_helper_(reinterpret_cast<const UInt64*>(a_ptr), v_ptr, size);
  }
  return true;
}
bool operator==(const ArrayBase &lhs, const std::vector<htm::Byte> &rhs) {
  return compare_array_0_and_non0s_(lhs, rhs);
}
bool operator==(const std::vector<htm::Byte> &lhs, const ArrayBase &rhs) {
  return compare_array_0_and_non0s_(rhs, lhs);
}

////////////////////////////////////////////////////////////////////////////////
//         Stream Serialization  (as Ascii text character strings)
//              [ type count ( item item item ...) ... ]
////////////////////////////////////////////////////////////////////////////////

template <typename T>
static void _templatedStreamBuffer(std::ostream &outStream, const void *inbuf,
                                   size_t numElements) {
  outStream << "( ";

  // Stream the elements
  auto it = reinterpret_cast<const T *>(inbuf);
  auto const end = it + numElements;
  if (it < end) {
    for (; it < end; ++it) {
      outStream << 0 + (*it) << " ";
    }
    // note: Adding 0 to value so Byte displays as numeric.
  }
  outStream << ") ";
}

std::string ArrayBase::toString() const {
  std::stringstream outStream;

  auto const inbuf = getBuffer();
  auto const numElements = getCount();
  auto const elementType = getType();
  if (elementType == NTA_BasicType_SDR) {
    if (!has_buffer())
      outStream << "[ SDR(0) nullptr ]";
    else
      outStream << "[ " << getSDR() << " ]";
  } else if (elementType == NTA_BasicType_Str) {
    outStream << "[ Str(" << numElements << ") ";
    const std::string *it = reinterpret_cast<const std::string *>(inbuf);
    for (size_t i = 0; i < numElements; i++) {
      outStream << "\"" << it[i] << "\" ";
    }
    outStream << "] ";
  } else {
    outStream << "[ " << BasicType::getName(elementType) << " " << numElements << " ";

    switch (elementType) {
    case NTA_BasicType_Byte:
      _templatedStreamBuffer<Byte>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int16:
      _templatedStreamBuffer<Int16>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt16:
      _templatedStreamBuffer<UInt16>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int32:
      _templatedStreamBuffer<Int32>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt32:
      _templatedStreamBuffer<UInt32>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int64:
      _templatedStreamBuffer<Int64>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt64:
      _templatedStreamBuffer<UInt64>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real32:
      _templatedStreamBuffer<Real32>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real64:
      _templatedStreamBuffer<Real64>(outStream, inbuf, numElements);
      break;
    case NTA_BasicType_Bool:
      _templatedStreamBuffer<bool>(outStream, inbuf, numElements);
      break;
    default:
      NTA_THROW << "Unexpected Element Type: " << elementType;
      break;
    }
    outStream << " ] ";
  }
  return outStream.str();
}

std::ostream &operator<<(std::ostream &outStream, const ArrayBase &a) {
  outStream << a.toString();
  return outStream;
}


template <typename T>
static void _templatedStreamBuffer(std::istream &inStream, void *buf,
                                   size_t numElements) {
  std::string v;
  inStream >> v;
  NTA_CHECK(v == "(")
      << "deserialize Array buffer...expected an opening '(' but not found.";

  // Stream the elements
  auto it = reinterpret_cast<T *>(buf);
  auto const end = it + numElements;
  if (it < end) {
    for (; it < end; ++it) {
      inStream >> *it;
    }
  }
  inStream >> v;
  NTA_CHECK(v == ")")
      << "deserialize Array buffer...expected a closing ')' but not found.";
}

std::istream &operator>>(std::istream &inStream, ArrayBase &a) {
  std::string v;
  size_t numElements;

  inStream >> v;
  NTA_CHECK(v == "[")
      << "deserialize Array object...expected an opening '[' but not found.";

  inStream >> v;
  a.type_ = BasicType::parse(v);
  inStream >> numElements;
  if (numElements > 0 && a.type_ == NTA_BasicType_SDR) {
    SDR *sdr = new SDR();
    sdr->load(inStream);
    std::shared_ptr<char> sp(reinterpret_cast<char *>(sdr));
    a.buffer_ = sp;
  } else {
    a.allocateBuffer(numElements);
  }

  if (a.has_buffer()) {
    auto inbuf = a.getBuffer();
    switch (a.type_) {
    case NTA_BasicType_Byte:
      _templatedStreamBuffer<Byte>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int16:
      _templatedStreamBuffer<Int16>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt16:
      _templatedStreamBuffer<UInt16>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int32:
      _templatedStreamBuffer<Int32>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt32:
      _templatedStreamBuffer<UInt32>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Int64:
      _templatedStreamBuffer<Int64>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_UInt64:
      _templatedStreamBuffer<UInt64>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real32:
      _templatedStreamBuffer<Real32>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Real64:
      _templatedStreamBuffer<Real64>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_Bool:
      _templatedStreamBuffer<bool>(inStream, inbuf, numElements);
      break;
    case NTA_BasicType_SDR:
      _templatedStreamBuffer<Byte>(inStream, inbuf, numElements);
      break;
    default:
      NTA_THROW << "Unexpected Element Type: " << a.type_;
      break;
    }
  }
  inStream >> v;
  NTA_CHECK(v == "]")
      << "deserialize Array buffer...expected a closing ']' but not found.";
  inStream.ignore(1);

  return inStream;
}

// a helper function to parse the type for an SDR in a JSON serialization of an Array.
//  syntax "SDR(dim1[, dim2[, dim3]])"
static std::vector<UInt> parseDim(const std::string &type) {
  std::vector<UInt> dim;
  char *end;
  const char *p1 = strchr(type.c_str(), '(');
  if (p1) {
    const char *p2 = strchr(p1, ')');
    if (p2) {
      while (true) {
        while (p1 < p2 && !isdigit(*p1))
          p1++;
        if (p1 >= p2)
          break;
        UInt i = strtoul(p1, &end, 0);
        dim.push_back(i);
        p1 = end;
      }
    }
  }
  NTA_CHECK(dim.size() > 0)
      << "In parse of Array object from JSON, SDR type does not include dimensions.  Expected SDR(nnn[,nnn[,nnn]])";
  return dim;
}

// Serialization and Deserialization using YAML parser
// For JSON, expecting something like {type: "Int32", data: [1, 0, 1]}
void ArrayBase::fromYAML(const std::string &data) { // handles both YAML and JSON
  Value vm, vm1, vm2;
  vm.parse(data);
  NTA_CHECK(vm.contains("type") && vm.contains("data"))
      << "Unexpected YAML or JSON format. Expecting something like {type: \"Real32\", data: [1,0,1]}";

  vm1 = vm["type"];
  NTA_CHECK(vm1 && vm1.isScalar())
      << "Unexpected YAML or JSON format. Expecting something like {type: \"Int32\", data: [1,0,1]}";

  vm2 = vm["data"];
  NTA_CHECK(vm2 && vm.isSequence())
      << "Unexpected YAML or JSON format. Expecting something like {type: \"SDR(1000)\", data: [1,2,3]}";

  std::string typeStr = vm1.as<std::string>();
  if (typeStr.size() >= 3 && typeStr.substr(0, 3) == "SDR") {
    type_ = NTA_BasicType_SDR;
    allocateBuffer(parseDim(typeStr));
  } else {
    type_ = BasicType::parse(typeStr);
    size_t num = vm2.size();
    allocateBuffer(num);
  }

  fromValue(vm);
}

void ArrayBase::fromValue(const Value &vm_) {
  Value vm = vm_["data"];
  size_t num = vm.size();

  if (getCount() == 0) {
    if (type_ == NTA_BasicType_SDR) {
      std::vector<UInt> dim;
      if (vm_.contains("dim")) {
        Value vm1 = vm_["dim"];
        for (size_t i = 0; i < vm1.size(); i++) {
          dim.push_back(vm1[i].as<UInt>());
        }
      } else {
        dim.push_back(static_cast<UInt>(num));
      }
      allocateBuffer(dim);
    } else {
      allocateBuffer(num);
    }
  }

  void *inbuf = getBuffer();

  switch (type_) {
  case NTA_BasicType_Byte:
    for (size_t i = 0; i < num; i++) {
      ((Byte *)inbuf)[i] = vm[i].as<Byte>();
    }
    break;
  case NTA_BasicType_Int16:
    for (size_t i = 0; i < num; i++) {
      ((Int16 *)inbuf)[i] = vm[i].as<Int16>();
    }
    break;
  case NTA_BasicType_UInt16:
    for (size_t i = 0; i < num; i++) {
      ((UInt16 *)inbuf)[i] = vm[i].as<UInt16>();
    }
    break;
  case NTA_BasicType_Int32:
    for (size_t i = 0; i < num; i++) {
      ((Int32 *)inbuf)[i] = vm[i].as<Int32>();
    }
    break;
  case NTA_BasicType_UInt32:
    for (size_t i = 0; i < num; i++) {
      ((UInt32 *)inbuf)[i] = vm[i].as<UInt32>();
    }
    break;
  case NTA_BasicType_Int64:
    for (size_t i = 0; i < num; i++) {
      ((Int64 *)inbuf)[i] = vm[i].as<Int64>();
    }
    break;
  case NTA_BasicType_UInt64:
    for (size_t i = 0; i < num; i++) {
      ((UInt64 *)inbuf)[i] = vm[i].as<UInt64>();
    }
    break;
  case NTA_BasicType_Real32:
    for (size_t i = 0; i < num; i++) {
      ((Real32 *)inbuf)[i] = vm[i].as<Real32>();
    }
    break;
  case NTA_BasicType_Real64:
    for (size_t i = 0; i < num; i++) {
      ((Real64 *)inbuf)[i] = vm[i].as<Real64>();
    }
    break;
  case NTA_BasicType_Bool:
    for (size_t i = 0; i < num; i++) {
      ((bool *)inbuf)[i] = vm[i].as<bool>();
    }
    break;
  case NTA_BasicType_SDR: //  Expecting sparse data
  {
    SDR &sdr = getSDR();
    bool isDense = false;
    if (num == getCount()) {
      if (num == 1) {
        isDense = true;
      } else if (num == 2) { // maybe problem here
        isDense = true;
      } else {
        UInt x = vm[2].as<UInt>();
        if (x <= 1) {
          isDense = true;
        }
      }
    }

    if (isDense) {
      SDR_dense_t dense;
      for (size_t i = 0; i < vm.size(); i++) {
        bool b = vm[i].as<bool>();
        Byte x = (b) ? 1u : 0u;
        dense.push_back(x);
      }
      sdr.setDense(dense);
    } else {
      SDR_sparse_t sparse;
      for (size_t i = 0; i < vm.size(); i++) {
        UInt x = vm[i].as<UInt>();
        sparse.push_back(x);
      }
      sdr.setSparse(sparse);
    }
    break;
  }
  case NTA_BasicType_Str:
    for (size_t i = 0; i < num; i++) {
      ((std::string *)inbuf)[i] = vm[i].as<std::string>();
    }
    break;
  default:
    NTA_THROW << "Unexpected Element Type: " << type_;
    break;
  }
}

std::string ArrayBase::toJSON() const {
  std::stringstream json;
  if (type_ == NTA_BasicType_SDR) {
    const SDR &sdr = getSDR();
    json << "[";
    auto sparse = sdr.getSparse();
    for (size_t i = 0; i < sparse.size(); i++) {
      int v = sparse[i];
      if (i == 0) {
        json << v;
      } else
        json << ", " << v;
    }
    json << "]";
  } else {

    json << "[";
    size_t num = getCount();
    const void *inbuf = getBuffer();
    bool first = true;
    for (size_t i = 0; i < num; i++) {
      if (first)
        first = false;
      else
        json << ", ";
      switch (type_) {
      case NTA_BasicType_Byte:
        json << (int)((Byte *)inbuf)[i];
        break;
      case NTA_BasicType_Int16:
        json << ((Int16 *)inbuf)[i];
        break;
      case NTA_BasicType_UInt16:
        json << ((UInt16 *)inbuf)[i];
        break;
      case NTA_BasicType_Int32:
        json << ((Int32 *)inbuf)[i];
        break;
      case NTA_BasicType_UInt32:
        json << ((UInt32 *)inbuf)[i];
        break;
      case NTA_BasicType_Int64:
        json << ((Int64 *)inbuf)[i];
        break;
      case NTA_BasicType_UInt64:
        json << ((UInt64 *)inbuf)[i];
        break;
      case NTA_BasicType_Real32:
        json << ((Real32 *)inbuf)[i];
        break;
      case NTA_BasicType_Real64:
        json << ((Real64 *)inbuf)[i];
        break;
      case NTA_BasicType_Bool: 
          json << ((((bool *)inbuf)[i]) ? "true" : "false");
        break;
      case NTA_BasicType_Str:
        json << Value::json_string(((std::string *)inbuf)[i]);
        break;
      default:
        NTA_THROW << "Unexpected Element Type: " << type_;
        break;
      }
    }
    json << "]";
  }

  return json.str();
}

} // namespace htm
