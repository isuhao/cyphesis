// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "ClientConnection.h"

#include "common/debug.h"
#include "common/globals.h"

#include <Atlas/Codecs/XML.h>
#include <Atlas/Net/Stream.h>
#include <Atlas/Objects/Encoder.h>

#include <sys/socket.h>
#include <sys/un.h>

static bool debug_flag = false;

using Atlas::Message::Element;

ClientConnection::ClientConnection() :
    client_fd(-1), encoder(NULL), serialNo(512)
{
}

ClientConnection::~ClientConnection()
{
    if (encoder != NULL) {
        delete encoder;
    }
}

void ClientConnection::operation(const RootOperation & op)
{
#if 0
    const std::string & from = op.getFrom();
    if (from.empty()) {
        std::cerr << "ERROR: Operation with no destination" << std::endl << std::flush;
        return;
    }
    dict_t::const_iterator I = objects.find(from);
    if (I == objects.end()) {
        std::cerr << "ERROR: Operation with invalid destination" << std::endl << std::flush;
        return;
    }
    OpVector res = I->second->message(op);
    OpVector::const_iterator J = res.begin();
    for(J = res.begin(); J != res.end(); ++J) {
        (*J)->setFrom(I->first);
        send(*(*J));
    }
#endif
}

void ClientConnection::objectArrived(const Error&op)
{
    debug(std::cout << "ERROR" << std::endl << std::flush;);
    push(op);
    error_flag = true;
}

void ClientConnection::objectArrived(const Info & op)
{
    debug(std::cout << "INFO" << std::endl << std::flush;);
    const std::string & from = op.getFrom();
    if (from.empty()) {
        reply_flag = true;
        error_flag = false;
        try {
            Element ac = op.getArgs().front();
            reply = ac.asMap();
            // const std::string & acid = reply["id"].asString();
            // objects[acid] = new ClientAccount(acid, *this);
        }
        catch (...) {
            std::cerr << "WARNING: Malformed account from server" << std::endl << std::flush;
        }
    } else {
        operation(op);
    }
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Action& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Appearance& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Combine& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Communicate& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Create& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Delete& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Disappearance& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Divide& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Feel& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Get& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Imaginary& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Listen& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Login& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Logout& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Look& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Move& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Perceive& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Perception& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::RootOperation& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Set& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Sight& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Smell& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Sniff& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Sound& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Talk& op)
{
    push(op);
}

void ClientConnection::objectArrived(const Atlas::Objects::Operation::Touch& op)
{
    push(op);
}

int ClientConnection::read() {
    if (ios.is_open()) {
        codec->poll();
        return 0;
    } else {
        return -1;
    }
}

#define UNIX_PATH_MAX 108

bool ClientConnection::connectLocal(const std::string & sockname)
{
    debug(std::cout << "Attempting local connect." << std::endl << std::flush;);
    std::string socket;
    if (sockname == "") {
        socket = var_directory + "/cyphesis.sock";
    } else if (sockname[0] != '/') {
        socket = var_directory + "/" + sockname;
    } else {
        socket = sockname;
    }

    struct sockaddr_un sun;
    sun.sun_family = AF_UNIX;
    strncpy(sun.sun_path, socket.c_str(), UNIX_PATH_MAX);

    int fd = ::socket(PF_UNIX, SOCK_STREAM, 0);

    if (0 != ::connect(fd, (struct sockaddr *)&sun, sizeof(sun))) {
        debug(std::cout << "Local connect refused" << std::endl << std::flush;);
        return false;
    }

    ios.setSocket(fd);
    if (!ios.is_open()) {
        std::cerr << "ERROR: For some reason " << sockname << " not open."
                  << std::endl << std::flush;
        return false;
    }

    client_fd = ios.getSocket();

    bool ret = negotiate();

    if (ret == false) {
        ios.close();
        // ::close(fd);
    }
    return ret;
}

bool ClientConnection::connect(const std::string & server)
{
    debug(std::cout << "Connecting to " << server << std::endl << std::flush;);

    ios.open(server, port_num);
    if (!ios.is_open()) {
        std::cerr << "ERROR: Could not connect to " << server << "."
                  << std::endl << std::flush;
        return false;
    }

    client_fd = ios.getSocket();

    return negotiate();
}

bool ClientConnection::negotiate()
{
    Atlas::Net::StreamConnect conn("cyphesis_aiclient", ios, this);

    debug(std::cout << "Negotiating... " << std::flush;);
    while (conn.getState() == Atlas::Net::StreamConnect::IN_PROGRESS) {
      conn.poll();
    }
    debug(std::cout << "done" << std::endl;);
  
    if (conn.getState() == Atlas::Net::StreamConnect::FAILED) {
        std::cerr << "Failed to negotiate" << std::endl;
        return false;
    }

    codec = conn.getCodec();

    encoder = new Atlas::Objects::Encoder(codec);

    codec->streamBegin();

    return true;
}

bool ClientConnection::login(const std::string & account,
                             const std::string & password)
{
    Atlas::Objects::Operation::Login l = Atlas::Objects::Operation::Login::Instantiate();
    Element::MapType acmap;
    acmap["username"] = account;
    acmap["password"] = password;

    l.setArgs(Element::ListType(1,Element(acmap)));

    reply_flag = false;
    error_flag = false;
    send(l);
    return true;
}

bool ClientConnection::create(const std::string & account,
                              const std::string & password)
{
    Atlas::Objects::Operation::Create c = Atlas::Objects::Operation::Create::Instantiate();
    Element::MapType acmap;
    acmap["id"] = account;
    acmap["password"] = password;

    c.setArgs(Element::ListType(1,Element(acmap)));

    reply_flag = false;
    error_flag = false;
    send(c);
    return true;
}

bool ClientConnection::wait()
// Waits for response from server. Used when we are expecting a login response
// Return whether or not an error occured
{
   error_flag = false;
   reply_flag = false;
   while (!reply_flag) {
      poll(1);
   }
   return error_flag;
}

void ClientConnection::send(Atlas::Objects::Operation::RootOperation & op)
{
    /* debug(Atlas::Codecs::XML c((std::iostream&)std::cout, (Atlas::Bridge*)this);
          Atlas::Objects::Encoder enc(&c);
          enc.streamMessage(&op);
          std::cout << std::endl << std::flush;); */

    op.setSerialno(++serialNo);
    encoder->streamMessage(&op);
    ios << std::flush;
}

void ClientConnection::poll(int timeOut)
{
    fd_set infds;
    struct timeval tv;

    FD_ZERO(&infds);

    FD_SET(client_fd, &infds);

    tv.tv_sec = timeOut;
    tv.tv_usec = 0;

    int retval = select(client_fd+1, &infds, NULL, NULL, &tv);

    if (retval && (FD_ISSET(client_fd, &infds))) {
        if (ios.peek() == -1) {
            std::cerr << "Server disconnected" << std::endl << std::flush;
            return;
        }
        codec->poll();
        return;
    }
    return;

}

RootOperation * ClientConnection::pop()
{
    poll();
    if (operationQueue.empty()) {
        return NULL;
    }
    RootOperation * op = operationQueue.front();
    operationQueue.pop_front();
    return op;
}

bool ClientConnection::pending()
{
    return !operationQueue.empty();
}

template<class O>
void ClientConnection::push(const O & op)
{
    reply_flag = true;
    RootOperation * new_op = new O(op); 
    operationQueue.push_back(new_op);
}
