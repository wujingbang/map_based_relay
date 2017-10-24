#include "mbr_sumomap.h"

//#include <iostream>


#include <stdlib.h>
#include "ns3/mobility-model.h"
#include "ns3/core-module.h"


#include "debug.h"
#include "geohash.h"

#include "linux_list.h"

#include "proj/include/proj_api.h"

using namespace std;
using namespace tinyxml2;
using namespace ns3;
using namespace mbr;

//string sumoMapFilename = "E:\\work\\SUMO\\sumo-osm-no-internal.net.xml";

int debug_level = MBR_DBG_DEFAULT;  

//Eager Singleton
MbrSumo* MbrSumo::p = new MbrSumo;

MbrSumo * MbrSumo::GetInstance(void)
{
	return p;
}
void MbrSumo::Tokenize(const string& str,
        vector<string>& tokens,
        const string& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


string MbrSumo::parseOsmWayid(string str)
{
	vector<string> tokens, tokens1;
	Tokenize(str, tokens, "#");
	Tokenize(tokens[0], tokens1, "-");
	if (tokens1.size() == 1)
	  return tokens1[0];
	else
	  return tokens1[1];
//	return tokens[0];
}

void MbrSumo::parseBoundary(XMLElement *location)
{
	string netOffset, convBoundary, origBoundary;
	vector<string> tokens;
	netOffset = location->Attribute("netOffset", NULL);
	convBoundary = location->Attribute("convBoundary", NULL);
	origBoundary = location->Attribute("origBoundary", NULL);
	m_bound.projParameter = location->Attribute("projParameter", NULL);

	Tokenize(netOffset, tokens, ",");
	m_bound.netoffset_x = atof(tokens[0].c_str());
	m_bound.netoffset_y = atof(tokens[1].c_str());
	tokens.clear();
	Tokenize(convBoundary, tokens, ",");
	m_bound.conv_x1 = atof(tokens[0].c_str());
	m_bound.conv_y1 = atof(tokens[1].c_str());
	m_bound.conv_x2 = atof(tokens[2].c_str());
	m_bound.conv_y2 = atof(tokens[3].c_str());
	tokens.clear();
	Tokenize(origBoundary, tokens, ",");
	m_bound.orig_x1 = atof(tokens[0].c_str());
	m_bound.orig_y1 = atof(tokens[1].c_str());
	m_bound.orig_x2 = atof(tokens[2].c_str());
	m_bound.orig_y2 = atof(tokens[3].c_str());
	return;
}

void MbrSumo::sumoCartesian2GPS(double input_x, double input_y,
		double *output_x, double *output_y)
{
    projPJ myProjection = pj_init_plus(m_bound.projParameter.c_str());
    projUV p;
    p.u = input_x - m_bound.netoffset_x;
    p.v = input_y - m_bound.netoffset_y;
    p = pj_inv(p, myProjection);
    *output_x = p.u * RAD_TO_DEG;
    *output_y = p.v * RAD_TO_DEG;
}

uint64_t MbrSumo::sumoCartesian2Geohash(double input_x, double input_y)
{
	double x,y;
	GeoHashRange lat_range, lon_range;
	GeoHashBits geohashbits;
	sumoCartesian2GPS(input_x, input_y, &x, &y);
	/**
	 * ����Ҫ�ñ����еľ�γ��������ʵ��ʵ�鱣��һ��
	 */
	lat_range.min = LAT_RANGE_MIN;//bound.orig_y1;
	lat_range.max = LAT_RANGE_MAX;//bound.orig_y2;
	lon_range.min = LON_RANGE_MIN;//bound.orig_x1;
	lon_range.max = LON_RANGE_MAX;//bound.orig_x2;
	geohash_fast_encode(lat_range, lon_range, y, x, GEOHASH_STEP_BIT, &geohashbits);
	return geohashbits.bits;

}

void MbrSumo::parseShapeAndUpdateGraph(
		const char *fromid, const char *toid, int roadid, string shape) {
	vector<string> tokens, tokens2;
	//vector<string>::iterator t;
	unsigned int i;
	uint64_t geohash;
	Vertex *v;
	Vertex *last_v = NULL;
	Tokenize(shape, tokens, " ");
	if(tokens.size() <= 2)
		return;

	/**
	 * first node is "fromNode", last node is "toNode",
	 * They will be added in the "junction" part.
	 * The id of junction node is different from normal node.
	 */
	/**
	 * Create Vertexes
	 */
	double x,y;
	for(i = 1; i < (tokens.size() - 1); i++) {

		if(MbrGraph::getVertex(m_graph, tokens[i].c_str()))
			continue;//vertex exists.

		tokens2.clear();
		Tokenize(tokens[i], tokens2, ",");
		geohash = sumoCartesian2Geohash(atof(tokens[0].c_str()), atof(tokens[1].c_str()));
		NS_ASSERT(geohash > 0);
		sumoCartesian2GPS(atof(tokens[0].c_str()), atof(tokens[1].c_str()), &x, &y);
		v = MbrGraph::vertex_create(tokens[i].c_str(),x,y, geohash, 0 /*notIntersection*/);
		MbrGraph::graph_add_vertex(m_graph, v);
	}

	/**
	 * Create Edges
	 */
	for(i = 1; i < (tokens.size() - 1); i++) {
		v = MbrGraph::getVertex(m_graph, tokens[i].c_str());
		if(!v) {
//			cout << "parseShapeAndUpdateGraph : ERROR!!" <<endl;
			return;
		}
		if(!last_v) {
			last_v = v;
			continue;
		}
		MbrGraph::vertex_add_edge_to_vertex_undirect_exclusive(last_v, v, roadid);
	}

	MbrGraph::vertex_add_edge_to_vertex_undirect_exclusive(
			MbrGraph::getVertex(m_graph, fromid), MbrGraph::getVertex(m_graph, tokens[1].c_str()), roadid);
	MbrGraph::vertex_add_edge_to_vertex_undirect_exclusive(
			MbrGraph::getVertex(m_graph, tokens[tokens.size() - 2].c_str()), MbrGraph::getVertex(m_graph, toid), roadid);


}
//static string make_coordinate_string(string x, string y)
//{
//	x.append(",");
//	x.append(y);
//	return x;
//}
void MbrSumo::Initialize(string sumoMapFilename, string osmMapFileName)
{
	if (!sumoMapFilename.empty())
		m_sumoMapFilename = sumoMapFilename;

	m_osmMapFileName = osmMapFileName;
	//m_netdevicelist.Add(netdevicelist);
	loadSumoMap(m_sumoMapFilename);
	MbrGraph::graph_division(m_graph);
	//m_initialized = 1;
	m_mapLoaded = true;

}

string MbrSumo::parseOsmRoadName(string wayid)
{
  //Search the original osm map file for road name.
  //FIXME: combined road name: West 50th Street/Avenue of the Americas (Node, not edge!)
  XMLDocument doc;
  string id, k;
  if(doc.LoadFile(m_osmMapFileName.c_str()))
  {
      NS_FATAL_ERROR("Could not open OSM Map file " << m_osmMapFileName.c_str() << " for reading, aborting here \n");
      return NULL;
  }
  XMLElement *root = doc.RootElement();
  XMLElement *way = root->FirstChildElement("way");
  while (way)
  {
      id = way->Attribute("id", NULL);
      if (id == wayid)
	{
	  XMLElement *tag = way->FirstChildElement("tag");
	  while (tag)
	    {
	      k = tag->Attribute("k", NULL);
	      if (k == "name")
		  return tag->Attribute("v", NULL);
	      tag = tag->NextSiblingElement("tag");
	    }
	}
      way = way->NextSiblingElement("way");
  }
  return "";
}

Graph * MbrSumo::loadSumoMap(string sumoMapFilename)
{
 	string id,roadid_str, xstr, ystr, fromid, toid;
 	int roadid_int;
//	mapboundary bound;
	map<string, node> nodeMap;

	node temp;

    XMLDocument doc;
//    cout<<sumoMapFilename << endl;
    if(doc.LoadFile(sumoMapFilename.c_str()))
    {
    	NS_FATAL_ERROR("Could not open sumoMap file " << sumoMapFilename.c_str() << " for reading, aborting here \n");
 //       cout<<"read file error!"<<endl;
//        cout << doc.LoadFile(sumoMapFilename.c_str()) << endl;
        return NULL;
    }

    m_graph = MbrGraph::graph_create();

    XMLElement *root = doc.RootElement();
    XMLElement *location = root->FirstChildElement("location");
    parseBoundary(location);

    /**
     * junction ������·�ڵĵ㣬û�а���������·�ϵĵ㡣������·�ϵĵ�λ��edge��shape��
     */
    XMLElement *junction = root->FirstChildElement("junction");
    /**
     * Create Map Nodes (Intersections) from SUMO net file
     */
    while (junction)
    {

    	id = junction->Attribute("id", NULL);
    	xstr = junction->Attribute("x", NULL);
    	ystr = junction->Attribute("y", NULL);
    	//id = make_coordinate_string(xstr, ystr);
    	if(nodeMap.count(id) > 0)
    		continue;

    	temp.x = atof(xstr.c_str());
    	temp.y = atof(ystr.c_str());

    	temp.geohash = sumoCartesian2Geohash(temp.x, temp.y);
    	NS_ASSERT(temp.geohash > 0);
    	sumoCartesian2GPS(temp.x, temp.y, &temp.x, &temp.y);
    	temp.id = id;
    	nodeMap[id] = temp;

    	MbrGraph::graph_add_vertex(m_graph, MbrGraph::vertex_create(id.c_str(), temp.x,temp.y, temp.geohash, 1 /*isIntersection*/));
    	junction = junction->NextSiblingElement("junction");
    }
    /**
     * Create Vertexes and Edges
     * edge��Ŀ�У��ܶ�û��shape�ֶΣ�lane�У�������junction���겻ƥ�䣩�������ʵ����ֻ�������㣬��from��to�ֶξͿ����ˡ�
     * �������shape�ֶΣ�����ĩ��������from��to��һ�µġ�
     */
    int kkk = 1;
    XMLElement *edge = root->FirstChildElement("edge");
    while (edge)
    {
    	string shape;
    	string osm_wayid;
    	id = edge->Attribute("id", NULL);
    	osm_wayid = parseOsmWayid(id); //roadid#roadedges_id
    	if (!m_osmMapFileName.empty())
    	  roadid_str = parseOsmRoadName(osm_wayid);
    	else
    	  roadid_str = osm_wayid; //Self-defined net file.

    	fromid = edge->Attribute("from", NULL);
    	toid = edge->Attribute("to", NULL);

    	map<string, int>::iterator iter;
        iter = m_map_roadid.find(roadid_str);
	if(iter != m_map_roadid.end())
	{
		roadid_int = iter->second;
	}
	else
	{
		roadid_int = kkk;
		m_map_roadid.insert(pair<string, int>(roadid_str, kkk++));
	}


    	if(!(edge->Attribute("shape", NULL))) {
    		MbrGraph::vertex_add_edge_to_vertex_undirect_exclusive(MbrGraph::getVertex(m_graph, fromid.c_str()),
    				MbrGraph::getVertex(m_graph, toid.c_str()),
    				roadid_int);
    	} else {
    		//Note that edge -> lane -> shape is ignored.
    		shape = edge->Attribute("shape", NULL);
    		parseShapeAndUpdateGraph(fromid.c_str(), toid.c_str(), roadid_int, shape);
    	}

    	edge = edge->NextSiblingElement("edge");
    }

	return m_graph;
}

uint64_t MbrSumo::GetNodeCurrentGeohash(Ptr<Node> node)
{
	Ptr<MobilityModel> MM = node->GetObject<MobilityModel> ();
	Vector pos;
	pos.x = MM->GetPosition ().x;
	pos.y = MM->GetPosition ().y;

	return sumoCartesian2Geohash(pos.x, pos.y);
}

void MbrSumo::GetNodeCurrentXY(Ptr<Node> node, double *x, double *y)
{
	Ptr<MobilityModel> MM = node->GetObject<MobilityModel> ();
	Vector pos;
	pos.x = MM->GetPosition ().x;
	pos.y = MM->GetPosition ().y;

	sumoCartesian2GPS(pos.x, pos.y, x, y);
	return ;
}

