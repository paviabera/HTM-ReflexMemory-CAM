/* ---------------------------------------------------------------------
 * HTM Community Edition of NuPIC
 * Copyright (C) 2018, Numenta, Inc.
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
 *
 * Author: @chhenning, 2018
 * --------------------------------------------------------------------- */

/** @file
Definition of the PyBindRegion class.  The base class for all Python Region implementations.
*/

#ifndef NTA_PYBIND_REGION_HPP
#define NTA_PYBIND_REGION_HPP


#include <bindings/suppress_register.hpp>  //include before pybind11.h
#include <pybind11/pybind11.h>

#include <htm/types/Types.hpp>
#include <htm/engine/RegionImpl.hpp>
#include <htm/engine/Spec.hpp>
#include <htm/ntypes/Value.hpp>

namespace htm
{
    class PyBindRegion : public RegionImpl, Serializable
    {
        typedef std::map<std::string, Spec> SpecMap;

    public:
        // Used by RegionImplFactory via RegisterRegionImplPy  to create and cache a nodespec
        static void createSpec(const char * module, Spec& ns, const char* classname = "");

		// called by .py code  on an already instantiated instance.
        const Spec & getSpec() { return nodeSpec_; }

        // Used by RegisterRegionImplPy to destroy a node spec when clearing its cache
        static void destroySpec(const char * nodeType, const char* className = "");

        // Constructors
        PyBindRegion() = delete;
        PyBindRegion(const char* module, const ValueMap& nodeParams, Region *region, const char* className);
        PyBindRegion(const char* module, ArWrapper& wrapper, Region *region, const char* className);

        // no copy constructor
        PyBindRegion(const Region &) = delete;

        // Destructor
        virtual ~PyBindRegion();


			  CerealAdapter;  // see Serializable.hpp
			  // FOR Cereal Serialization
			  template<class Archive>
			  void save_ar(Archive& ar) const {
				    std::string p = pickleSerialize();
						std::string e = extraSerialize();
						ar(p, e);
				}
			  template<class Archive>
			  void load_ar(Archive& ar) {
				    std::string p;
						std::string e;
						ar(p, e);
						pickleDeserialize(p);
						extraDeserialize(e);
				}

		    bool operator==(const RegionImpl &other) const override {
					NTA_THROW << " ==  not implemented yet for PyBindRegion.";
				}
		    inline bool operator!=(const PyBindRegion &other) const {
		      return !operator==(other);
		    }
				// TODO: implement compare of two .py implemented Regions.


        ////////////////////////////
        // RegionImpl
        ////////////////////////////

        size_t getNodeOutputElementCount(const std::string& outputName) const override;

        void initialize() override;
        void compute() override;
        std::string executeCommand(const std::vector<std::string>& args, Int64 index) override;

        size_t getParameterArrayCount(const std::string& name, Int64 index) const override;

        virtual Byte getParameterByte(const std::string& name, Int64 index) const override;
        virtual Int32 getParameterInt32(const std::string& name, Int64 index) const override;
        virtual UInt32 getParameterUInt32(const std::string& name, Int64 index) const override;
        virtual Int64 getParameterInt64(const std::string& name, Int64 index) const override;
        virtual UInt64 getParameterUInt64(const std::string& name, Int64 index) const override;
        virtual Real32 getParameterReal32(const std::string& name, Int64 index) const override;
        virtual Real64 getParameterReal64(const std::string& name, Int64 index) const override;
        virtual bool getParameterBool(const std::string& name, Int64 index) const override;
        virtual std::string getParameterString(const std::string& name, Int64 index) const override;

        virtual void setParameterByte(const std::string& name, Int64 index, Byte value) override;
        virtual void setParameterInt32(const std::string& name, Int64 index, Int32 value) override;
        virtual void setParameterUInt32(const std::string& name, Int64 index, UInt32 value) override;
        virtual void setParameterInt64(const std::string& name, Int64 index, Int64 value) override;
        virtual void setParameterUInt64(const std::string& name, Int64 index, UInt64 value) override;
        virtual void setParameterReal32(const std::string& name, Int64 index, Real32 value) override;
        virtual void setParameterReal64(const std::string& name, Int64 index, Real64 value) override;
        virtual void setParameterBool(const std::string& name, Int64 index, bool value) override;
        virtual void setParameterString(const std::string& name, Int64 index, const std::string& value) override;

        virtual void getParameterArray(const std::string& name, Int64 index, Array & array) const override;
        virtual void setParameterArray(const std::string& name, Int64 index, const Array & array) override;

        // Helper methods
        template <typename T>
        T getParameterT(const std::string & name, Int64 index) const;

        template <typename T>
        void setParameterT(const std::string & name, Int64 index, T value);


    private:
        std::string module_;     // Full path to the class.
        std::string className_;  // Just the name of the class.

        pybind11::object node_;

        static std::string last_error;

        Spec nodeSpec_;   // locally cached version of spec.

        std::string pickleSerialize() const;
        std::string extraSerialize() const;
				void pickleDeserialize(std::string p);
				void extraDeserialize(std::string e);
   };



} // namespace htm

#endif //NTA_PYBIND_REGION_HPP
