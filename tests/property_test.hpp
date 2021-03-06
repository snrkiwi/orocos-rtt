/***************************************************************************
  tag: Peter Soetens  Tue Apr 5 16:53:26 CEST 2005  property_test.hpp

                        property_test.hpp -  description
                           -------------------
    begin                : Tue April 05 2005
    copyright            : (C) 2005 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef PROPERTYTEST_H
#define PROPERTYTEST_H

#include <Property.hpp>
#include <PropertyBag.hpp>

using namespace RTT;

struct Composite {
    Composite() : a(0), b(-1), c(+1) {}
    void reset() { a = 0; b = -1; c = +1; }
    void change() { a = -1; b=+1; c=0; }
    bool isReset() { return a == 0 && b == -1 && c == +1;}
    bool isChanged() { return a == -1 && b == +1 && c == 0;}
    int a,b,c;
};

class PropertyTest
{
public:
    PropertyBag bag1;
    PropertyBag bag2;
    PropertyBase* pb;
    Property<int>* pi1;
    Property<int>* pi2;
    Property<int>* pi1ref;
    Property<int>* pi2ref;
    int intref;
    Composite compref;

    PropertyTest(){ setUp(); };
    ~PropertyTest(){ tearDown(); };

    void setUp();
    void tearDown();

    void testPrimitives();
    void testBags();
    void testBagOperations();
    void testRepository();
    void testComposition();
    void testInit();
    void testUpdate();
    void testPropMarsh();
    void testPropMarshVect();
};

#endif  // PROPERTYTEST_H
