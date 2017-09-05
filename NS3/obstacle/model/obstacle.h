/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 North Carolina State University
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

#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <fstream>
#include <iostream>
#include <map>
#include "ns3/core-module.h"

// CGAL includes
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Ray_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Range_segment_tree_traits.h>
#include <CGAL/Range_tree_k.h>

// CGAL types
typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef K::FT Coord_type;
typedef CGAL::Ray_2<K> Ray_2;
typedef CGAL::Point_2<K> Point;
typedef CGAL::Segment_2<K> Segment_2;
typedef CGAL::Polygon_2<K> Polygon_2;
typedef CGAL::Bbox_2 Bbox_2;
typedef CGAL::Polygon_2<K>::Edge_const_iterator EdgeIterator;

namespace ns3 {
/**
 * \ingroup obstacle
 * \brief The Obstacle class maintains the information for a 
 * polygon representation of an obstacle and per-wall and per-meter
 * attenuation values.
 */
class Obstacle
{
public:
  /**
   * \brief Constructor
   * \return none
   */
  Obstacle ();

  /**
   * \brief Sets the id (i.e., name) of the obstacle
   * \param id the identifier (i.e., name) of the obstacle
   * \return none
   */
  void SetId(std::string id);

  /**
   * \brief Gets the id (i.e., name) of the obstacle
   * \return the id (i.e., name) of the obstacle
   */
  const std::string GetId();

  /**
   * \brief Adds a vertex to the obstacle
   * \param p the vertex (a CGAL Point)
   * \return none
   */
  void AddVertex(Point p);

  /**
   * \brief Calculates the location of the centerpoint
   * of the bounding box of the polygon, used for
   * optimization of searching for obstacles.  To be 
   * called after the last vertex is added to the 
   * obstacle (so that the centerpoint can then be located)
   * \return none
   */
  void Locate();

  /**
   * \brief Get the centerpoint of the obstacle
   * \return the centerpoint (i.e., midpoint of longest
   * ray between vertices that traverses the interior)
   */
  const Point &GetCenter();

  /**
   * \brief Gets the radius of the obstacles region
   * (squared for performance optimizations)
   * \return the radius (squared) =
   * (1/2 of longest interior ray) ^ 2
   */
  double GetRadiusSq();

  /**
   * \brief Gets the polygonal region that defines the obstacle
   * \return The polygonal region defining the obstacle
   * (i.e., a CGAL Polygon_2)
   */
  Polygon_2 &GetPolygon();

  /**
   * \brief Gets the value of beta, the per-wall 
   * attenuation parameter
   * \return beta, the per-wall attenuation parameter
   */
  double GetBeta();

  /**
   * \brief Gets the value of gamma, the per-meter
   * attenuation parameter
   * \return gamma, the per-meter attenuation parameter
   */
  double GetGamma();

  /**
   * \brief Sets the value of beta, the per-wall
   * attenuation parameter
   * \param beta, the per-meter attenuation parameter
   * \return none
   */
  void SetBeta(double beta);

  /**
   * \brief Sets the value of gamma, the per-meter
   * attenuation parameter
   * \param gamma, the per-meter attenuation parameter
   * \return none
   */
  void SetGamma(double gamma);

private:

  // the obstacle identifier (i.e., name)
  std::string m_id;

  // 2D polygonal represenation of the obsstacle (i.e., a CGAL Polygon_2)
  Polygon_2 m_obstacle;

  // centerpoint of obstacle bounding box
  // i.e., the midpoint of the longest ray between vertices that
  // traverses the interior of the polygon.  used for search optimizations)
  Point m_center;

  // radius squared from centerpoint to bounding box vertex
  double m_radiusSq;

  // For IEEE 802.11p propagation loss through obstacles
  // See C. Sommer et. al.:
  // A Computationally Inexpensive Empirical Model of IEEE 802.11p
  // Radio Shadowing in Urban Environments;
  double m_beta;  // per-wall attenuation parameter
  double m_gamma; // per-meter attenuation parameter
};

}

#endif /* OBSTACLE_H */

