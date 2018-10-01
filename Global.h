#ifndef GLOBAL_H
#define GLOBAL_H

#include "typedefs.h"
#include "utilities.h"

class NodePair;



// ======================================================
//  class Global
// ======================================================

/*
 * This class stores all the data of the program as public members. 
 * The program creates one object of class Global, called globdat.
 * And this object is passed around in different functions to work on the global data. 
 */


struct Global
{
   NodeSet                  nodeSet;         // set of original nodes
   NodeSet                  newNodeSet;      // set of original nodes+new added nodes
   ElemSet                  elemSet;         // set of volumetric elements (modified)
   ElemSet                  interfaceSet;    // set of interface elements 
   ElemSet                  bndElementSet;   // set of boundary elements (for external force vector) 

   Int2IntVectMap           nodeSupport;     // nodal support 
   Int2IntVectMap           duplicatedNodes;
   Int2IntVectMap           duplicatedNodes0; // back up for above
   Int2IntVectMap           dom2Elems;
   Int2IntVectMap           dom2BndElems;
   Int2IntMap               elem2Domain;
   Int2IntMap               nodeId2Position; // given node's id => position in nodeSet
   Int2IntMap               elemId2Position; // given elem's id => position in elemSet
   IntSet                   boundaryNodes;   // id of nodes on the external boundary
   Int2IntSetMap            bndNodesMap;
   vector<IntVector>        elemNeighbors;
   vector<NodePair>         nodePairs;     // 2D line elements on the boundary
   IntVector                bndElemsDomain; 
   IntVector                isolatedNodes; // store physical points from Gmsh
				    // normally on which displacement is applied 

   IntVector                interfaceNodes; // list of nodes on the mat interfaces
   IntVector                interfaceMats;
   vector<IntVector>        interfacePlanes; // for 3D mesh, 
				      // interfacePlanes[i] contains the nodes
   IntVector                oppositeVertices; // used to compute the correct normals 

   vector<NodePair>         ignoredEdges;   // used for polycrystal

   int                      rigidDomain;  // id of domain where no interface element is created.
   int                      nodeElemCount; // no of nodes per solid element
   int                      nodeICount;    // no of nodes per interface element 
				 

   vector<Segment>          segment; // initial notches
   Segment                  ignoredSegment; // no duplicated nodes, interface elements along
				     // this segment even if they are interfacial nodes

   bool			    isEveryWhere;   
   bool			    isInterface ;
   bool			    isDomain    ;
   bool			    isPolycrystal;
   bool			    isNotch      ;
   bool			    isIgSegment ;
   bool			    isQuadratic ;
   bool			    is3D       ;
   bool			    isConverter;

   bool                     isContinuum; // continuum interface elements or srping elements
   bool                     isNURBS;

   bool                     outAbaqus; // write to Abaqus input files

                            Global ();
};

#endif
