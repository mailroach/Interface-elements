#include "InterfaceWriter.h"
#include "Global.h"
#include "Element.h"
#include "Node.h"

// ---------------------------------------------------------
//   writeInterface
// ---------------------------------------------------------

/*
 * Write the interface elements to file using the following format
 *
 * Element
 * Number-of-element (120, for example)
 * Number-of-nodes-per-element (4, for linear 1D interface elements)
 * elemID matID node1 node2 node1p node2p 
 * ...
 * Node
 * Number-of-node
 * nodeId  dupNode1 dupNode2 1 (1 to indicate not an interfacial node)
 * nodeId  dupNode1 dupNode2 2 (2 to indicate nodeId is an interfacial node)
 */

void  writeInterface 

(       Global&  globdat,
  const char*    fileName )

{
  if (globdat.outAbaqus) return;
 
  const int   ieCount = globdat.interfaceSet.size ();
  const int   inCount = globdat.nodeSet.     size ();

  ElemPointer ep;

  IntVector   connec;
  IntVector   dupNodes;

  int         index;
  int         inter;

  ofstream    file ( fileName, std::ios::out );
  
  cout << "Writing interface elements...\n";

  file << "Element\n" 
       << ieCount    << endl
       << globdat.nodeICount << endl;

  for ( int ie = 0; ie < ieCount; ie++ )
  {
    ep = globdat.interfaceSet[ie];

    file << ep->getIndex() << " " 
         << globdat.interfaceMats[ie] << " "
         << ep->getBulk1() << " " 
         << ep->getBulk2() << " ";

    ep->getConnectivity ( connec );

    copy ( connec.begin(), 
	   connec.end(), 
	   ostream_iterator<int> ( file, " " ) 
	 );

    file << "\n";
  }

  file << "Node\n" << inCount << endl;

  for ( int in = 0; in < inCount; in++ )
  {
    index  = globdat.nodeSet[in]->getIndex ();
    inter  = 1;

    if ( globdat.nodeSet[in]->getIsInterface () )
    {
      inter = 2;
    }

    dupNodes = globdat.duplicatedNodes[index];

    dupNodes.push_back ( inter );

    if ( dupNodes.size () != 1 )
    {
      copy ( dupNodes.begin(),
	     dupNodes.end  (), 
	     ostream_iterator<int> ( file, " " ) 
	   );
    }
    else
    {
      file << index << " " << inter;
    }

    file << endl;
  }

  if ( globdat.is3D )
  {
    file << "OppositeVertices\n";

    for ( int ie = 0; ie < ieCount; ie++ )
    {
      file << globdat.oppositeVertices[ie] << "\n";
    }
  }

  cout << "Writing interface elements...done!\n\n";

  file.close ();
}

