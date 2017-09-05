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

#include "obstacle.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("obstacle");

Obstacle::Obstacle() :
  // default values for per-wall and per-meter attenuation, 
  // (per obstacle instance)
  // See C. Sommer et. al.:
  // A Computationally Inexpensive Empirical Model of IEEE 802.11p
  // Radio Shadowing in Urban Environments;  
  m_beta(9.0),
  m_gamma(0.4)
{
  NS_LOG_FUNCTION (this);
}

void
Obstacle::SetId(std::string id)
{
  NS_LOG_FUNCTION (this);

  this->m_id = id;
}

const std::string
Obstacle::GetId()
{
  NS_LOG_FUNCTION (this);

  return m_id;
}

Polygon_2 &
Obstacle::GetPolygon()
{
  NS_LOG_FUNCTION (this);

  return m_obstacle;
}

void
Obstacle::AddVertex(Point p)
{
  NS_LOG_FUNCTION (this);

  // add vertex to internal CGAL Polygon_2 object
  m_obstacle.push_back(p);
}

void
Obstacle::Locate()
{
  NS_LOG_FUNCTION (this);

  // get the bounding box of the obstacle
  Bbox_2 bbox = m_obstacle.bbox();

  double bx = (double)bbox.xmin();
  double by = (double)bbox.ymin();
  // get the centerpoint
  double cx = (double)(bx + (bbox.xmax() - bx));
  double cy = (double)(by + (bbox.ymax() - by));

  m_center = Point(cx, cy);

  m_radiusSq = (cx - bx) * (cx - bx) + (cy - by) * (cy - by);
}

const Point &
Obstacle::GetCenter()
{
  NS_LOG_FUNCTION (this);

  return m_center;
}

double
Obstacle::GetRadiusSq()
{
  NS_LOG_FUNCTION (this);

  return m_radiusSq;
}

double
Obstacle::GetBeta()
{
  NS_LOG_FUNCTION (this);

  return m_beta;
}

double
Obstacle::GetGamma()
{
  NS_LOG_FUNCTION (this);

  return m_gamma;
}

void
Obstacle::SetBeta(double beta)
{
  NS_LOG_FUNCTION (this);

  m_beta = beta;
}

void
Obstacle::SetGamma(double gamma)
{
  NS_LOG_FUNCTION (this);

  m_gamma = gamma;
}

}

