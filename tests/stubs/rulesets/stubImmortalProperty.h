/*
 Copyright (C) 2014 Erik Ogenvik

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef STUBIMMORTALPROPERTY_H_
#define STUBIMMORTALPROPERTY_H_

ImmortalProperty::ImmortalProperty()
{
}

ImmortalProperty::~ImmortalProperty()
{
}

ImmortalProperty * ImmortalProperty::copy() const
{
    return 0;
}

void ImmortalProperty::apply(LocatedEntity * owner)
{
}

void ImmortalProperty::install(LocatedEntity * ent, const std::string & name)
{
}

void ImmortalProperty::remove(LocatedEntity * ent, const std::string & name)
{
}

HandlerResult ImmortalProperty::operation(LocatedEntity *, const Operation &, OpVector &)
{
    return OPERATION_IGNORED;
}


#endif /* STUBIMMORTALPROPERTY_H_ */
