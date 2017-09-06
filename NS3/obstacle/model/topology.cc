/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 North Carolina State University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Scott E. Carpenter <scarpen@ncsu.edu>
 *
 */

#include "topology.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("topology");

Topology::Topology () : 
  // initially very large values
  // so that obstacle bounding box
  // updates will be made
  m_minX(999999999.0),
  m_minY(999999999.0),
  m_maxX(-999999999.0),
  m_maxY(-999999999.0)
{
  NS_LOG_FUNCTION (this);
}

void Topology::CommandSetup (int argc, char **argv)
{
  NS_LOG_FUNCTION (this);

  CommandLine cmd;

  cmd.Parse (argc, argv);
}

void
Topology::Run ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_UNCOND ("Topology::Run()");
}

Topology **
Topology::PeekTopology (void)
{
  // ensure no topology exists
  static Topology *topo = 0;
  return &topo;
}

Topology * 
Topology::GetTopology (void)
{
  Topology **ptopo = PeekTopology ();
  /* Please, don't include any calls to logging macros in this function
   * or pay the price, that is, stack explosions.
   */
  if (*ptopo == 0)
    {
      // create the topology
      *ptopo = new Topology();
    }

  return *ptopo;
}

void 
Topology::
CreateVertex(Obstacle &obstacle, std::string vertex) 
{
  NS_LOG_FUNCTION (this);

  // x, y valus are comma-separated
  size_t pos = vertex.find(",");
  std::string x = vertex.substr(0, pos);
  std::string y = vertex.substr(pos + 1);

  // convert values to double
  double dx = atof(x.c_str ());
  double dy = atof(y.c_str ());

  // create a 2D x,y point
  Point p(dx, dy);
  // add the point as a vertex to an obstacle
  obstacle.AddVertex(p);

  // if possible, update topology bounding box values
  if (dx < m_minX)
    {
      m_minX = dx;
    }
  if (dx > m_maxX)
    {
      m_maxX = dx;
    }
  if (dy < m_minY)
    {
      m_minY = dy;
    }
  if (dy > m_maxY)
    {
      m_maxY = dy;
    }
}

void 
Topology::
CreateShape(std::string id, std::string vertices) 
{
  // create an obstacle
  Obstacle obstacle;
  // name the obstacle
  obstacle.SetId(id);

  // tokenize each vertex
  size_t pos1 = 0;
  size_t pos2 = vertices.find(" ", pos1);
  std::string vertex;
  while (pos2 != std::string::npos) 
    {
      vertex = vertices.substr(pos1, pos2-pos1);
      CreateVertex(obstacle, vertex);
      pos1 = pos2 + 1;
      pos2 = vertices.find(" ", pos1);
    }
  // NOTE:  In OSM data, last vertex is dup of first
  // so, we don't need to get it for polygonal obstacle
  // get last vertex

  // calculate the obstacle center of 
  // bounding box and radius(squared).
  obstacle.Locate();

  // load centerpoint into Range Tree
  Point c = obstacle.GetCenter();
  Key k = Key(c, obstacle);

  // add the obstacle to the topolgoy
  m_obstacles.push_back(k);
}

// range tree (binary space partition, BSP) 
// for quickly searching for obstacles within a range
static Range_tree_2_type m_rangeTree;

void 
Topology::LoadBuildings(std::string bldgFilename)
{
  NS_LOG_UNCOND ("Load buildings");
  std::ifstream file (bldgFilename.c_str (), std::ios::in);
  if (!(file.is_open ())) 
    {
      NS_FATAL_ERROR("Could not open buildings file " << bldgFilename.c_str() << " for reading, aborting here \n"); 
    }
  else
    {
      Topology * topology = Topology::GetTopology();
      NS_ASSERT(topology != 0);

      NS_LOG_UNCOND ("Reading file: " << bldgFilename);
      while (!file.eof () )
        {
          std::string line;

          getline (file, line);

          NS_LOG_UNCOND (line);

          size_t posB = line.find("type=\"building");
          size_t posU = line.find("type=\"unknown");
          if ((posB != std::string::npos)
              || (posU != std::string::npos))
            {
              // could *possibly* use XML-DOM here, but 
              // seems faster to just read through the file

              // found a building
              // get the building id (name) and shape (vertices)
              std::string polyid;
              std::string shape;
              size_t pos = line.find("<poly id=\"");
              if (pos != std::string::npos)
                {
                  size_t pos2 = line.find("\"", pos + 11);
                  if (pos2 != std::string::npos)
                    {
                      polyid = line.substr(pos + 10, pos2 - pos - 10);
                      pos = line.find("shape=\"");
                      if (pos != std::string::npos)
                      {
                        size_t pos2 = line.find("\"", pos + 8);
                        if (pos2 != std::string::npos)
                        {
                          shape = line.substr(pos + 7, pos2 - pos - 7);
                        }
                      }
                      topology->CreateShape(polyid, shape);      
                    }
                }
            }
        }
      NS_LOG_UNCOND ("Topology buildings bounded by x:" << topology->GetMinX() << "," << topology->GetMaxX() << " y:" << topology->GetMinY() << "," << topology->GetMaxY());
      // all obstacles have been loaded
      // so now create a searchable range tree based on those obstacles
      topology->MakeRangeTree();
    }
}

