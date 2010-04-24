/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2010, Willow Garage, Inc.
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redstributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Tim Field */

#ifndef COLLADA_URDF_COLLADA_URDF_H
#define COLLADA_URDF_COLLADA_URDF_H

#include <string>

#include <boost/shared_ptr.hpp>

#include <dae.h>

#include "urdf/model.h"

namespace collada_urdf {

/** Constructs a COLLADA DOM from a file, given the filename
 * \param file The filename from where to read the XML
 * \param dom The resulting COLLADA DOM
 * \return true on success, false on failure
 */
bool colladaFromFile(std::string const& file, boost::shared_ptr<DAE>& dom);

/** Constructs a COLLADA DOM from a string containing XML
 * \param xml A string containing the XML description of the robot
 * \param dom The resulting COLLADA DOM
 * \return true on success, false on failure
 */
bool colladaFromString(std::string const& xml, boost::shared_ptr<DAE>& dom);

/** Constructs a COLLADA DOM from a TiXmlDocument
 * \param xml_doc The TiXmlDocument containing the XML description of the robot
 * \param dom The resulting COLLADA DOM
 * \return true on success, false on failure
 */
bool colladaFromXml(TiXmlDocument* xml_doc, boost::shared_ptr<DAE>& dom);

/** Constructs a COLLADA DOM from a URDF robot model
 * \param robot_model The URDF robot model
 * \param dom The resulting COLLADA DOM
 * \return true on success, false on failure
 */
bool colladaFromUrdfModel(urdf::Model const& robot_model, boost::shared_ptr<DAE>& dom);

}

#endif
