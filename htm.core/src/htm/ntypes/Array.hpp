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

// ---
//
// Definitions for the Array class
//
// It is a sub-class of ArrayBase that owns its buffer.
// This object is a container for most data sets that get passed around between
// regions. This container can hold an array of any of the NTA_BasicType types.
// It does not use templates so the getBuffer() function which returns a void*
// pointer will have to cast to the appropreate type.
//
// ASSIGNMENT:
// If an Array object is assigned, it makes a new instance of itself but does
// NOT copy the buffer. Both instances of Array point to the same buffer.  Only
// when all instances of the original object are deleted will the buffer be
// deleted.  (uses smart pointers, std::shared_ptr())
//
// Array B = A;
//
// Returning an Array from a function will also make an assignment (the Array
// inside the function is deleted when it went out of scope).
//
// BUFFER CHANGES:
// The buffer in an Array can be modified so if any instance of the Array
// changes the buffer then all instances see the changes. The buffer is modified
// using the getBuffer() function.
//    NTA_BasicType type = A.getType();
//    size_t count = A.getCount();
//    void *ptr = A.getBuffer();    Then change the contents of the array
//    pointed to by ptr.
//
//    A.zeroBuffer();               This will fill the buffer with 0's.
//
// RELEASING BUFFER:
// Note that re-allocating the buffer or releasing the buffer will disconnect it
// from other Array object instances. Those instances will continue to see the
// old buffer. The old buffer will be deleted only when all instances have
// released the buffer or were deleted.
//    A.allocateBuffer(count);      A new buffer is created (old one released if
//    there was one)
//                                  Then use getBuffer() to fetch the pointer
//                                  and populate the buffer.
//
//    A.releaseBuffer();            The buffer is released for this instance
//    only.
//
//    delete A;                     The buffer is release for this instance but
//    other instances unaffected.
//
// FULL COPY OF BUFFER
// You can make a FULL or deep copy of the buffer either using the type 2
// constructor for Array or the copy() function.
//     Array B(A.getType(), A.getBuffer(), A.getCount());
//      (note if A is SDR, dimensions will get lost)
// or
//     Array B = A.copy();
// This full copy will not be affected by changes to the original buffer.
//
// TYPE CONVERSION:
// You can convert a buffer using the convertInto() or as() functions
// The as() function will allocate a new buffer convert this buffer into another
// type. The convertInto() function trys to keep the same output buffer and just
// converts into the existing buffer. However, if the existing buffer is too
// small it will reallocate the buffer.
//
// POPULATING:
//   constructor A(type)              - Empty buffer of specified type.
//   constructor A(type, buff, size)  - Buffer filled from C array
//   constructor A(sdr)               - Buffer initialized from SDR
//   constructor A(vector)            - Buffer initialized from vector
//   B = A;                           - Shallow copy, B has same, type, buffer, size
//   B = A.copy()                     - Creates B as a deep copy of A.
//   A.populate(vector)               - fills A from vector, with conversion, A retains type.
//
//
// ACCESSING:
//   vector = A.asVector()            - creates vector and populates it from A
//   B = A.get_as(type)               - creates B with conversion, to specified type.
//   A.convertInto(B, offset, maxsize)- fills B with conversion at offset, B retains type.
//   B = A.subset(offset, length)     - fills B with subset of A, same type.
//
//
// FROM ArrayBase:
// A.AllocateBuffer(count)           -- single dimension buffer allocation
// A.AllocateBuffer(dimensions)      -- multi dimensional buffer allocation (for SDR)
// A.getBuffer()                     -- returns a void* pointer to beginning of buffer.
// A.setBuffer(ptr, count)           -- set un-owned buffer
// A.setBuffer(sdr)                  -- set un-owned SDR
// A.zeroBuffer()                    -- fills A with 0's, A retains type and size.
// A.releaseBuffer()                 -- free everything (if owned)
// A.getSDR()                        -- get reference to enclosed SDR
// A.getBufferSize( )                -- size of buffer in bytes
// A.getMaxElementsCount()           -- capacity in number of elements 
// A.setCount(count)                 -- truncate buffer size to this length, keeping capacity.   (not SDR)
// A.getType()                       -- return NTA_BasicType of buffer
// A.has_buffer()                    -- returns true if a buffer has been allocated.
// A.is_instance(B)                  -- returns true if B is shared with A
// A.RefreshCache()                  -- tells SDR to update cache
// A.toJSON()                        -- returns a JSON serialization containing contents of A.
// A.toYAML()                        -- returns a YAML serialization containing contents of A.
// A.save(stream)                    -- serialize
// A.load(stream)                    -- deserialize
// cout << A << std::endl;           -- stream out
// cin  >> A;                        -- stream in
// A == B                            -- equals overload
// A == v                            -- equals a vector
// A != B                            -- not equals overload
//
// SERIALIZATION
// Three serialization methods are supported.
//
// The simplest is a std::istream/std::ostream interface.
//
//      Array A;
//        ... populate A
//      std::ofstream &f = bundle.getOutputStream("Region");
//      f << A;      // serializes A
// or
//      std::ifstream &f = bundle.getInputStream("Region");
//      Array A(type);
//      f >> A;      // deserializes A
//
//  The second serialization method is YAML serialization using Cereal.
//      Array A;
//        ...A gets populated
//      A.save(ofstream &f, SerializableFormat::JSON);
//
//      Array A;
//      A.load(ifstream &f, SerializableFormat::JSON);
//
//  The third serialization method is YAML or JSON serialization using our YAML parser
//
//     Array A;
//        ...A gets populated
//     A.toYAML();
//    or
//     A.toJSON();
//
//     Array A;
//     A.fromYAML();    handles both YAML and JSON
// ---

