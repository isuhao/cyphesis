// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2004 Alistair Riddoch

#include "terrain_utils.h"

#include "Database.h"
#include "debug.h"

#include <Mercator/Terrain.h>

#include <Atlas/Message/Element.h>

#include <vector>

static const bool debug_flag = false;

typedef Mercator::Terrain::Pointstore Pointstore;
typedef Mercator::Terrain::Pointcolumn Pointcolumn;

void storeTerrain(const std::string & id, const Mercator::Terrain & terrain)
{
    const Pointstore & points = terrain.getPoints();
    for (Pointstore::const_iterator I=points.begin(); I != points.end(); ++I) {
        Pointcolumn::const_iterator J = I->second.begin();
        for (; J != I->second.end(); ++J) {
            std::vector<int> coords(2);

            coords[0] = I->first;
            coords[1] = J->first;

            Atlas::Message::MapType heightPoint;
            heightPoint["height"] = J->second.height();

            Database::instance()->createArrayRow("terrain", id, coords,
                                                 heightPoint);

            debug(std::cout << I->first << ", " << J->first << " = "
                            << J->second.height() << std::endl << std::flush;);
        }
    }
}

void loadTerrain(const std::string & id, Mercator::Terrain & terrain)
{
    DatabaseResult res = Database::instance()->selectArrayRows("terrain", id);

    for (DatabaseResult::const_iterator I = res.begin(); I != res.end(); ++I) {
        int x = 4, y = 5;
        float h = 3.14159265;

        I.readColumn("i", x);
        I.readColumn("j", y);
        I.readColumn("height", h);

        debug(std::cout << "Heightpoint " << x << "," << y << " = " << h
                        << std::endl << std::flush;);

        terrain.setBasePoint(x, y, h);
    }
    res.clear();
}

void updateStoredTerrain(const std::string & id,
                         const Mercator::Terrain & terrain,
                         const PointSet & modified,
                         const PointSet & created)
{
    Mercator::BasePoint bp;
    debug(std::cout << "Update Stored Terrain" << std::endl << std::flush;);

    PointSet::const_iterator I = modified.begin();
    for (; I != modified.end(); ++I) {
        std::set<int>::const_iterator J = I->second.begin();
        for (; J != I->second.end(); ++J) {
            debug(std::cout << "Modified " << I->first << *J
                            << std::endl << std::flush;);
            std::vector<int> coords(2);

            coords[0] = I->first;
            coords[1] = *J;

            Atlas::Message::MapType heightPoint;
            assert(terrain.getBasePoint(I->first, *J, bp));
            heightPoint["height"] = bp.height();

            Database::instance()->updateArrayRow("terrain", id, coords,
                                                 heightPoint);
        }
    }

    for (I = created.begin(); I != created.end(); ++I) {
        std::set<int>::const_iterator J = I->second.begin();
        for (; J != I->second.end(); ++J) {
            std::cout << "Created " << I->first << "," << *J << std::endl << std::flush;
            std::vector<int> coords(2);

            coords[0] = I->first;
            coords[1] = *J;

            Atlas::Message::MapType heightPoint;
            assert(terrain.getBasePoint(I->first, *J, bp));
            heightPoint["height"] = bp.height();

            Database::instance()->createArrayRow("terrain", id, coords,
                                                 heightPoint);
        }
    }
}
