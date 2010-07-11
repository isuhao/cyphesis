// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2001 Alistair Riddoch
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

// $Id$

/// \page cypasswd_index
///
/// \section Introduction
///
/// cypasswd is an interactive commandline tool to manage account data in
/// the database. For information on the usage, please see the unix
/// manual page. The manual page is generated from docbook sources, so can
/// also be converted into other formats.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "common/AccountBase.h"
#include "common/globals.h"
#include "common/log.h"
#include "common/system.h"

#include <varconf/config.h>

#include <string>
#include <iostream>

#include <cstring>

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#include <unistd.h>
#endif

// This is the currently very basic password management tool, which can
// be used to control the passwords of accounts in the main servers account
// database. This is the only way to create accounts on a server in
// restricted mode.

// TODO: Make sure the rest of the Object is preserved, rather than just
//       blatting it with a new Object.

using Atlas::Message::MapType;

#define ADD 0
#define SET 1
#define DEL 2

void usage(std::ostream & stream, char * n, bool verbose = false)
{
    stream << "usage: " << n << std::endl;
    stream << "       " << n << " [ -a | -d ] [ -s ] account" << std::endl;
    stream << "       " << n << " -h" << std::endl;
    if (!verbose) {
        stream << std::flush;
        return;
    }
    stream << std::endl;
    stream << "Help options" << std::endl;
    stream << "  -h                          Display this help" << std::endl;
    stream << std::endl;
    stream << "Managing accounts" << std::endl;
    stream << "  -a                          Add a new account" << std::endl;
    stream << "  -d                          Delete an account" << std::endl;
    stream << "  -s                          Make server account" << std::endl;
}

int main(int argc, char ** argv)
{
    varconf::Config * conf = varconf::Config::inst();

    conf->setParameterLookup('a', "add");
    conf->setParameterLookup('d', "del");
    conf->setParameterLookup('s', "serv");

    int config_status = loadConfig(argc, argv, USAGE_DBASE);
    if (config_status < 0) {
        if (config_status == CONFIG_VERSION) {
            reportVersion(argv[0]);
            return 0;
        } else if (config_status == CONFIG_HELP) {
            usage(std::cout, argv[0]);
            return 0;
        } else if (config_status != CONFIG_ERROR) {
            std::cerr << "Unknown error reading configuration." << std::endl;
        }
        // Fatal error loading config file
        return 1;
    }

    std::cout << config_status << "," << argc << std::endl << std::flush;

    int extra_arg_count = argc - config_status;

    std::string acname;
    int action = SET;

    if (global_conf->findItem("", "add")) {
        action = ADD;
    }

    if (global_conf->findItem("", "del")) {
        if (action != SET) {
            usage(std::cerr, argv[0]);
            return 1;
        }
        action = DEL;
    }

    if (extra_arg_count == 0) {
        if (action != SET) {
            usage(std::cerr, argv[0]);
            return 1;
        }
        acname = "admin";
    } else if (extra_arg_count == 1) {
        acname = argv[config_status];
    } else {
        usage(std::cerr, argv[0]);
        return 1;
    }

    if (security_init() != 0) {
        log(CRITICAL, "Security initialisation Error. Exiting.");
        return EXIT_SECURITY_ERROR;
    }

    AccountBase db;

    if (db.init() != 0) {
        std::cerr << "Unable to connect to database" << std::endl << std::flush;
        return 1;
    }

    // MapType data;

    // bool res = db->getAccount("admin", data);

    // if (!res) {
        // std::cout << "Admin account does not yet exist" << std::endl << std::flush;
        // acname = "admin";
        // action = ADD;
    // }
    if (action != ADD) {
        MapType o;
        bool res = db.getAccount(acname, o);
        if (!res) {
            std::cout<<"Account "<<acname<<" does not yet exist"<<std::endl<<std::flush;
            return 0;
        }
    }
    if (action == DEL) {
        bool res = db.delAccount(acname);
        if (res) {
            std::cout << "Account " << acname << " removed." << std::endl << std::flush;
        }
        return 0;
    }

    // TODO Catch signals, and restore terminal
#ifdef HAVE_TERMIOS_H
    termios termios_old, termios_new;
    
    tcgetattr( STDIN_FILENO, &termios_old );
    termios_new = termios_old;
    termios_new.c_lflag &= ~(ICANON|ECHO);
    tcsetattr( STDIN_FILENO, TCSADRAIN, &termios_new );
#endif
    
    std::string password, password2;
    std::cout << "New " << acname << " password: " << std::flush;
    std::cin >> password;
    std::cout << std::endl << "Retype " << acname << " password: " << std::flush;
    std::cin >> password2;
    std::cout << std::endl << std::flush;
    
#ifdef HAVE_TERMIOS_H
    tcsetattr( STDIN_FILENO, TCSADRAIN, &termios_old );
#endif
    
    if (password == password2) {
        MapType amap;
        amap["password"] = password;

        bool res;
        if (action == ADD) {
            amap["username"] = acname;
            if(server) {
                amap["type"] = "server";
            }
            res = db.putAccount(amap);
        } else {
            res = db.modAccount(amap, acname);
        }
        if (res) {
            std::cout << "Password changed." << std::endl << std::flush;
        }
    } else {
        std::cout << "Passwords did not match. Account database unchanged."
                  << std::endl << std::flush;
    }
}
