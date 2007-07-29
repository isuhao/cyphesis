// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000,2001 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

// $Id: Pedestrian.cpp,v 1.80 2007-07-29 03:33:34 alriddoch Exp $

#include "Pedestrian.h"

#include "Entity.h"

#include "common/const.h"
#include "common/debug.h"
#include "common/log.h"
#include "common/compose.hpp"

#include "common/Tick.h"

#include <wfmath/atlasconv.h>

#include <Atlas/Objects/Operation.h>
#include <Atlas/Objects/Anonymous.h>

using Atlas::Message::Element;
using Atlas::Objects::Operation::Move;
using Atlas::Objects::Entity::Anonymous;

static const bool debug_flag = false;

Pedestrian::Pedestrian(Entity & body) : Movement(body)
{
}

Pedestrian::~Pedestrian()
{
}

double Pedestrian::getTickAddition(const Point3D & coordinates,
                                   const Vector3D & velocity) const
{
    // This may seem a little weird. Everything is handled in squares to
    // reduce the number of square roots that have to be calculated. In
    // this case only one is required.
    double basic_square_distance = velocity.sqrMag()
                                   * consts::square_basic_tick;
    if (m_targetPos.isValid()) {
        double square_distance = squareDistance(coordinates, m_targetPos);
        debug( std::cout << "basic_distance: " << basic_square_distance
                         << std::endl << std::flush;);
        debug( std::cout << "distance: " << square_distance << std::endl
                         << std::flush;);
        if (basic_square_distance > square_distance) {
            debug( std::cout << "\tshortened tick" << std::endl << std::flush;);
            return sqrt(square_distance / basic_square_distance)
                        * consts::basic_tick;
        }
    }
    return consts::basic_tick;
}

/// \brief Calculate the updated position of the entity since the last
/// move operation.
///
/// The does not modify the entity, but determines a new Location based
/// on the velocity of the entity, time elapsed, whether any collision
/// occurs, and whether the entity has reached its target.
/// @return 1 if no update was made, or 0 otherwise
int Pedestrian::getUpdatedLocation(Location & return_location)
{
    const double & current_time = BaseWorld::instance().getTime();
    double time_diff = current_time - m_body.m_location.timeStamp();

    if (!updateNeeded(m_body.m_location)) {
        debug( std::cout << "No update" << std::endl << std::flush;);
        return 1;
    }

    Location new_location(m_body.m_location);

    // Update location
    Point3D new_coords = m_body.m_location.pos();
    new_coords += (m_body.m_location.velocity() * time_diff);
    if (m_targetPos.isValid()) {
        Point3D new_coords2 = new_coords;
        new_coords2 += (m_body.m_location.velocity() * (consts::basic_tick / 10.0));
        // The values returned by squareDistance are squares, so
        // cannot be used except for comparison
        double dist = squareDistance(m_targetPos, new_coords);
        double dist2 = squareDistance(m_targetPos, new_coords2);
        debug( std::cout << "dist: " << dist << "," << dist2 << std::endl
                         << std::flush;);
        if (dist2 > dist) {
            // If dist2 is larger than dist, then further movement
            // is away from m_targetPos, so we know we have arrived.
            debug( std::cout << "m_targetPos achieved";);
            new_location.m_pos = m_targetPos;

            // We have arrived at our target position and must
            // stop, or be deflected
            // Arrived at intended destination
            reset();
            new_location.m_velocity = Vector3D(0,0,0);
        } else {
            new_location.m_pos = new_coords;
        }
    } else {
        new_location.m_pos = new_coords;
    }

    std::string mode("standing");

    if (m_body.hasAttr("mode")) {
        Element mode_attr;
        m_body.getAttr("mode", mode_attr);
        if (mode_attr.isString()) {
            mode = mode_attr.String();
        } else {
            log(ERROR, String::compose("MODE on entity is a \"%1\" "
                                       "in Pedestrain::getUpdatedLocation.",
                                       Element::typeName(mode_attr.getType())));
        }
    }

    float z = BaseWorld::instance().constrainHeight(new_location.m_loc, new_location.m_pos,
                                              mode);
    debug(std::cout << "Height adjustment " << z << " " << new_location.m_pos.z()
                    << std::endl << std::flush;);

    new_location.m_pos.z() = z;

    return_location = new_location;

    return 0;
}

Operation Pedestrian::generateMove(Location & new_location)
{
    // Create move operation
    Move moveOp;
    moveOp->setTo(m_body.getId());

    // Set up argument for operation
    Anonymous move_arg;
    move_arg->setId(m_body.getId());

    // Walk out what the mode of the character should be.
    // Performed in squares to save on that critical sqrt() call
    double vel_square_mag = 0;
    if (new_location.velocity().isValid()) {
        vel_square_mag = new_location.velocity().sqrMag();
    }
    double square_speed_ratio = vel_square_mag / consts::square_base_velocity;

    float height = 0;
    if (m_body.m_location.bBox().isValid()) {
        height = m_body.m_location.bBox().highCorner().z() - 
                 m_body.m_location.bBox().lowCorner().z();
    }

    if (new_location.pos().z() < (0 - height * 0.75)) {
        move_arg->setAttr("mode", "swimming");
    } else {
        if (square_speed_ratio > 0.25) { // 0.5 ^ 2
            move_arg->setAttr("mode", "running");
        } else if (square_speed_ratio > 0.0025) { // 0.05 ^ 2
            move_arg->setAttr("mode", "walking");
        } else {
            move_arg->setAttr("mode", "standing");
        }
    }
    debug(std::cout << move_arg->getAttr("mode").asString() << std::endl << std::flush;);

    new_location.addToEntity(move_arg);
    moveOp->setArgs1(move_arg);

    return moveOp;
}