#ifndef NTA_ARRAY_HPP
#define NTA_ARRAY_HPP

#include <cstring>

#include <htm/ntypes/ArrayBase.hpp>

#include <htm/ntypes/BasicType.hpp>
#include <htm/ntypes/Dimensions.hpp>
#include <htm/utils/Log.hpp>
#include <htm/types/Serializable.hpp>
#include <htm/types/Sdr.hpp>

namespace htm {
class Array : public ArrayBase {
public:
  /**
   * default constructor.
   */
  Array() : ArrayBase(NTA_BasicType_Int32) {}

  /**
   * constructor for empty Array with a known type.
   */
  Array(NTA_BasicType type) : ArrayBase(type) {}

  /**
   * Initialize by copying in from a raw C-type buffer.  See ArrayBase
   */
  Array(NTA_BasicType type, const void *buffer, size_t count)
      : ArrayBase(type, buffer, count) {}

  /**
   * Create an ArrayBase containing a copy of an SDR.
   */
  Array(const SDR &sdr) : ArrayBase(sdr) {}

  /**
   * Create an ArrayBase with type and data from vector
   *  -- The templated constructor did not seem to work so
   *  defining explicitly.
   */
  template <typename T> 
  Array(const std::vector<T> &vect) 
      : ArrayBase(BasicType::getType<T>()) {
    allocateBuffer(vect.size());
    if (has_buffer()) {
      // iterate the elements in the vector so we don't need to worry about the size of it's internal elements.
      T *ptr = (T *)getBuffer();
      for (size_t i = 0; i < count_; i++) {
        ptr[i] = vect[i];
      }
    }

  }

  // copy constructor
  // by default, a copy constructor is a shallow copy which would result in
  // two Array objects pointing to the same buffer. However, this is stored
  // in a shared smart pointer so both Array objects must be deleted before
  // the buffer is deleted. So the default copy and assignment constructor
  // is ok.
  // Array(const Array &other) : ArrayBase(other)
  // Array& operator=(const Array& other)

  /////////////////////////////////////
  //   Copy tools

