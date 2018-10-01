#include "MeshModifier.h"
#include "Global.h"
#include "Node.h"
#include "Element.h"

// -------------------------------------------------------
//    doIt
// -------------------------------------------------------

/*
 *  Main function to duplicate nodes and tear the elements.
 *  Before that, some datastructure of the mesh are built: node supports, 
 *  element neighbors, interfacial nodes...
 */

void MeshModifier::doIt 

  ( Global&  globdat )
{

  buildNodeSupport      ( globdat );
  buildNeighborElems    ( globdat );
  buildInterfacialNodes ( globdat );
  duplicateNodes        ( globdat ); 
  tearElements          ( globdat ); 
}

// -------------------------------------------------------
//    buildNodeSupport
// -------------------------------------------------------

void MeshModifier::buildNodeSupport

  ( Global&  globdat )

{
  const int   elemCount = globdat.elemSet.size ();
  const int   nodeCount = globdat.nodeSet.size ();

  IntVector   inodes;

  ElemPointer ep;

  cout << "building nodal support...\n";

  for ( int ie = 0; ie < elemCount; ie++ )
  {
    ep = globdat.elemSet[ie];

    ep->getConnectivity ( inodes );

    //print (inodes.begin(), inodes.end());  cout << "\n";

    for ( int in = 0; in < inodes.size(); in++ )
    {
      globdat.nodeSupport[inodes[in]].push_back ( ep->getIndex () );
    }
  }

  cout << "building nodal support...done!\n\n";
}


// -------------------------------------------------------
//    buildNeighborElems
// -------------------------------------------------------

void MeshModifier::buildNeighborElems

  ( Global&  globdat )

{
  int         inodeCnt;

  ElemPointer ep;

  IntVector   inodes;

  cout << "building element neighbors...\n";

  const int   elemCount = globdat.elemSet.size ();

  globdat.elemNeighbors.resize ( elemCount );

  for ( int ie = 0; ie < elemCount; ie++ )
  {
    ep = globdat.elemSet[ie];

    ep->getConnectivity ( inodes );

    inodeCnt = inodes.size ();

    IntSet neighbors;

    for ( int in = 0; in < inodeCnt; in++ )
    {
      int ii = inodes[in];

      for ( int iee = 0; iee < globdat.nodeSupport[ii].size(); iee++ )
      {
	neighbors.insert ( globdat.nodeSupport[ii][iee] );
      }
    }

    copy ( neighbors.begin(), neighbors.end(), 
	   back_inserter ( globdat.elemNeighbors[ie] ) );
  }

  //print ( elemNeighbors );

  cout << "building element neighbors...done!\n\n";
}


// -------------------------------------------------------
//    buildInterfacialNodes
// -------------------------------------------------------

void MeshModifier::buildInterfacialNodes

  ( Global&  globdat )