void
Topology::MakeRangeTree()
{
  NS_LOG_FUNCTION (this);

  m_rangeTree.make_tree(m_obstacles.begin(), m_obstacles.end());
}

void
Topology::GetObstructedDistance(const Point &p1, const Point &p2, Obstacle &obs, double & obstructedDistance, int &intersections)
{
  NS_LOG_FUNCTION (this);

  // initialize values as if no intersections found
  obstructedDistance = 0.0;
  intersections = 0;

  // construct a segment, r, from p1 to p2
  // test r for intersection with each edge of obs
  // calculate distance from p1 to nearest (d_min)  and farthest (d_max) intersecting edge
  // obstruction distance is then estimated by d_max - d_min

  double d_min = 999999999.0;
  double d_max = -999999999.0;

  Segment_2 r(p1, p2);

  Polygon_2 &poly = obs.GetPolygon();  
  for (EdgeIterator iterEdge = poly.edges_begin(); iterEdge != poly.edges_end(); ++iterEdge) 
    {
      Segment_2 s = (Segment_2) (*iterEdge);

      CGAL::Object result = CGAL::intersection(s, r);

      if (const CGAL::Point_2<K> *ipoint = CGAL::object_cast<CGAL::Point_2<K> >(
		&result)) 
        {
          intersections++;

          // handle the point intersection case with *ipoint.
          double dx = CGAL::to_double(ipoint->x()) - CGAL::to_double(p1.x());
          double dy = CGAL::to_double(ipoint->y()) - CGAL::to_double(p1.y());
          // square it, for performance optimization
          // (there will be many compares to this vaule)
          double distP1toIPsq = dx * dx + dy * dy;

          if (distP1toIPsq < d_min) 
            {
              d_min = distP1toIPsq;
            }
          if (distP1toIPsq > d_max) 
            {
              d_max = distP1toIPsq;
            }
        } 
      /*
      else if (const CGAL::Segment_2<K> *iseg = CGAL::object_cast<
		CGAL::Segment_2<K> >(&result)) 
        {
          // handle the segment intersection case with *iseg.
          //cout << "s and rho intersect at a segment " << *iseg << endl;
        } 
      else 
        {
          // handle the no intersection case.
          //NS_LOG_UNCOND ("s and rho do not intersect");
          //cout << "s and rho do not intersect." << endl;
        }
      */

    }

  if ((d_min < 999999999.0) && (d_max > 0)) 
    {
      // if d_min == d_max, then intersection was at a vertex of an edge
      if (d_min == d_max)
        {
          // intersection was at a vertex
        }
      else
        {
          double d1 = sqrt(d_min);
          double d2 = sqrt(d_max);
          obstructedDistance = d2 - d1;
        }
    }
}