  /**
   * There are times when we do want a deep copy.  The copy() function
   * will make a full copy of the buffer and becomes the buffer owner.
   */
  Array copy() const {
    Array a(type_);
    if (getCount() > 0) {
      if (type_ == NTA_BasicType_SDR) {
        a.allocateBuffer(getSDR().dimensions);
        // for an SDR, bulk copy the buffer inside the SDR
        std::memcpy(static_cast<char *>(a.getBuffer()), static_cast<const char *>(getBuffer()),
                    getCount() * BasicType::getSize(type_));
      }
      else if (type_ == NTA_BasicType_Str) {
        a.allocateBuffer(getCount());
        // String values must be copied one at a time.
        const std::string *ptr1 = static_cast<const std::string *>(getBuffer());
        std::string *ptr2 = static_cast<std::string *>(a.getBuffer());
        for (size_t i = 0; i < getCount(); i++) {
          ptr2[i] = ptr1[i];
        }
      } else {
        a.allocateBuffer(getCount());
        // numeric values can be bulk copied
        std::memcpy(static_cast<char *>(a.getBuffer()), static_cast<const char *>(getBuffer()),
                    getCount() * BasicType::getSize(type_));
      }
   }
    return a;
  }

  /**
   * Convert to a vector; copies buffer, With conversion
   * example: vector<Int32> v = array.asVector<Int32>();
   */
  template <typename T> 
  std::vector<T> asVector() const {
    std::vector<T> v;
    NTA_BasicType to_type = BasicType::getType<T>();
    if (has_buffer()) {
      v.resize(count_);
      BasicType::convertArray(v.data(), to_type, 
                              getBuffer(), getType(), getCount());
    }
    return v;
  }


  /**
    * Type Conversion
    * This populates our Array from a templeted vector.
    * The buffer is copied and a coversion is provided if
    * vector element types are not the same as the Array type.
    * The vector is assumed to contain only dense format data.
    */
  template <typename T> void populate(const std::vector<T> &v) {
    NTA_CHECK(typeid(T) != typeid(SDR)) << "A vector<SDR> not allowed.";
    NTA_BasicType fromType = BasicType::getType<T>();
    allocateBuffer(v.size());
    if (getCount() > 0) {
      if (getType() == NTA_BasicType_SDR) {
        getSDR().setDense(v);
        RefreshCache(); 
      } else {
        BasicType::convertArray(getBuffer(), getType(), v.data(), fromType,v.size());
      }
    }
  }

  /**
    * Type conversion
    * This will do a full copy, converting to target type,
    * and return the new Array.
    */
  Array get_as(NTA_BasicType type) const {
    Array a(type);
    a.allocateBuffer(getCount());
    a.zeroBuffer();
    convertInto(a);
    return a;
  }

  template <typename T> inline T item(size_t index) const {
    NTA_CHECK(index < count_) << "index out of range.";
    NTA_CHECK(BasicType::getType<T>() == type_) << "incorrect type. Expecting " << BasicType::getName(type_);
    const T *buf = static_cast<const T *>(getBuffer());
    return buf[index];
  }


  /**
    * Type conversion
    * This will do a full copy into a shared buffer.
    * Note: this will attempt to reuse the same buffer
    *       so that shared instances will not be disconnected.
    *       However, if buffer is too small, it will be reallocated.
    */
  void convertInto(Array & a, size_t offset = 0, size_t maxsize = 0) const {
    ArrayBase::convertInto(a, offset, maxsize);
  }

  /**
    * Copy a subset
    * This creates a new Array of the same type but contains a subset
    * range of values from this Array.
    */
  Array subset(size_t offset, size_t count) const {
    NTA_CHECK(getCount() >= count + offset)
        << "Requested subset out of range; "
        << " offset(" << offset << ")+ count=(" << count
        << ") is larger than array size (" << getCount() << ").";
    NTA_CHECK(type_ != NTA_BasicType_SDR) << "subset() not valid for SDR";
    Array a(type_);
    a.allocateBuffer(count);
    if (type_ == NTA_BasicType_Str) {
      // String values must be copied one at a time.
      const std::string *ptr1 = static_cast<const std::string *>(getBuffer());
      std::string *ptr2 = static_cast<std::string *>(a.getBuffer());
      for (size_t i = 0; i < count; i++) {
        ptr2[i] = ptr1[i + offset];
      }
    } else {
      // bulk copy all numeric arrays
      std::memcpy(a.getBuffer(), static_cast<const char *>(getBuffer()) + offset * BasicType::getSize(type_),
                  count * BasicType::getSize(type_));
    }
    return a;
  }
};
} // namespace htm

CEREAL_REGISTER_TYPE(htm::Array)
CEREAL_REGISTER_POLYMORPHIC_RELATION(htm::ArrayBase, htm::Array)



#endif
