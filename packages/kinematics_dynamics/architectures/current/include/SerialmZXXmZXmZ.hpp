/***************************************************************************
  tag: Peter Soetens  Thu Oct 10 16:21:19 CEST 2002  SerialmZXXmZXmZ.hpp 

                        SerialmZXXmZXmZ.hpp -  description
                           -------------------
    begin                : Thu October 10 2002
    copyright            : (C) 2002 Peter Soetens
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
 

#ifndef SerialmZXXmZXmZ_HPP
#define SerialmZXXmZXmZ_HPP

#include "SerialZXXZXZ.hpp"
#include <math.h>    /* cos,sin,fabs */
#include <corelib/MultiVector.hpp>

namespace ORO_KinDyn
{

    /**
     * This class provides the kinematics for an ZXXZXZ robot with inverse Z-axes.
     * It uses the ZXXZXZ implementation for that.
     * 
     * @todo do this with angular offsets in kinematics component
     */
    class SerialmZXXmZXmZ : public Serial321
    {

    public:
        virtual ~SerialmZXXmZXmZ();

        virtual bool jacobianForward( const ORO_CoreLib::Double6D& q, double J[ 6 ][ 6 ], Singularity& s ) const;

        virtual bool jacobianInverse( const ORO_CoreLib::Double6D& q, double J[ 6 ][ 6 ], Singularity& s ) const;

        virtual bool positionForward( const ORO_CoreLib::Double6D& q, ORO_Geometry::Frame& mp_base, Singularity& s ) const;

        virtual bool positionInverse( const ORO_Geometry::Frame& mp_base, Configuration c, ORO_CoreLib::Double6D& q, Singularity& s ) const;

        virtual bool velocityInverse( const ORO_Geometry::Frame& pos_base, const ORO_Geometry::Twist& vel_base, Configuration c,  ORO_CoreLib::Double6D& q, ORO_CoreLib::Double6D& qdot, Singularity& s ) const;

        virtual bool velocityInverse( const ORO_CoreLib::Double6D& q, const ORO_Geometry::Twist& vel_base, ORO_CoreLib::Double6D& qdot, Singularity& s ) const;

        virtual bool velocityForward( const ORO_CoreLib::Double6D& q, const ORO_CoreLib::Double6D& qdot, ORO_Geometry::Frame& pos_base, ORO_Geometry::Twist& vel_base, Singularity& s ) const;

        virtual void configurationGet(const ORO_CoreLib::Double6D& q, Configuration& c) const;

    virtual void geometrySet( const double len1, const double len2, const double len3,
                      const double len4, const double len5, const double len6 )
    {
        zxxzxz.geometrySet(len1,len2,len3,len4,len5,len6);
    }

    virtual void offsetSet( const double off )
    {
        zxxzxz.offsetSet(off);
    }

    virtual void eccentricitySet( const double ecc )
    {
        zxxzxz.eccentricitySet(ecc);
    }

    private:
        SerialZXXZXZ zxxzxz;
    };
}

#endif