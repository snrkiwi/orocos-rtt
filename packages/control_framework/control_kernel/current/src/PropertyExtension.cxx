/***************************************************************************
  tag: Peter Soetens  Mon May 10 19:10:37 CEST 2004  PropertyExtension.cxx 

                        PropertyExtension.cxx -  description
                           -------------------
    begin                : Mon May 10 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
#pragma implementation
#include "control_kernel/PropertyExtension.hpp"
#include <corelib/PropertyComposition.hpp>
#include <corelib/PropertyBag.hpp>
#include <corelib/Logger.hpp>
#include <corelib/marshalling/CPFMarshaller.hpp>

namespace ORO_ControlKernel
{
    using namespace std;
    using namespace ORO_CoreLib;

    bool PropertyComponentInterface::enableAspect( PropertyExtension* ext)
    {
        if (master == 0)
            {
                master = ext;
                return master->addComponent( this );
            }
        return false;
    }

    void PropertyComponentInterface::disableAspect()
    {
        if (master !=0)
            {
                master->removeComponent( this );
                master = 0;
            }
    }

    PropertyExtension::PropertyExtension(ControlKernelInterface* _base ) 
        : detail::ExtensionInterface(_base, "Property"),
          save_props("SaveProperties","Save the components property files on kernel stop.",false),
          configureOnLoad("ConfigureOnLoad","Configure the component when loaded," \
                          "instead of when the kernel is started", true),
          ignoreMissingFiles("IgnoreMissingFiles","Do not configure a component if no" \
                             "property file is listed.", false),
          saveFilePrefix("SaveFilePrefix","The suffix of the filename to store the components state into.", std::string("Exported_")),
          saveFileExtension("SaveFileExtension","The extension of the filename to store the components state into.",std::string("cpf")),
          base(_base)
    {
    }

    PropertyExtension::~PropertyExtension()
    {
        for( CompNames::iterator it = componentFileNames.begin();
             it != componentFileNames.end(); ++it )
            delete *it;
    }

//     TaskInterface* PropertyExtension::getTask() const
//     {
//         return base->getTask();
//     }
    
//     void PropertyExtension::setTask( TaskInterface* task )
//     {
//         base->setTask( task );
//     }

    bool PropertyExtension::updateProperties(const PropertyBag& bag)
    {
        composeProperty(bag, save_props);
        composeProperty(bag, saveFilePrefix);
        composeProperty(bag, saveFileExtension);
        composeProperty(bag, ignoreMissingFiles);
        composeProperty(bag, configureOnLoad);

        Logger::log() << Logger::Info << "PropertyExtension Properties : "<<Logger::nl
                      << save_props.getName()<< " : " << save_props.get() << Logger::nl
                      << saveFilePrefix.getName()<< " : " << saveFilePrefix.get() << Logger::nl
                      << saveFileExtension.getName()<< " : " << saveFileExtension.get() << Logger::nl
                      << ignoreMissingFiles.getName()<< " : " << ignoreMissingFiles.get() << Logger::nl
                      << configureOnLoad.getName()<< " : " << configureOnLoad.get() << Logger::endl;
            
        // build new list of present component config files
        for( CompNames::iterator it = componentFileNames.begin();
             it != componentFileNames.end(); ++it )
            delete *it;
        componentFileNames.clear();

        PropertyBase* res = bag.find("PropertyFiles");
        if ( res && dynamic_cast< Property< PropertyBag >*>( res ) )
            {
                PropertyBag& bag =
                    dynamic_cast<Property<PropertyBag>* >( res )->value();

                for( PropertyBag::iterator it = bag.getProperties().begin();
                     it !=bag.getProperties().end();
                     ++it)
                    if ( dynamic_cast< Property<string>*>( *it ) )
                        componentFileNames.push_back
                            ( new Property<std::string>( *dynamic_cast< Property<string>*>(*it) ) );
            }
        else
            {
                Logger::log() << Logger::Error << "PropertyExtension : sequence \"PropertyFiles\" not found !"<< Logger::endl;
                return false;
            }
        return true;
    }
        
    bool PropertyExtension::initialize()
    {
        if (configureOnLoad)
            return true; // All is done.

        Logger::log() << Logger::Info << "PropertyExtension : initialize on Kernel.start()."<< Logger::endl;
        // read xml file for each component, if we know it.
//         for ( CompNames::iterator it = componentFileNames.begin(); it!= componentFileNames.end(); ++it)
//             {
//                 CompMap::iterator tg = myMap.find( (*it)->getName() );
//                 if ( tg == myMap.end() )
//                     {
//                         Logger::log() << Logger::Warning << "PropertyExtension : ";
//                         Logger::log() << "Component "<<(*it)->getName() << " not found !"<< Logger::endl;
//                         continue;
//                     }
//                 if ( configureComponent( (*it)->value(), tg->second ) ==  false)
//                     return false;
//             }
        for ( CompMap::iterator tg = myMap.begin(); tg!= myMap.end(); ++tg)
            {
                bool didconf=false; // keep track of configuration
                for ( CompNames::iterator it = componentFileNames.begin(); it!= componentFileNames.end(); ++it)
                    if ( (*it)->getName() == tg->second->getName() )
                        if ( this->configureComponent( (*it)->value(), tg->second ) ==  false) {
                            return false;
                        }
                        else {
                            didconf = true;
                            break;
                        }
                // If not tg not configured and not ignore unlisted components :
                if ( ! didconf && !ignoreMissingFiles.get() ) {
                    Logger::log() << Logger::Info << "PropertyExtension : ";
                    Logger::log() << "No Property file for Component "<<tg->second->getName() << " listed !"<< Logger::endl;
                    PropertyBag emptyBag;
                    if ( tg->second->updateProperties( emptyBag ) == false ) {
                        Logger::log() << Logger::Error << "PropertyExtension : "
                                      << "Component " << tg->second->getName() 
                                      << " does not accept empty properties." << Logger::nl
                                      << "Fix your PropertyExtension config file first, or set property ignoreMissingFiles to 1."<< Logger::endl;
                        return false;
                    }
                    didconf=false; // reset
                }
            }
        return true;
    }

    bool PropertyExtension::configureComponent(const std::string& filename, PropertyComponentInterface* target)
    {
        ComponentConfigurator cc;
        if ( !cc.configure( filename, target) )
            {
                Logger::log() << Logger::Error << "PropertyExtension : ";
                Logger::log() << "Component "<<target->getName() 
                              << " does not accept properties from file '"+filename+"'." << Logger::nl
                              << "Fix your Component property config file first."<< Logger::endl;
                return false;
            }
        return true;
    }

    void PropertyExtension::step()
    {
        // do nothing ?
    }

    void PropertyExtension::finalize()
    {
        if ( save_props )
            {
                Logger::log() << Logger::Debug << "Saving Component Properties to files..."<<Logger::endl;
                // iterate over components
                CompMap::iterator comp_it = myMap.begin();
                while ( comp_it != myMap.end() )
                {
                    PropertyBag allProps;
                    // collect properties
                    comp_it->second->exportProperties( allProps );
                    // serialize and cleanup
                    std::ofstream file((saveFilePrefix.get() + comp_it->first+"." + saveFileExtension.get()).c_str());
                    CPFMarshaller<std::ostream> marshaller( file );
                    marshaller.serialize( allProps );
                    allProps.clear();
                    Logger::log() << Logger::Info << "Wrote "<<saveFilePrefix.get() + comp_it->first +"."+ saveFileExtension.get()<<Logger::endl;
                    ++comp_it;
                }
            }
    }
        
    bool PropertyExtension::addComponent(PropertyComponentInterface* comp)
    {
        if ( myMap.count(comp->getLocalStore().getName() ) != 0  )
            return false;
        myMap[ comp->getLocalStore().getName() ] = comp;
        if ( configureOnLoad )
            {
                for ( CompNames::iterator it = componentFileNames.begin(); it!= componentFileNames.end(); ++it)
                    if ( (*it)->getName() == comp->getLocalStore().getName() )
                        return configureComponent( (*it)->value(), comp );
                // reached when not found
                Logger::log() << Logger::Info << "PropertyExtension : "
                              << "No property file found for "<<comp->getLocalStore().getName()<< Logger::endl;
                if ( !ignoreMissingFiles.get() )
                    {
                        PropertyBag emptyBag;
                        if ( comp->updateProperties( emptyBag ) == false ) {
                            Logger::log() << Logger::Error << "PropertyExtension : "
                                          << "Component " << comp->getName() 
                                          << " does not accept empty properties : not Loading." << Logger::nl 
                                          << "Fix your PropertyExtension config file first, or set property 'IgnoreMissingFiles' to 1."<< Logger::endl;
                            return false;
                        }
                    }

            }
        return true;
    }

    void PropertyExtension::removeComponent(PropertyComponentInterface* comp)
    {
        myMap.erase( comp->getLocalStore().getName() );
    }


}