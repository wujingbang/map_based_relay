#include "boost/config.hpp"
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/adjacency_list.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

#include <boost/bimap.hpp>
#include <boost/bimap/support/lambda.hpp>

#include <cstdio>
#include <iostream>
#include <assert.h>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <algorithm>


using namespace boost;
using namespace rapidjson;

#define GPS_DECIMALS 10000000

struct Node {
    std::string id;
    double x;
    double y;

    Node() : id(""), x(0.0), y(0.0) {}
    Node(std::string id, double x, double y) : id(id), x(x), y(y) {}
    Node(const Node& p) : id(p.id), x(p.x), y(p.y) {}
};

struct Road {
    int id;
    int type;   // store road's type

    Road() : id(-1), type(0) {}
    Road(const Road& r) : id(r.id), type(r.type) {}
    Road(int id, int t) : id(id), type(t) {}
};

typedef adjacency_list<listS, listS, undirectedS, Node, Road> Map;
typedef graph_traits<Map>::vertex_descriptor vertex_descriptor;
typedef graph_traits<Map>::edge_descriptor edge_descriptor;
typedef std::pair<int, int> Edge;

typedef boost::bimap< std::string, int > bm_type;
typedef bm_type::value_type position;

std::vector<Node> nodes_;
std::map<int,float> roads_map_;
std::vector<Road> roads_;
std::vector<Edge> edges_; // XXX: an edge describes a road between two nodes.
Map* map_;

bm_type coordinate_id_bimap;
static int	node_count_bimap = 0; // start with 0

void print_status_of_graph(Map &g)
{
    graph_traits<Map>::vertex_iterator vi, vend;
    for (tie(vi, vend) = vertices(*map_); vi != vend; ++vi) {
//    	if(out_degree(*vi, g) > 0) {
			printf("vertex: , out_degree: %d\n",  out_degree(*vi, g));
			fgetc(stdin);
//    	}
    }
}

void make_coordinate_string(char* str, double x, double y)
{
	sprintf(str, "%010.7lf,%010.7lf", x, y);
	return;
}
void coordString_to_coordinate(const char* str, double &x, double &y)
{
	sscanf(str, "%lf,%lf", &x, &y);
	return;
}