double 
Topology::GetObstructedLossBetween(const Point &p1, const Point &p2, double r, bool forbeacon, bool isSub1G)
{
  NS_LOG_FUNCTION (this);

  // initially assume no loss
  double obstructedLoss = 0.0;

//  int countFound = 0;

  double rSq = r * r;

  double p1x = CGAL::to_double(p1.x());
  double p1y = CGAL::to_double(p1.y());
  double p2x = CGAL::to_double(p2.x());
  double p2y = CGAL::to_double(p2.y());

  // test first to see if we have a cached value
  // for loss between these two points
  // using their positions to the nearest 0.1m

  char buff[100];
  sprintf(buff, "%010.1f %010.1f", p1x, p1y);
  std::string p1Pos = buff;
  sprintf(buff, "%010.1f %010.1f", p2x, p2y);
  std::string p2Pos = buff;
  sprintf(buff, "%s %s", p1Pos.c_str(), p2Pos.c_str());
  std::string key = buff;

  // check if cached
  if (m_obstructedDistanceMap.count(key) > 0) 
    {
      // found it
      TStrDblMap::iterator it;
      it = m_obstructedDistanceMap.find(key);
      obstructedLoss = it->second;
      return obstructedLoss;
    }
  else
    {
      // B to A is same as A to B
      sprintf(buff, "%s %s", p2Pos.c_str(), p1Pos.c_str());
      std::string key = buff;
      if (m_obstructedDistanceMap.count(key) > 0) 
        {
          // found it
          TStrDblMap::iterator it;
          it = m_obstructedDistanceMap.find(key);
          obstructedLoss = it->second;
          return obstructedLoss;
        }
    }

  // optimization
  // only if dist between p1 and p2 < 2r
  double dx = p2x - p1x;
  double dy = p2y - p1y;
  double distP1toP2sq = dx * dx + dy * dy;
  // distance must be less then (2r)^2 = 4r^2
  double x4rSq = 4.0 * rSq;
  if (distP1toP2sq < x4rSq)
    {
      // now search by range tree search
      // get bounding box, and extend by r is all directions
      double xmin = std::min(p1x, p2x) - r;
      double xmax = std::max(p1x, p2x) + r;
      double ymin = std::min(p1y, p2y) - r;
      double ymax = std::max(p1y, p2y) + r;
      Point pLow(xmin, ymin);
      Point pHigh(xmax, ymax);
      Interval win(Interval(pLow, pHigh));
      m_outputList.clear();
      m_rangeTree.window_query(win, std::back_inserter(m_outputList));
      std::vector<Key>::iterator current = m_outputList.begin();
      while (current != m_outputList.end())
        {
          Obstacle obstacle = (*current).second;
//          std::string id = obstacle.GetId();
//          Point center = obstacle.GetCenter();

//          double dx1 = CGAL::to_double(center.x()) - p1x;
//          double dy1 = CGAL::to_double(center.y()) - p1y;
//          double distCtoP1sq = dx1 * dx1 + dy1 * dy1;
//
//          double dx2 = CGAL::to_double(center.x()) - p2x;
//          double dy2 = CGAL::to_double(center.y()) - p2y;
//          double distCtoP2sq = dx2 * dx2 + dy2 * dy2;
//
//          double t1 = distCtoP1sq - rSq;
//          double t2 = distCtoP2sq - rSq;
////          if (((distCtoP1sq - rSq) < 0)
////              && ((distCtoP2sq - rSq) < 0))
//          if (t1 < 0 && t2 < 0)
//            {
              // obtstacle is within range
//              countFound++;

              double obstructedDistanceBetween = 0.0;
              int intersections = 0;
              GetObstructedDistance(p1, p2, obstacle, obstructedDistanceBetween, intersections);
              // From C. Sommer et. al.:
              // A Computationally Inexpensive Empirical Model of IEEE 802.11p
              // Radio Shadowing in Urban Environments, 2011.
              // Additional loss due to propagation through obstacles:
              // Lobs = beta x n + gamma x d_m
              // Where
              // Lobs is the additional loss
              // beta is a (constant) factor for the obstacle type
              // n is the number of intersections through the obstacle
              // gamma is a (constant) factor for the obstacle type
              // d_m is the distance in meters of propagation through the obstacle
              if ((obstructedDistanceBetween > 0.0) && (intersections > 1))
                {
                  double beta = obstacle.GetBeta();
                  double gamma = obstacle.GetGamma();
                  if (forbeacon)
                    {
                      if (isSub1G)
                	{
			  beta = beta/4;
			  gamma = gamma/4;
                	}
                      else
                	{
			  beta = beta/2;
			  gamma = gamma/2;
                	}
                    }

                  obstructedLoss += beta * (double) intersections + gamma * obstructedDistanceBetween;
                              
                }
//            }
          current++;
        }
    }

  else
    obstructedLoss = 9999;

  // cache results
  if (m_obstructedDistanceMap.size() > 1000) 
    {
      // clear it every once in a while, to avoid bloat.
      m_obstructedDistanceMap.clear();
    }
  m_obstructedDistanceMap.insert(TStrDblPair(key, obstructedLoss));

  return obstructedLoss;
}

double
Topology::GetMinX()
{
  NS_LOG_FUNCTION (this);

  return m_minX;
}

double
Topology::GetMaxX()
{
  NS_LOG_FUNCTION (this);

  return m_maxX;
}

double
Topology::GetMinY()
{
  NS_LOG_FUNCTION (this);

  return m_minY;
}

double
Topology::GetMaxY()
{
  NS_LOG_FUNCTION (this);

  return m_maxY;
}

bool 
Topology::HasObstacles()
{
  NS_LOG_FUNCTION (this);

  bool hasObstacles = false;

  if (m_obstacles.size() > 0)
    {
      hasObstacles = true;
    }

  return hasObstacles;
}
