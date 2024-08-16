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

#ifndef NTA_TESTNODE_HPP
#define NTA_TESTNODE_HPP

#include <string>
#include <vector>


#include <htm/engine/RegionImpl.hpp>
#include <htm/ntypes/Value.hpp>

namespace htm {

/*
 * TestNode is does simple computations of inputs->outputs
 * inputs and outputs are Real64 arrays
 *
 * delta is a parameter used for the computation. defaults to 1
 *
 * Size of each node output is given by the outputSize parameter (cg)
 * which defaults to 2 and cannot be less than 1. (parameter not yet
 implemented)
 *
 * Here is the totally lame "computation"
 * output[0] = number of inputs to this baby node + current iteration number (0
 for first compute)
 * output[1] = baby node num + sum of inputs to this baby node
 * output[2] = baby node num + sum of inputs + (delta)
 * output[3] = baby node num + sum of inputs + (2*delta)
 * ...
 * output[n] = baby node num + sum of inputs + ((n-1) * delta)

 * It can act as a sensor if no inputs are connected (sum of inputs = 0)
 */

class BundleIO;

class TestNode : public RegionImpl, Serializable {
public:
  typedef void (*computeCallbackFunc)(const std::string &);
  TestNode(const ValueMap &params, Region *region);
  TestNode(ArWrapper& wrapper, Region *region);
  virtual ~TestNode();

  /* -----------  Required RegionImpl Interface methods ------- */

  // Used by RegionImplFactory to create and cache
  // a nodespec. Ownership is transferred to the caller.
  static Spec *createSpec();

  std::string getNodeType() { return "TestNode"; };
  void compute() override;
  std::string executeCommand(const std::vector<std::string> &args,
                             Int64 index) override;

  size_t getNodeOutputElementCount(const std::string &name) const override;

  void initialize() override;

  CerealAdapter;  // see Serializable.hpp
  // FOR Cereal Serialization
  template<class Archive>
  void save_ar(Archive& ar) const {
    ar(CEREAL_NVP(nodeCount_),
       CEREAL_NVP(int32Param_),
       CEREAL_NVP(uint32Param_),
       CEREAL_NVP(int64Param_),
       CEREAL_NVP(uint64Param_),
       CEREAL_NVP(real32Param_),
       CEREAL_NVP(real64Param_),
       CEREAL_NVP(boolParam_),
       CEREAL_NVP(stringParam_),
       CEREAL_NVP(outputElementCount_),
       CEREAL_NVP(delta_),
       CEREAL_NVP(iter_));
    ar(CEREAL_NVP(dim_));

    ar(CEREAL_NVP(real32ArrayParam_),
       CEREAL_NVP(int64ArrayParam_),
       CEREAL_NVP(boolArrayParam_),
       CEREAL_NVP(unclonedParam_),
       CEREAL_NVP(shouldCloneParam_),
       CEREAL_NVP(possiblyUnclonedParam_),
       CEREAL_NVP(unclonedInt64ArrayParam_));
  }
  // FOR Cereal Deserialization
  // NOTE: the Region Implementation must have been allocated
  //       using the RegionImplFactory so that it is connected
  //       to the Network and Region objects. This will populate
  //       the region_ field in the Base class.
  template<class Archive>
  void load_ar(Archive& ar) {
    ar(CEREAL_NVP(nodeCount_),
       CEREAL_NVP(int32Param_),
       CEREAL_NVP(uint32Param_),
       CEREAL_NVP(int64Param_),
       CEREAL_NVP(uint64Param_),
       CEREAL_NVP(real32Param_),
       CEREAL_NVP(real64Param_),
       CEREAL_NVP(boolParam_),
       CEREAL_NVP(stringParam_),
       CEREAL_NVP(outputElementCount_),
       CEREAL_NVP(delta_),
       CEREAL_NVP(iter_));
    ar(CEREAL_NVP(dim_));  // in base class

    ar(CEREAL_NVP(real32ArrayParam_),
       CEREAL_NVP(int64ArrayParam_),
       CEREAL_NVP(boolArrayParam_),
       CEREAL_NVP(unclonedParam_),
       CEREAL_NVP(shouldCloneParam_),
       CEREAL_NVP(possiblyUnclonedParam_),
       CEREAL_NVP(unclonedInt64ArrayParam_));
  }

  bool operator==(const RegionImpl &other) const override;
  inline bool operator!=(const TestNode &other) const {
    return !operator==(other);
  }


  /* -----------  Optional RegionImpl Interface methods ------- */

  size_t getParameterArrayCount(const std::string &name, Int64 index) const override;

  virtual Int32 getParameterInt32(const std::string &name, Int64 index) const override;
  virtual UInt32 getParameterUInt32(const std::string &name, Int64 index) const override;
  virtual Int64 getParameterInt64(const std::string &name, Int64 index) const override;
  virtual UInt64 getParameterUInt64(const std::string &name, Int64 index) const override;
  virtual Real32 getParameterReal32(const std::string &name, Int64 index) const override;
  virtual Real64 getParameterReal64(const std::string &name, Int64 index) const override;
  virtual bool getParameterBool(const std::string &name, Int64 index) const override;
  virtual std::string getParameterString(const std::string &name, Int64 index) const override;
  virtual void getParameterArray(const std::string &name, Int64 index, Array &array) const override;

  virtual void setParameterInt32(const std::string &name, Int64 index, Int32 value) override;
  virtual void setParameterUInt32(const std::string &name, Int64 index, UInt32 value) override;
  virtual void setParameterInt64(const std::string &name, Int64 index, Int64 value) override;
  virtual void setParameterUInt64(const std::string &name, Int64 index, UInt64 value) override;
  virtual void setParameterReal32(const std::string &name, Int64 index, Real32 value) override;
  virtual void setParameterReal64(const std::string &name, Int64 index, Real64 value) override;
  virtual void setParameterBool(const std::string &name, Int64 index, bool value) override;
  virtual void setParameterString(const std::string &name, Int64 index, const std::string &s) override;
  virtual void setParameterArray(const std::string &name, Int64 index, const Array &array) override;



private:
  TestNode() = delete;

  // parameters
  // cgs parameters for parameter testing
  Int32 int32Param_;
  UInt32 uint32Param_;
  Int64 int64Param_;
  UInt64 uint64Param_;
  Real32 real32Param_;
  Real64 real64Param_;
  bool boolParam_;
  std::string stringParam_;
  computeCallbackFunc computeCallback_;

  std::vector<Real32> real32ArrayParam_;
  std::vector<Int64> int64ArrayParam_;
  std::vector<bool> boolArrayParam_;

  // read-only count of iterations since initialization
  UInt64 iter_;

  // Constructor param specifying per-node output size
  UInt32 outputElementCount_;
  Dimensions dim_;

  // parameter used for computation
  Int64 delta_;

  // cloning parameters
  std::vector<UInt32> unclonedParam_;
  bool shouldCloneParam_;
  std::vector<UInt32> possiblyUnclonedParam_;
  std::vector<std::vector<Int64>> unclonedInt64ArrayParam_;

  /* ----- cached info from region ----- */
  size_t nodeCount_;

  // Input/output buffers for the whole region
  std::shared_ptr<Input> bottomUpIn_;   // required
  std::shared_ptr<Output> bottomUpOut_; // required
};
} // namespace htm

#endif // NTA_TESTNODE_HPP
