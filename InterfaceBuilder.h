/**
 * This file is a part of the interface element generator program.
 *
 * V.P.Nguyen, TU Delft 2009
 *
 */

#ifndef INTERFACE_BUILDER_H
#define INTERFACE_BUILDER_H

#include "typedefs.h"

class Global;

class  InterfaceBuilder
{
  public:

    static    void       doIt 

         ( Global& globdat );

    static    void       doForMatInterface

         ( Global& globdat );

    static    void       doForDomain

         ( Global& globdat );
    
    static    void       doForEverywhere

         ( Global& globdat );
    
    static    void       doForEverywhere2D

         ( Global& globdat );
    
    static    void       doForEverywhere3D

         ( Global& globdat );

    static    void       doForPolycrystal

         ( Global& globdat );

    
    static    void       doFor3DMatInterface

         ( Global& globdat );

    static    void       doFor2DMatInterface

         ( Global& globdat );
    
    static    void       doFor2DMatInterfaceNURBS

         ( Global& globdat );

    static    void       doFor2DPolycrystal

         ( Global& globdat );

    static    void       doFor3DPolycrystal

         ( Global& globdat );

};

void                     addInterface

         ( IntVector& connectivity,
	   int   n1,
	   int   n2,
	   int   m12,
	   bool  changed,
	   Global& globdat );

void                     addInterface

         ( IntVector& connectivity,
	   int        n1,
	   int        n2,
	   int        m12,
	   int        p1,
	   int        p2,
	   bool       changed,
	   Global&    globdat );

void                     addDiscreteInterface

         ( Global& globdat );

void                     findCommonEdge

         ( int&              p1,
	   int&              p2,
	   int               n1,
	   int               n2,
	   int               ielem,
	   const IntVector&  neighbors,
	   Global&           globdat );

#endif
