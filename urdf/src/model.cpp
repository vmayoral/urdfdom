/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
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

/* Author: Wim Meeussen */

#include <boost/algorithm/string.hpp>
#include <ros/ros.h>
#include <vector>
#include "urdf/model.h"

namespace urdf{


Model::Model()
{
  this->clear();
}

void Model::clear()
{
  name_.clear();
  this->links_.clear();
  this->joints_.clear();
  this->materials_.clear();
  this->root_link_.reset();
}


bool Model::initFile(const std::string& filename)
{
  TiXmlDocument xml_doc;
  xml_doc.LoadFile(filename);

  return initXml(&xml_doc);
}


bool Model::initParam(const std::string& param)
{
  ros::NodeHandle nh;
  std::string xml_string;
  
  // gets the location of the robot description on the parameter server
  std::string full_param;
  if (!nh.searchParam(param, full_param)){
    ROS_ERROR("Could not find parameter %s on parameter server", param.c_str());
    return false;
  }

  // read the robot description from the parameter server
  if (!nh.getParam(full_param, xml_string)){
    ROS_ERROR("Could read parameter %s on parameter server", full_param.c_str());
    return false;
  }
  return initString(xml_string);
}


bool Model::initString(const std::string& xml_string)
{
  TiXmlDocument xml_doc;
  xml_doc.Parse(xml_string.c_str());

  return initXml(&xml_doc);
}


bool Model::initXml(TiXmlDocument *xml_doc)
{
  if (!xml_doc)
  {
    ROS_ERROR("Could not parse the xml");
    return false;
  }

  TiXmlElement *robot_xml = xml_doc->FirstChildElement("robot");
  if (!robot_xml)
  {
    ROS_ERROR("Could not find the 'robot' element in the xml file");
    return false;
  }
  return initXml(robot_xml);
}

bool Model::initXml(TiXmlElement *robot_xml)
{
  this->clear();

  ROS_DEBUG("Parsing robot xml");
  if (!robot_xml) return false;

  // Get robot name
  const char *name = robot_xml->Attribute("name");
  if (!name)
  {
    ROS_ERROR("No name given for the robot.");
    return false;
  }
  this->name_ = std::string(name);

  // Get all Material elements
  for (TiXmlElement* material_xml = robot_xml->FirstChildElement("material"); material_xml; material_xml = material_xml->NextSiblingElement("material"))
  {
    boost::shared_ptr<Material> material;
    material.reset(new Material);

    if (material->initXml(material_xml))
    {
      if (this->getMaterial(material->name))
      {
        ROS_ERROR("material '%s' is not unique.", material->name.c_str());
        material.reset();
        return false;
      }
      else
      {
        this->materials_.insert(make_pair(material->name,material));
        ROS_DEBUG("successfully added a new material '%s'", material->name.c_str());
      }
    }
    else
    {
      ROS_ERROR("material xml is not initialized correctly");
      material.reset();
      return false;
    }
  }

  // Get all Link elements
  for (TiXmlElement* link_xml = robot_xml->FirstChildElement("link"); link_xml; link_xml = link_xml->NextSiblingElement("link"))
  {
    boost::shared_ptr<Link> link;
    link.reset(new Link);

    if (link->initXml(link_xml))
    {
      if (this->getLink(link->name))
      {
        ROS_ERROR("link '%s' is not unique.", link->name.c_str());
        link.reset();
        return false;
      }
      else
      {
        // set link visual material
        ROS_DEBUG("setting link '%s' material", link->name.c_str());
        if (link->visual)
        {
          if (!link->visual->material_name.empty())
          {
            if (this->getMaterial(link->visual->material_name))
            {
              ROS_DEBUG("setting link '%s' material to '%s'", link->name.c_str(),link->visual->material_name.c_str());
              link->visual->material = this->getMaterial( link->visual->material_name.c_str() );
            }
            else
            {
              if (link->visual->material)
              {
                ROS_DEBUG("link '%s' material '%s' defined in Visual.", link->name.c_str(),link->visual->material_name.c_str());
                this->materials_.insert(make_pair(link->visual->material->name,link->visual->material));
              }
              else
              {
                ROS_ERROR("link '%s' material '%s' undefined.", link->name.c_str(),link->visual->material_name.c_str());
                link.reset();
                return false;
              }
            }
          }
        }

        this->links_.insert(make_pair(link->name,link));
        ROS_DEBUG("successfully added a new link '%s'", link->name.c_str());
      }
    }
    else
    {
      ROS_ERROR("link xml is not initialized correctly");
      link.reset();
      return false;
    }
  }
  if (this->links_.empty()){
    ROS_ERROR("No link elements found in urdf file");
    return false;
  }

  // Get all Joint elements
  for (TiXmlElement* joint_xml = robot_xml->FirstChildElement("joint"); joint_xml; joint_xml = joint_xml->NextSiblingElement("joint"))
  {
    boost::shared_ptr<Joint> joint;
    joint.reset(new Joint);

    if (joint->initXml(joint_xml))
    {
      if (this->getJoint(joint->name))
      {
        ROS_ERROR("joint '%s' is not unique.", joint->name.c_str());
        joint.reset();
        return false;
      }
      else
      {
        this->joints_.insert(make_pair(joint->name,joint));
        ROS_DEBUG("successfully added a new joint '%s'", joint->name.c_str());
      }
    }
    else
    {
      ROS_ERROR("joint xml is not initialized correctly");
      joint.reset();
      return false;
    }
  }


  // every link has children links and joints, but no parents, so we create a
  // local convenience data structure for keeping child->parent relations
  std::map<std::string, std::string> parent_link_tree;
  parent_link_tree.clear();

  // building tree: name mapping
  if (!this->initTree(parent_link_tree))
  {
    ROS_ERROR("failed to build tree");
    return false;
  }

  // find the root link
  if (!this->initRoot(parent_link_tree))
  {
    ROS_ERROR("failed to find root link");
    return false;
  }
 
  return true;
}

bool Model::initTree(std::map<std::string, std::string> &parent_link_tree)
{
  // loop through all joints, for every link, assign children links and children joints
  for (std::map<std::string,boost::shared_ptr<Joint> >::iterator joint = this->joints_.begin();joint != this->joints_.end(); joint++)
  {
    std::string parent_link_name = joint->second->parent_link_name;
    std::string child_link_name = joint->second->child_link_name;

    ROS_DEBUG("build tree: joint: '%s' has parent link '%s' and child  link '%s'", joint->first.c_str(), parent_link_name.c_str(),child_link_name.c_str());
    if (parent_link_name.empty() || child_link_name.empty())
    {
      ROS_ERROR("    Joint %s is missing a parent and/or child link specification.",(joint->second)->name.c_str());
      return false;
    }
    else
    {
      // find child and parent links
      boost::shared_ptr<Link> child_link, parent_link;
      this->getLink(child_link_name, child_link);
      if (!child_link)
      {
        ROS_ERROR("    child link '%s' of joint '%s' not found", child_link_name.c_str(), joint->first.c_str() );
        return false;
      }
      this->getLink(parent_link_name, parent_link);
      if (!parent_link)
      {
        ROS_ERROR("    parent link '%s' of joint '%s' not found.  The Boxturtle urdf parser used to automatically add this link for you, but this is not valid according to the URDF spec. Every link you refer to from a joint needs to be explicitly defined in the robot description. To fix this problem you can either remove this joint from your urdf file, or add \"<link name=\"%s\" />\" to your urdf file.", parent_link_name.c_str(), joint->first.c_str(), parent_link_name.c_str() );
        return false;
      }
      
      //set parent link for child link
      child_link->setParent(parent_link);

      //set parent joint for child link
      child_link->setParentJoint(joint->second);

      //set child joint for parent link
      parent_link->addChildJoint(joint->second);
      
      //set child link for parent link
      parent_link->addChild(child_link);
      
      // fill in child/parent string map
      parent_link_tree[child_link->name] = parent_link_name;
      
      ROS_DEBUG("    now Link '%s' has %i children ", parent_link->name.c_str(), (int)parent_link->child_links.size());
    }
  }

  return true;
}



bool Model::initRoot(std::map<std::string, std::string> &parent_link_tree)
{

  this->root_link_.reset();

  // find the links that have no parent in the tree
  for (std::map<std::string, boost::shared_ptr<Link> >::iterator l=this->links_.begin(); l!=this->links_.end(); l++)  
  {
    std::map<std::string, std::string >::iterator parent = parent_link_tree.find(l->first);
    if (parent == parent_link_tree.end())
    {
      // store root link
      if (!this->root_link_)
      {
         getLink(l->first, this->root_link_);
      }
      // we already found a root link
      else{
        ROS_ERROR("Two root links found: '%s' and '%s'", this->root_link_->name.c_str(), l->first.c_str());
        return false;
      }
    }
  }
  if (!this->root_link_)
  {
    ROS_ERROR("No root link found. The robot xml is not a valid tree.");
    return false;
  }
  ROS_DEBUG("Link '%s' is the root link", this->root_link_->name.c_str());

  return true;
}

boost::shared_ptr<Material> Model::getMaterial(const std::string& name) const
{
  boost::shared_ptr<Material> ptr;
  if (this->materials_.find(name) == this->materials_.end())
    ptr.reset();
  else
    ptr = this->materials_.find(name)->second;
  return ptr;
}

boost::shared_ptr<const Link> Model::getLink(const std::string& name) const
{
  boost::shared_ptr<const Link> ptr;
  if (this->links_.find(name) == this->links_.end())
    ptr.reset();
  else
    ptr = this->links_.find(name)->second;
  return ptr;
}

void Model::getLinks(std::vector<boost::shared_ptr<Link> >& links) const
{
  for (std::map<std::string,boost::shared_ptr<Link> >::const_iterator link = this->links_.begin();link != this->links_.end(); link++)
  {
    links.push_back(link->second);
  }
}

void Model::getLink(const std::string& name,boost::shared_ptr<Link> &link) const
{
  boost::shared_ptr<Link> ptr;
  if (this->links_.find(name) == this->links_.end())
    ptr.reset();
  else
    ptr = this->links_.find(name)->second;
  link = ptr;
}

boost::shared_ptr<const Joint> Model::getJoint(const std::string& name) const
{
  boost::shared_ptr<const Joint> ptr;
  if (this->joints_.find(name) == this->joints_.end())
    ptr.reset();
  else
    ptr = this->joints_.find(name)->second;
  return ptr;
}

}