int main() {

	char coord_str1[30];
	char coord_str2[30];
	double x,y;
	std::vector<std::string> tmp;
    // 1. Parse a JSON string into DOM.
    FILE* fp = fopen("c", "r");
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document d;
    d.ParseStream(is);
    fclose(fp);

    Value & features = d["features"];
    assert(features.IsArray());
	for (Value::ConstValueIterator itr = features.Begin(); itr != features.End(); ++itr)
	{//A feature is a road, Because we have filterd out the other features.
		assert((*itr)["geometry"]["coordinates"].IsArray());
		//Road
		Road r((*itr)["properties"]["osm_id"].GetInt(), 0);//TYPE UNDEFINED!!!!
		roads_.push_back(r);
		roads_map_.insert(std::make_pair((*itr)["properties"]["osm_id"].GetInt(), 0.0));//TYPE UNDEFINED!!!!
		for (Value::ConstValueIterator coitr = (*itr)["geometry"]["coordinates"].Begin(); coitr != (*itr)["geometry"]["coordinates"].End(); ++coitr)
		{//Road's coordinates combination
			assert((*coitr).IsArray());

			/**
			 * Node
			 */
			//Start node:
			//double s_d = (*coitr)[0].GetDouble();
			//int sid = (s_d - (int)s_d) * GPS_DECIMALS;
			x = (*coitr)[0].GetDouble();
			y = (*coitr)[1].GetDouble();
			make_coordinate_string(coord_str1, x, y);
			Node n1(coord_str1, x, y);

			//Prevent duplication nodes
			std::vector<std::string>::iterator res = find(tmp.begin(), tmp.end(), std::string(coord_str1));
			if(res == tmp.end()){
				nodes_.push_back(n1);
				tmp.push_back(std::string(coord_str1));
				//push into bimap
				coordinate_id_bimap.insert(position(coord_str1,  node_count_bimap++)); //start with 0
			}
			coitr++;
			if(coitr == (*itr)["geometry"]["coordinates"].End())
			{
				//printf("Reach the end\n");
				break;
			}
			//End node:
			//s_d = (*coitr)[0].GetDouble();
			//int eid = (s_d - (int)s_d) * GPS_DECIMALS;
			x = (*coitr)[0].GetDouble();
			y = (*coitr)[1].GetDouble();
			make_coordinate_string(coord_str2, x, y);
			Node n2(coord_str2, x, y);

			//Prevent duplication nodes #2
			res = find(tmp.begin(), tmp.end(), std::string(coord_str2));
			if(res == tmp.end()) {
				nodes_.push_back(n2);
				tmp.push_back(std::string(coord_str2));
				//push into bimap
				coordinate_id_bimap.insert(position(coord_str2,  node_count_bimap++));
			}

			/**
			 * Edge
			 */
			bm_type::left_iterator it = coordinate_id_bimap.left.find(coord_str1);
			int sid = it->second;
			it = coordinate_id_bimap.left.find(coord_str2);
			int eid = it->second;
			Edge e(sid, eid);
			edges_.push_back(e);
			printf("%d, %d\n", sid, eid);
			//printf("%.7f, %.7f\n", (*coitr)[0].GetDouble(), (*coitr)[1].GetDouble());
			coitr--;
			//printf("%d, %d\n", sid, eid);
			//fgetc(stdin);
		}
	}

	printf("Edge size: %d, Node size: %d, node_count_bimap: %d\n", edges_.size(), nodes_.size(), node_count_bimap);
	map_ = new Map(edges_.begin(), edges_.end(), roads_.begin(), nodes_.size());
    if (map_ == NULL) {
        printf("Out of memory\n");
        return -1;
    }
    printf("ttttttttttt\n");

    /**
     * put coordinates into map list.
     */
    graph_traits<Map>::vertex_iterator vi, vend;
    int i=0;
    for (tie(vi, vend) = vertices(*map_); vi != vend; ++vi, ++i) { //serial number starts with 0 !
    	bm_type::right_iterator it = coordinate_id_bimap.right.find(i);
    	coordString_to_coordinate((it->second).c_str(), x, y);
    	printf("%.7lf, %.7lf\n", x,y);
    	//fgetc(stdin);
        std::vector<Node>::iterator iter = nodes_.begin();
        for ( ; iter != nodes_.end(); ++iter)
            if (iter->id == it->second)
                break;
        (*map_)[*vi].id = iter->id;
        (*map_)[*vi].x = iter->x;
        (*map_)[*vi].y = iter->y;
    }
    printf("ttttttttttt\n");
}

int out_degree_from_id(std::string node_id, Map & g, vertex_descriptor & u)
{
	bm_type::left_iterator it = coordinate_id_bimap.left.find(node_id);
	if (it != coordinate_id_bimap.left.end()) {
		u = vertex(it->second, *map_);
		return out_degree(u, g);
	}

    return -1;
}

void remove_vertex_from_graph(std::string node_id, Map & g)
{
	bm_type::left_iterator it = coordinate_id_bimap.left.find(node_id);
	assert(it != coordinate_id_bimap.left.end());
	vertex_descriptor u = vertex(it->second, g);
	clear_vertex(u, g);
	remove_vertex(u, g);
	//Update bimap
	coordinate_id_bimap.left.erase(node_id);
	for(int pos = it->second + 1; pos < node_count_bimap; pos++) {
	    bm_type::right_iterator it = coordinate_id_bimap.right.find(pos);
		assert(it != coordinate_id_bimap.right.end());
	    bool successful_modify = coordinate_id_bimap.right.modify_key( it, boost::bimaps::_key = (pos-1) );
	    assert(successful_modify);
	}
	node_count_bimap--;
	return;
}
/**
 * Remove the data of a tile from the map list.
 * "Document" is a class from Rapidjson. The tile data need to be filtered first.
 */