{
  const int   nodeCount = globdat.nodeSet.size ();
  const int   elemCount = globdat.elemSet.size ();
  
  int         index, suppCount, imat, matCount;
  int         rCount;
  int         duplicity;
  int         nnode;

  IntSet      matSet;
  IntVector   inodes, support;

  NodePointer np;
  ElemPointer ep;

  cout << "detecting interface nodes...\n";

  for ( int in = 0; in < nodeCount; in++ )
  {
    rCount = 0;
    np     = globdat.nodeSet[in];
    index  = np->getIndex ();

    // ignore nodes belong to only ONE element

    support   =  globdat.nodeSupport[index];
    suppCount =  support.size (); 

    //print ( support.begin(), support.end() );

    if ( suppCount == 1 ) continue; 

    for ( int ie = 0; ie < suppCount; ie++ )
    {
      imat = globdat.elem2Domain[support[ie]]; 

      matSet.insert ( imat );

      if ( imat == globdat.rigidDomain ) rCount++;
    }

    matCount  = matSet.size ();

    // if interface elements are created everywhere then 
    // duplicity = nodal support. Else duplicity = materials.

    if       ( globdat.isEveryWhere )
    {
      duplicity = suppCount;
    }
    else if  ( globdat.isInterface  ||  globdat.isPolycrystal )
    {
      duplicity = matCount;
    }
    else if  ( globdat.isDomain     )
    {
      duplicity = matCount != 1 ? suppCount - rCount + 1 : suppCount;
    }

    np->setDuplicity   ( duplicity );

    if ( matCount != 1 )    // interfacial node 
    {
      if ( !globdat.isIgSegment )   // option --noInterface is false
      {
        np->setIsInterface ( true ); 
        globdat.interfaceNodes.push_back ( index );
      }
      else
      {
	if ( globdat.ignoredSegment.isOn ( np->getX(), np->getY() ) )
	{
          np->setDuplicity   ( 1 );
	}
	else
	{
	  np->setIsInterface ( true ); 
          globdat.interfaceNodes.push_back ( index );
	}
      }
    }
    else
    {
      if ( globdat.isDomain )
      {
	if ( find ( matSet.begin(), 
	            matSet.end  (), 
		    globdat.rigidDomain ) != matSet.end () )
	{
	  np->setIsRigid   ( true );
	  np->setDuplicity ( 1    );
	}
      }
    }

    matSet.clear (); // clear for the next node

    // check if this node is on an external boundary or not

    if ( find ( globdat.boundaryNodes.begin(), 
	        globdat.boundaryNodes.end(), 
		index ) != globdat.boundaryNodes.end () )
    {
      np->setIsOnBoundary ( true );
    }
  }

  // For polycrystal, there are some edges connecting
  // two interfacial nodes but this edge is not a grain
  // boundary, so do not add interface element along this edge.

  if ( globdat.isPolycrystal )
  {
     int n1, n2, m1, m2, o1, o2;
     int neiCount, jelem, ielem;
     int imat, jmat;

     ElemPointer jp;
     IntVector   neighbors, jnodes0;

     bool        found;

     IntVector::const_iterator it1, it2;

     // loop over all elements

     for ( int ie = 0; ie < elemCount; ie++ )
     {
	ep    = globdat.elemSet[ie];
        ielem = ep->getIndex() ;
	found = false;

	// if this element contains all interfacial nodes
	
	if ( ep->isInterfaceElement ( globdat.nodeSet, 
	                              globdat.nodeId2Position ) ) 
	{
          neighbors = globdat.elemNeighbors[ie]; 
	  neiCount  = neighbors.size ();

	  ep->getCornerConnectivity0 ( inodes ); //print (inodes.begin(), inodes.end());
	  inodes.push_back ( inodes[0] );  
	  nnode = inodes.size()-1;

	  imat  = globdat.elem2Domain[ielem];

	  // loop over edges to find trouble edge 

	  for ( int in = 0; in < nnode; in++ )
	  {
	    n1 = inodes[in];
	    n2 = inodes[in+1]; //cout << n1 << ";" << n2 << endl;

	    // loop over neighbors of element ie

	    for ( int je = 0; je < neiCount; je++ )
	    {
	       jelem = globdat.elemId2Position[neighbors[je]];
	       jp    = globdat.elemSet[jelem];

	       // omit element being considered

	       if ( jp->getIndex() == ielem ) continue; 

	       jp->getCornerConnectivity0 ( jnodes0 );
	       it1 = find ( jnodes0.begin(), jnodes0.end(), n1 );
	       it2 = find ( jnodes0.begin(), jnodes0.end(), n2 );

	       // common edge between ielem and jelem found

	       if (  it1 != jnodes0.end()  && it2 != jnodes0.end()  )
	       {
                 jmat  = globdat.elem2Domain[neighbors[je]];
                 
		 //print (jnodes0.begin(), jnodes0.end());
		 //cout << imat << "," << jmat << endl;

		 if ( jmat == imat )
		 {
	           globdat.ignoredEdges.push_back ( NodePair(n1,n2) );
                   found = true;

		   //cout << NodePair(n1,n2);
		   break; // only have ONE edge in common
		 }
	       }
	    } // end of loop on neighbors

	    if ( found ) continue; 

	  }   // end of loop on edges
	}
     }        // end of loop on elements

     // debug code
     
     for ( int ie = 0; ie < globdat.ignoredEdges.size (); ie++ )
     {
       cout << globdat.ignoredEdges[ie];
     }
  }

  cout << "Number of interface nodes..." << globdat.interfaceNodes.size() << "\n";
  cout << "detecting interface nodes...done!\n\n";
}


// -------------------------------------------------------
//    duplicateNodes
// -------------------------------------------------------


void MeshModifier::duplicateNodes

  ( Global&  globdat )

