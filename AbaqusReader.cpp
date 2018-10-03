#include "Global.h"
#include "Node.h"
#include "Element.h"


#include <boost/algorithm/string.hpp>


void                     readAbaqusMesh 

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
  int             elemNode;
  int             elemType, paraElemType;

  double          x,y,z;

  StrVector       splitLine;
  IntVector       connectivity;

  string  line;

  cout << "Reading Abaqus mesh file ...\n";
  cout << "Reading nodes...\n";

  for (int i = 0; i < 9; i++)
  {
	  getline(file, line);
  }

  getline ( file, line );

  const int nodeCount = boost::lexical_cast<int> ( line );

  for ( int in = 0; in < nodeCount; in++ )
  {
	  getline(file, line);
	  boost::erase_all(line, " ");

	  boost::split(splitLine, line, boost::is_any_of(","));

	  length = splitLine.size();

	  id = boost::lexical_cast<int> (splitLine[0]);
	  x = boost::lexical_cast<double> (splitLine[1]);
	  y = boost::lexical_cast<double> (splitLine[2]);
	  if (length > 3)
	  {
		  z = boost::lexical_cast<double> (splitLine[3]);
		  globdat.is3D = true;
	  }


    globdat.nodeSet.push_back ( NodePointer( new Node(x,y,z,id) ) );

    globdat.nodeId2Position[id] = globdat.nodeSet.size()-1;
  }


  cout << "Reading nodes...done!\n\n";

  cout << "Reading elements...\n";

  getline(file, line);
  std::size_t found = line.find("type=");
  elemNode = boost::lexical_cast<int> (line[found + 8]);
  if (elemNode == 3)
  {
	  elemType = 2;
  }
  else if (elemNode == 4)
  {
	  elemType = 3;
  }
  else if (elemNode == 8)
  {
	  elemType = 5;
  }
  else
  {
	  cerr << "element type is not supported!\n";
	  exit(1);
  }
  matId = 1;

  getline(file, line);
  const int elemCount = boost::lexical_cast<int> (line);

  for ( int ie = 0; ie < elemCount; ie++ )
  {
    getline ( file, line );
	boost::erase_all(line, " ");

    boost::split ( splitLine, line, boost::is_any_of(",") );

    length       = splitLine.size ();
   
    // material => elements
    // element  => material (domain)

    globdat.dom2Elems[matId].push_back ( ie );
    globdat.elem2Domain[ie] = matId;

    // the rest are solid elements 
    // either 2D solid elements or 3D solid elements

    // read connectivity 

    connectivity.clear ();

    transform ( splitLine.begin()+1, 
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

    for ( unsigned int ie = 0; ie < globdat.elemSet.size (); ie++ )
    {
      globdat.elemSet[ie]->buildFaces0 ();
    }
    cout << "Building initial faces of 3D elements...done\n\n"; 
  }


}