void remove_data_from_list(Document ** data_del, int n_del, Document ** data_hold, int n_hold, Map & g)
{
	//1. Import road id set from data_hold.
	std::vector<int> road_id_set;
	std::vector<int> remove_road_id_set;
	std::vector<std::string> remove_node_id_set;
	char coordStr[30];

	for(int i=0; i< n_hold; i++) {
		for (Value::ConstValueIterator itr = (*data_hold[i]).Begin(); itr != (*data_hold[i]).End(); ++itr) {
			road_id_set.push_back((*itr)["properties"]["osm_id"].GetInt());
		}
	}
	//2. Find out roads need to be removed.
	for(int i=0; i< n_del; i++) {
		for (Value::ConstValueIterator itr = (*data_del[i]).Begin(); itr != (*data_del[i]).End(); ++itr) {
			int id = (*itr)["properties"]["osm_id"].GetInt();
			std::vector<int>::iterator res = find( road_id_set.begin(), road_id_set.end(), id);
			if ( res == road_id_set.end() )
				remove_road_id_set.push_back(id);
		}
	}

	//3. Find out which Nodes need to be remove from adjacency list
	//    need to check the degree of the vertex,
	for(int i=0; i< n_del; i++) {
		for (Value::ConstValueIterator itr = (*data_del[i]).Begin(); itr != (*data_del[i]).End(); ++itr) {
			int id = (*itr)["properties"]["osm_id"].GetInt();
			std::vector<int>::iterator res = find( remove_road_id_set.begin(), remove_road_id_set.end(), id);
			if ( res == remove_road_id_set.end())
				continue;

			//Check the degree of the vertex
			for (Value::ConstValueIterator coitr = (*itr)["geometry"]["coordinates"].Begin(); coitr != (*itr)["geometry"]["coordinates"].End(); ++coitr)
			{//Road's coordinates combination
				//get ID
				make_coordinate_string(coordStr, (*coitr)[0].GetDouble(), (*coitr)[1].GetDouble() );
				int degree;
				vertex_descriptor u;
				if(coitr == (*itr)["geometry"]["coordinates"].Begin() || coitr == (*itr)["geometry"]["coordinates"].End())
				{
					degree = out_degree_from_id(coordStr, g, u);
					if(degree == 1) {
						//remove_vertex_from_graph(coordStr, g);
						remove_node_id_set.push_back(coordStr);
					} else if (degree <= 0){
						printf("out_degree_from_id fatal error !!\n");
						exit(0);
					}
				} else {
					degree = out_degree_from_id(coordStr, g, u);
					if (degree <= 0){
						printf("out_degree_from_id fatal error !!\n");
						exit(0);
					} else if(degree <= 2) {
						//remove_vertex_from_graph(coordStr, g);
						remove_node_id_set.push_back(coordStr);
					} 
				}
			}

			//Remove road form roads_map_
			roads_map_.erase(id);
		}
	}

	//4. Remove Nodes from adjacency list
	for(std::vector<std::string>::iterator res = remove_node_id_set.begin();
			res != remove_node_id_set.end(); res++) {
		remove_vertex_from_graph(*res, g);
	}
}

void add_data_to_list(Document ** data_add, int n_add, Map & g)
{
	/**
	 * Find out roads need to be removed, and then remove it!!
	 */
	double x,y;
	char coordStr1[30];
	char coordStr2[30];
	int sid,eid;
	for(int i=0; i< n_add; i++) {
		for (Value::ConstValueIterator itr = (*data_add[i]).Begin(); itr != (*data_add[i]).End(); ++itr) {
			int id = (*itr)["properties"]["osm_id"].GetInt();
			std::map<int, float>::iterator rit = roads_map_.find(id);
			if(rit != roads_map_.end())
				continue; //this road is duplicated

			//Update roads_map_
			roads_map_.insert(std::make_pair(id, 0.0)); //TYPE UNDEFINED!!!!

			for (Value::ConstValueIterator coitr = (*itr)["geometry"]["coordinates"].Begin(); coitr != (*itr)["geometry"]["coordinates"].End(); ++coitr)
			{//Road's coordinates combination
				assert((*coitr).IsArray());

				//Start node:
				x = (*coitr)[0].GetDouble();
				y = (*coitr)[1].GetDouble();
				make_coordinate_string(coordStr1, x, y);

				bm_type::left_iterator it = coordinate_id_bimap.left.find(coordStr1);
				if(it == coordinate_id_bimap.left.end()) {
					//push into bimap
					coordinate_id_bimap.insert(position(coordStr1,  node_count_bimap));
					sid = node_count_bimap++;
				} else {
					sid = it->second;
				}

				if(++coitr == (*itr)["geometry"]["coordinates"].End())
					break;

				//End node:
				x = (*coitr)[0].GetDouble();
				y = (*coitr)[1].GetDouble();
				make_coordinate_string(coordStr2, x, y);
				it = coordinate_id_bimap.left.find(coordStr2);
				if(it == coordinate_id_bimap.left.end()) {
					//push into bimap
					coordinate_id_bimap.insert(position(coordStr2,  node_count_bimap));
					eid = node_count_bimap++;
				} else {
					eid = it->second;
				}

				//Add an edge
				add_edge(vertex(sid, g), vertex(eid, g), g);
				
				coitr--;
			}
		}


	}
}