{
  globdat.newNodeSet = globdat.nodeSet;

  if ( globdat.isConverter ) return; 

  cout << "duplicating nodes...\n";

  const int     nodeCount = globdat.nodeSet.size ();

        int     index, duplicity;
	int     idd(0);

  NodePointer   np;
  
  for ( int in = 0; in < nodeCount; in++ )
  {
    np    = globdat.nodeSet[in];

    index = np->getIndex ();

    // if only generate interface elements along material
    // interfaces, then ignore nodes which are not interfacial

    if (  ( globdat.isInterface || globdat.isPolycrystal ) && 
	  !np->getIsInterface ()  ) 
    {
      continue;
    }
	
    // no interface elements in rigid domain then
    // not duplicate nodes that are defined rigid.

    if ( ( globdat.isDomain ) && ( np->getIsRigid() ) )
    {
      continue;
    }

    duplicity = np->getDuplicity ();

    //cout << duplicity << "\n";
    
    if ( duplicity == 1 ) continue;  // ignore nodes belong to only ONE element
 
    globdat.duplicatedNodes[index].push_back ( index );

    for ( int id = 0; id < duplicity-1; id++ )
    {
      idd++;

      globdat.newNodeSet.push_back ( NodePointer 	  
	                             ( new Node (np->getX(), np->getY(), np->getZ(),
					 idd + nodeCount ) ) );

      globdat.duplicatedNodes[index].push_back ( idd + nodeCount );
    }
  }

  cout << "number of nodes added: " << idd << "\n\n";

  globdat.duplicatedNodes0 = globdat.duplicatedNodes;
}


// -------------------------------------------------------
//    tearElements
// -------------------------------------------------------

void MeshModifier::tearElements

  ( Global&  globdat )

{
  if ( globdat.isConverter ) return; 

  // tearing elements (modifying its connectivity)

  cout << "tearing elements...\n";

  if      ( globdat.isInterface )
  {
    tearInterfaceElements   ( globdat );
  }
  else if ( globdat.isDomain    )
  {
    tearDomainElements      ( globdat );
  }
  else if ( globdat.isPolycrystal )
  {
    tearPolycrystalElements ( globdat );
  }
  else if ( globdat.isEveryWhere    )
  {
    tearAllElements         ( globdat );
  }

  cout << "tearing elements...done!\n\n";
}


// -------------------------------------------------------
//    tearInterfaceElements
// -------------------------------------------------------

void MeshModifier::tearInterfaceElements

  ( Global&  globdat )

{
  IntVector       support;

  const int       interNodeCount = globdat.interfaceNodes.size (); 

  int             inode; 
  int             suppCount;
  int             ielem;
  int             mat,imat;

  // loop over interface nodes

  for ( int in = 0; in < interNodeCount; in++ )
  {
    inode      = globdat.interfaceNodes[in];

    support    = globdat.nodeSupport[inode];
    suppCount  = support.size ();

    mat        = globdat.elem2Domain[support[0]];

    // loop over support of this node

    for ( int ie = 1; ie < suppCount; ie++ )
    {
      ielem    = globdat.elemId2Position[support[ie]]; 
      imat     = globdat.elem2Domain[support[ie]];

      if ( imat == mat ) continue;

      globdat.elemSet[ielem]->changeConnectivity 
	
	             ( inode, globdat.duplicatedNodes[inode][1] );
    }
  }
}

// -------------------------------------------------------
//    tearDomainElements
// -------------------------------------------------------

void MeshModifier::tearDomainElements

  ( Global&  globdat )

