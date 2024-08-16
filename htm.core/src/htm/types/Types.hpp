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
 * Basic C++ type definitions used throughout `htm.core`
 */
#include <cstdlib>  // defines size_t
#include <cstdint>  // defines int of specific sizes.

#ifndef NTA_TYPES_HPP
#define NTA_TYPES_HPP


#define UNUSED(x) (void)(x)

//----------------------------------------------------------------------

namespace htm {
/**
 * @name Basic types
 *  see https://en.cppreference.com/w/cpp/language/types
 *
 * @{
 */

/**
 * Represents a signed 8-bit byte.
 * (note that std::byte is unsigned char.)
 */
#ifdef Byte
#undef Byte
#endif
typedef char Byte;

/**
 * Represents a 16-bit signed integer.
 */
typedef short Int16;

/**
 * Represents a 16-bit unsigned integer.
 */
typedef unsigned short UInt16;

/**
 * Represents a 32-bit signed integer.
 */
typedef int32_t Int32;

/**
 * Represents a 32-bit unsigned integer.
 */
typedef uint32_t UInt32;

/**
 * Represents a 64-bit signed integer.
 */
typedef int64_t Int64;

/**
 * Represents a 64-bit unsigned integer.
 */
typedef uint64_t UInt64;

/**
 * Represents a 32-bit real number(a floating-point number).
 */
typedef float Real32;

/**
 * Represents a 64-bit real number(a floating-point number).
 */
typedef double Real64;

/**
 * Represents an opaque handle/pointer
 */
typedef void* Handle;


/**
 * Represents lengths of arrays, strings and so on.
 */
typedef std::size_t Size;

/**
 * @}
 */

/**
 * @name Flexible types
 *
 * The following are flexible types depending on `NTA_DOUBLE_PRECISION` and
 * `NTA_BIG_INTEGER`.
 *
 * @{
 *
 */

/**
 * Represents a real number(a floating-point number).
 *
 * Same as htm::Real64 if `NTA_DOUBLE_PRECISION` is defined, htm::Real32
 * otherwise.
 */
#ifdef NTA_DOUBLE_PRECISION
  typedef Real64 Real;
#else
  typedef Real32 Real;
#endif


/**
 * Represents a signed integer.
 *
 * Same as htm::Int64 if `NTA_BIG_INTEGER` is defined, htm::Int32 otherwise.
 */
#ifdef NTA_BIG_INTEGER
  typedef Int64 Int;
#else
  typedef Int32 Int;
#endif

/**
 * Represents a unsigned integer.
 *
 * Same as htm::UInt64 if `NTA_BIG_INTEGER` is defined, htm::UInt32
 * otherwise.
 */
#ifdef NTA_BIG_INTEGER
  typedef UInt64 UInt;
#else
  typedef UInt32 UInt;
#endif


/**
 * @}
 */

 /**
 * Basic types enumeration
 */
typedef enum NTA_BasicType {
  /**
   * Represents a 8-bit byte.
   */
  NTA_BasicType_Byte,

  /**
   * Represents a 16-bit signed integer.
   */
  NTA_BasicType_Int16,

  /**
   * Represents a 16-bit unsigned integer.
   */
  NTA_BasicType_UInt16,

  /**
   * Represents a 32-bit signed integer.
   */
  NTA_BasicType_Int32,

  /**
   * Represents a 32-bit unsigned integer.
   */
  NTA_BasicType_UInt32,

  /**
   * Represents a 64-bit signed integer.
   */
  NTA_BasicType_Int64,

  /**
   * Represents a 64-bit unsigned integer.
   */
  NTA_BasicType_UInt64,

  /**
   * Represents a 32-bit real number(a floating-point number).
   */
  NTA_BasicType_Real32,

  /**
   * Represents a 64-bit real number(a floating-point number).
   */
  NTA_BasicType_Real64,

  /**
   * Represents a opaque handle/pointer, same as `void *`
   */
  NTA_BasicType_Handle,

  /**
   * Represents a boolean. The size is compiler-defined.
   *
   * There is no typedef'd "Bool" or "NTA_Bool". We just need a way to refer
   * to bools with a NTA_BasicType.
   */
  NTA_BasicType_Bool,

  /**
   * Represents an SDR object as pyload.  Not an array.
   * See SDR.hpp
   */
  NTA_BasicType_SDR,

  /**
    * Represents a std::string object.
    */
  NTA_BasicType_Str,

  /**
   * @note This is not an actual type, just a marker for validation purposes
   */
  NTA_BasicType_Last,

  /**
   * Represents a default-sized unsigned integer.
   */
#ifdef NTA_BIG_INTEGER
  NTA_BasicType_UInt = NTA_BasicType_UInt64,
#else
  NTA_BasicType_UInt = NTA_BasicType_UInt32,
#endif

  /**
   * Represents a default-sized real number(a floating-point number).
   */
#ifdef NTA_DOUBLE_PRECISION
  NTA_BasicType_Real = NTA_BasicType_Real64,
#else
  NTA_BasicType_Real = NTA_BasicType_Real32,
#endif

} NTA_BasicType;


} // end namespace htm

#endif // NTA_TYPES_HPP
