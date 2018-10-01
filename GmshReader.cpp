#include "Global.h"
#include "Node.h"
#include "Element.h"


#include <boost/algorithm/string.hpp>


void                     readGmshMesh 

    ( Global&     globdat,
      const char* fileName )
{
  ifstream file ( fileName, std::ios::in );

  if ( !file ) 
  {
    cout << "Unable to open mesh file!!!\n\n";
    exit(1);
  }

  int             id;
  int             length;
  int             matId;
  int             elemType, paraElemType;

  double          x,y,z;

  StrVector       splitLine;
  IntVector       connectivity;

  string  line;

  cout << "Reading Gmsh mesh file ...\n";
  cout << "Reading nodes...\n";

  getline ( file, line );
  getline ( file, line );
  getline ( file, line );
  getline ( file, line );

  getline ( file, line );

  const int nodeCount = boost::lexical_cast<int> ( line );

  for ( int in = 0; in < nodeCount; in++ )
  {
    file >> id >> x >> y >> z;

    globdat.nodeSet.push_back ( NodePointer( new Node(x,y,z,id) ) );

    globdat.nodeId2Position[id] = globdat.nodeSet.size()-1;
  }

  // checking two or three dimensional mesh

  for ( int in = 0; in < nodeCount; in++ )
  {
    if ( globdat.nodeSet[in]->getZ() != 0. )
    {
      globdat.is3D = true;
      break;
    }
  }

  cout << "Reading nodes...done!\n\n";

  cout << "Reading elements...\n";

  getline ( file, line );
  getline ( file, line );
  getline ( file, line );

  getline ( file, line ); 

  const int elemCount = boost::lexical_cast<int> ( line );

  for ( int ie = 0; ie < elemCount; ie++ )
  {
    getline ( file, line );

    boost::split ( splitLine, line, boost::is_any_of("\t ") );

    length       = splitLine.size ();

    elemType     = boost::lexical_cast<int> ( splitLine[1] );
    matId        = boost::lexical_cast<int> ( splitLine[3] );	

    // line elements => boundary nodes
    // elemType == 1: two-node   line element
    // elemType == 8: three-node line element

    if ( elemType == 1 ) 
    {
      int no1 = boost::lexical_cast<int> ( splitLine[5] );
      int no2 = boost::lexical_cast<int> ( splitLine[6] );

      globdat.boundaryNodes.insert ( no1 );
      globdat.boundaryNodes.insert ( no2 );

      globdat.bndNodesMap[matId].insert ( no1 );
      globdat.bndNodesMap[matId].insert ( no2 );

      globdat.nodePairs     .push_back ( NodePair (no1, no2) );
      globdat.bndElemsDomain.push_back ( matId );

      continue;
    }

    if ( elemType == 8 )
    {
      int no1 = boost::lexical_cast<int> ( splitLine[5] );
      int no2 = boost::lexical_cast<int> ( splitLine[6] );
      int no3 = boost::lexical_cast<int> ( splitLine[7] ); //midside node

      globdat.boundaryNodes.insert ( no1 );
      globdat.boundaryNodes.insert ( no2 );
      globdat.boundaryNodes.insert ( no3 );

      globdat.bndNodesMap[matId].insert ( no1 );
      globdat.bndNodesMap[matId].insert ( no2 );
      globdat.bndNodesMap[matId].insert ( no3 );

      globdat.nodePairs.push_back ( NodePair (no1, no2) );
      globdat.bndElemsDomain.push_back ( matId );

      continue;
    }

    // node element 

    if ( elemType == 15 )
    {
      globdat.isolatedNodes.push_back ( boost::lexical_cast<int> ( splitLine[5] ) );
      continue;
    }

    // for a 3D mesh, triangles or quadrangles are surface elements
    // ie. they are boundary elements not bulk elements.

    if ( globdat.is3D )
    {
      if ( elemType == 2 || elemType == 9 || // linear or quadratic triangle
	   elemType == 3 || elemType == 16 ) // linear or quadratic quadrangle
      {
	connectivity.clear ();

        transform ( splitLine.begin()+5, 
         	    splitLine.end  (), 
		    back_inserter(connectivity), 
		    Str2IntFunctor() );

        globdat.bndNodesMap[matId].insert ( connectivity.begin(),
	                                    connectivity.end() );
        continue;
      }
    }

    // material => elements
    // element  => material (domain)

    globdat.dom2Elems[matId].push_back ( ie );
    globdat.elem2Domain[ie] = matId;

    // the rest are solid elements 
    // either 2D solid elements or 3D solid elements

    // read connectivity 

    connectivity.clear ();

    transform ( splitLine.begin()+5, 
	        splitLine.end  (), 
		back_inserter(connectivity), 
		Str2IntFunctor() );

    globdat.elemSet.push_back ( ElemPointer ( new Element ( ie, elemType, connectivity ) )  );

    globdat.elemId2Position[ie] = globdat.elemSet.size() - 1;
  }

  cout << "Reading elements...done!\n\n";

  file.close ();

  // check validity of input

  if ( globdat.isDomain )
  {
    Int2IntVectMap::const_iterator it;
    Int2IntVectMap::const_iterator eit = globdat.dom2Elems.end   ();

    it = globdat.dom2Elems.find ( globdat.rigidDomain );

    if ( it == eit )
    {
      cerr << "invalid number of rigid domain!!!\n";
      exit(1);
    }
  }

  if ( globdat.isInterface )
  {
  }

  if ( globdat.isNotch )
  {
    cout << "Existing notch segment is: "; // << globdat.segment << endl;
  }

  if ( globdat.isIgSegment )
  {
    cout << "Do not treat nodes on this segment: "<< globdat.ignoredSegment << endl;
  }

  cout << endl;

  globdat.nodeElemCount = connectivity.size ();

  string elemTypeStr = "linear";

  elemType = globdat.elemSet[0]->getElemType ();

  if ( !globdat.is3D )
  {
    if ( elemType == 2 || elemType == 3 )
    {
      globdat.nodeICount  = globdat.isContinuum ? 4 : 2;
      globdat.isQuadratic = false;
      elemTypeStr         = "linear";
    }
    else
    {
      globdat.nodeICount  = 6;
      globdat.isQuadratic = true;
      elemTypeStr         = "quadratic";
    }
  }
  else
  {
    // hex8 or tet4 elements 
    if ( elemType == 4 || elemType == 5 ) 
    {
      globdat.nodeICount  = elemType == 4 ? 6 : 8;
      globdat.isQuadratic = false; 
      elemTypeStr = "linear";
    }
    // hex20 or tet10 elements 
    else
    {
      globdat.nodeICount  = elemType == 11 ? 12 : 16;
      globdat.isQuadratic = true;
      elemTypeStr = "quadratic";
    }

    // build faces for 3D elements
  
    cout << "Building initial faces of 3D elements...\n\n"; 

    for ( int ie = 0; ie < globdat.elemSet.size (); ie++ )
    {
      globdat.elemSet[ie]->buildFaces0 ();
    }
    cout << "Building initial faces of 3D elements...done\n\n"; 
  }


}
