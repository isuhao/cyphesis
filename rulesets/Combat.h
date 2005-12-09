// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2005 Alistair Riddoch

#ifndef RULESETS_COMBAT_H
#define RULESETS_COMBAT_H

#include "rulesets/Task.h"

/// \brief Task class for fighting
class Combat : public Task {
  protected:
    Character & m_target;
    bool m_attack;

    static const double minStamina;
  public:
    explicit Combat(Character & chr, Character & target);
    virtual ~Combat();

    virtual void irrelevant();

    virtual void initTask(const Operation & op, OpVector & res);

    virtual void TickOperation(const Operation & op, OpVector & res);
};

#endif // RULESETS_COMBAT_H
