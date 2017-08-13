/*
 * mbr_load_sumomap.h
 *
 *  Created on: 2017年5月28日
 *      Author: wu
 */
#ifndef MBR_LOAD_SUMOMAP_H_
#define MBR_LOAD_SUMOMAP_H_

#include <string.h>
#include <map>
#include <vector>
#include <string>
#include "graph.h"
#include "tinyxml2.h"

namespace ns3 {
namespace mbr {

#define LAT_RANGE_MIN 39.74732
#define LAT_RANGE_MAX 40.15929
#define LON_RANGE_MIN 116.16677
#define LON_RANGE_MAX 116.73407



typedef struct {
	double x,y;
	uint64_t geohash;
	std::string id;
} node;

typedef struct {
	std::string roadid;
	std::string fromid;
	std::string toid;
}edge;

typedef struct {
	double conv_x1,conv_y1,conv_x2,conv_y2;
	double orig_x1,orig_y1,orig_x2,orig_y2;
	double netoffset_x, netoffset_y;
	std::string projParameter;
} mapboundary;

class MbrSumo {

public:
	void sumoCartesian2GPS(double input_x, double input_y,
			double *output_x, double *output_y);
	uint64_t sumoCartesian2Geohash(double input_x, double input_y);
	Graph* loadSumoMap(std::string sumoMapFilename);
	/**
	* \brief Gets the topology instance
	* \return the topology instance
	*/
	static MbrSumo * GetInstance();
private:
	static MbrSumo * p;
	mapboundary m_bound;
	//graph
	Graph *m_graph;
	//private constructor
	MbrSumo(){};
	void Tokenize(const std::string& str,
			std::vector<std::string>& tokens,
	        const std::string& delimiters);
	std::string parseRoadid(std::string str);
	void parseBoundary(tinyxml2::XMLElement *location);
	void parseShapeAndUpdateGraph(const char *fromid, const char *toid, const char *roadid, std::string shape);

};

}
}
#endif /* MBR_LOAD_SUMOMAP_H_ */

