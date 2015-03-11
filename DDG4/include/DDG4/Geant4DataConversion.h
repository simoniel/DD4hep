// $Id: Geant4Field.cpp 875 2013-11-04 16:15:14Z markus.frank@cern.ch $
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================
#ifndef DD4HEP_DDG4_GEANT4CONVERSION_H
#define DD4HEP_DDG4_GEANT4CONVERSION_H

// Framework include files
#include "DD4hep/VolumeManager.h"
#include "DD4hep/Detector.h"
#include <typeinfo>

/// Namespace for the AIDA detector description toolkit
namespace DD4hep {

  /// Namespace for the Geant4 based simulation part of the AIDA detector description toolkit
  namespace Simulation {

    /// Helper class for data conversion
    /**
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_SIMULATION
     */
    class Geant4ConversionHelper {
    public:
      /// Default constructor
      Geant4ConversionHelper();
      /// Default destructor
      virtual ~Geant4ConversionHelper();
      /// Access to the data encoding using the volume manager and a specified volume id
      static std::string encoding(Geometry::VolumeManager vm, Geometry::VolumeManager::VolumeID vid);
      /// Access to the hit encoding in this sensitive detector
      static std::string encoding(Geometry::Handle<Geometry::SensitiveDetectorObject> sd);
      /// Access to the hit encoding in this readout object
      static std::string encoding(Geometry::Readout ro);
    };

    /// Data conversion class
    /**
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_SIMULATION
     */
    template <typename OUTPUT, typename ARGS> class Geant4Conversion : public Geant4ConversionHelper {
    public:
      typedef ARGS arg_t;
      typedef OUTPUT output_t;
      typedef Geant4Conversion<output_t, arg_t> self_t;
      typedef std::map<const std::type_info*, Geant4Conversion*> Converters;
      static Converters& conversions();
      static const Geant4Conversion& converter(const std::type_info& typ);
      /// Default constructor
      Geant4Conversion();
      /// Default destructor
      virtual ~Geant4Conversion();
      virtual OUTPUT* operator()(const ARGS& args) const = 0;
    };
  }    // End namespace Simulation
}      // End namespace DD4hep

#if defined(DDG4_MAKE_INSTANTIATIONS)
#include <stdexcept>

/*
 *   DD4hep namespace declaration
 */
namespace DD4hep {

  /*
   *   Simulation namespace declaration
   */
  namespace Simulation {

    template <typename OUTPUT,typename ARGS>
      Geant4Conversion<OUTPUT,ARGS>::Geant4Conversion()
      : Geant4ConversionHelper()
      {
      }

    template <typename OUTPUT,typename ARGS>
      Geant4Conversion<OUTPUT,ARGS>::~Geant4Conversion()
      {
      }

    template <typename OUTPUT,typename ARGS>
      typename Geant4Conversion<OUTPUT,ARGS>::Converters&
      Geant4Conversion<OUTPUT,ARGS>::conversions()
      {
        static Converters s_converter;
        return s_converter;
      }

    template <typename OUTPUT, typename ARGS>
      const Geant4Conversion<OUTPUT,ARGS>&
      Geant4Conversion<OUTPUT,ARGS>::converter(const std::type_info& typ)
      {
        typename Converters::const_iterator i = conversions().find(&typ);
        if ( i != conversions().end() ) {
          return *((*i).second);
        }
        throw std::runtime_error(typeName(typeid(self_t))+
                                 ": No appropriate LCIO_OUTPUT conversion "
                                 "mechanism known for tag:"+
                                 typeName(typ));
      }

    /// Template class for data conversion. To be specialized by the client.
    /**
     *  \author  M.Frank
     *  \version 1.0
     *  \ingroup DD4HEP_SIMULATION
     */
    template <typename OUTPUT, typename ARGS, typename TAG>
      class Geant4DataConversion : public Geant4Conversion<OUTPUT,ARGS> {
    public:
      typedef TAG tag_t;
      typedef ARGS arg_t;
      typedef OUTPUT output_t;
      typedef Geant4Conversion<output_t,arg_t> self_t;
    Geant4DataConversion(void*) : Geant4Conversion<OUTPUT,ARGS>()
        {
          this->self_t::conversions().insert(make_pair(&typeid(TAG),this));
          //std::cout << "Registered " << typeName(typeid(*this)) << std::endl;
        }
      virtual OUTPUT* operator()(const ARGS& args) const;
    };

  }    // End namespace Simulation
}      // End namespace DD4hep

#define GEANT4_CNAME(a,b) a ## _instance_ ## b
#define DECLARE_GEANT4_DATACONVERTER(output_type,args_type,tag,serial)  \
  DD4hep::Simulation::Geant4DataConversion<output_type,args_type,tag> GEANT4_CNAME(s_g4_data_cnv,serial) (0);

#define DECLARE_GEANT4_HITCONVERTER(output_type,args_type,tag) DECLARE_GEANT4_DATACONVERTER(output_type,args_type,tag,__LINE__)

#endif

#endif // DD4HEP_DDG4_GEANT4CONVERSION_H