{
  const  int         nodeCount = globdat.nodeSet.size ();

         int         inode;
	 int         ielem, iel;
	 int         suppCount;

	 IntVector   support;

  for ( int in = 0; in < nodeCount; in++ )
  {
    // ignore nodes in rigid domain

    if ( globdat.nodeSet[in]->getIsRigid() ) 
    {
      continue;
    }

    inode      = globdat.nodeSet[in]->getIndex ();
    support    = globdat.nodeSupport[inode];
    suppCount  = support.size ();

    // different treatment for interfacial nodes

    if ( globdat.nodeSet[in]->getIsInterface() )
    {
      //cout << inode << ":   "; // detect interface node correctly
      //print ( support.begin(), support.end() );
      //print ( duplicatedNodes[inode].begin(), duplicatedNodes[inode].end());

      int c = 0;

      for ( int ie = 0; ie < suppCount; ie++ )
      {
        iel      = support[ie];
        if ( globdat.elem2Domain[iel] == globdat.rigidDomain ) 
	{
	  continue; 
	}
        ielem    = globdat.elemId2Position[iel]; 
        globdat.elemSet[ielem]->changeConnectivity ( inode, globdat.duplicatedNodes[inode][1+c] );
	c++;
      }
      continue;  // to next node (the below is skipped)
    }

    // the rest is handled in the following ...
    // first element in the support keeps the original connectivity

    for ( int ie = 1; ie < suppCount; ie++ )
    {
      iel      = support[ie];
      ielem    = globdat.elemId2Position[iel]; 
      
      globdat.elemSet[ielem]->changeConnectivity ( inode, globdat.duplicatedNodes[inode][ie] );
    }
  }
}

// -------------------------------------------------------
//    tearAllElements
// -------------------------------------------------------

void MeshModifier::tearAllElements

  ( Global&  globdat )

{
  const  int         nodeCount = globdat.nodeSet.size ();

         int         inode;
	 int         ielem, iel;
	 int         suppCount;

	 IntVector   support;

  for ( int in = 0; in < nodeCount; in++ )
  {
    inode      = globdat.nodeSet[in]->getIndex ();
    support    = globdat.nodeSupport[inode];
    suppCount  = support.size ();

    // first element in the support keeps the original connectivity

    for ( int ie = 1; ie < suppCount; ie++ )
    {
      iel      = support[ie];
      ielem    = globdat.elemId2Position[iel]; 
      
      globdat.elemSet[ielem]->changeConnectivity ( inode, globdat.duplicatedNodes[inode][ie] );
    }
  }
   
  if ( globdat.is3D )
  {
    cout << "  -rebuilding faces for 3D elements...\n"; 
    for ( int ie = 0; ie < globdat.elemSet.size (); ie++ )
    {
        globdat.elemSet[ie]->buildFaces ();
    }
  }
}

// -------------------------------------------------------
//    tearPlolycrystalElements
// -------------------------------------------------------

void MeshModifier::tearPolycrystalElements

  ( Global&  globdat )

{
  IntVector       support;

  const int       interNodeCount = globdat.interfaceNodes.size (); 

  int             inode, jnode; 
  int             suppCount;
  int             ielem, jelem;
  int             mat, imat, jmat, matCount;

  // loop over interface nodes

  for ( int in = 0; in < interNodeCount; in++ )
  {
    inode      = globdat.interfaceNodes[in];
    jnode      = globdat.nodeId2Position[inode];

    support    = globdat.nodeSupport[inode];
    suppCount  = support.size ();

    matCount   = globdat.nodeSet[jnode]->getDuplicity ();

    // loop over support of this node

    if ( matCount == 2 ) // node between 2 intergrnular boundaries
    {
      // first element in the support is kept unchanded
    
      mat = globdat.elem2Domain[support[0]];

      for ( int ie = 1; ie < suppCount; ie++ )
      {
	ielem    = globdat.elemId2Position[support[ie]]; 
	imat     = globdat.elem2Domain[support[ie]];

	if ( imat == mat ) continue;

	globdat.elemSet[ielem]->changeConnectivity 
	  
	          ( inode, globdat.duplicatedNodes[inode][1] );
      }
    }
    else // nodes at the junction with 3 materials
    {
      int jj = 0;

      IntVector doneNodes;

      for ( int ie = 0; ie < suppCount; ie++ )
      {
	if ( find ( doneNodes.begin(),
		    doneNodes.end  (), ie ) != doneNodes.end () )
	{
	  continue;
	}

	ielem    = globdat.elemId2Position[support[ie]]; 
	imat     = globdat.elem2Domain[support[ie]];

        for ( int je = 0; je < suppCount; je++ )
        {
	  if ( je < ie ) continue;

	  jmat     = globdat.elem2Domain[support[je]]; 

	  if ( jmat != imat ) continue;
	
	  jelem    = globdat.elemId2Position[support[je]]; 

	  globdat.elemSet[jelem]->changeConnectivity 

	            ( inode, globdat.duplicatedNodes[inode][jj] );

	  doneNodes.push_back ( ie );
	  doneNodes.push_back ( je );
        }

	jj++;
      }
    }
  }
}
