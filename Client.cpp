#include "Client.hpp"
#include <chrono>

Client::Client(void) {}

Client::Client(const Client&){}


Client::Client(int socket,std::string ip, std::string port, std::string hostname)
:_ip(ip),_port(port),_socket(socket),_hostname(hostname),_inboundBuffer(""),_outboundBuffer(""),
_realname(""),_authenticated(false),_isregistered(false)
{
    if (_hostname.empty())
        _hostname = ip;
}

Client& Client::operator=(const Client&){return *this;}

Client::~Client(void) {}

std::string Client::getNetworkIdentifier(void) const {
    return _nickname + "!" + _username + "@" + _hostname;
}

int Client::getSocket(void) const {
    return _socket;
}

void Client::appendToInboundBuffer(std::string data) {
    _inboundBuffer << data;
}

bool Client::inboundReady(void) const {
    return _inboundBuffer.str().find("\r\n") != std::string::npos;
}

std::vector<std::string> Client::getCompleteCommands(void) {
    std::vector<std::string> commands;
    std::string buffer = _inboundBuffer.str();
    size_t pos;
    while ((pos = buffer.find("\r\n")) != std::string::npos) {
        std::string sub = buffer.substr(0, pos);
        if (sub.size() > 0) // ignore empty lines
            commands.push_back(buffer.substr(0, pos));
        buffer.erase(0, pos + 2);
    }
    _inboundBuffer.str(buffer);
    return commands;
}

bool Client::outboundReady(void) const {
    return _outboundBuffer.str().size();
}

std::string Client::getOutboundBuffer(void) {
    return _outboundBuffer.str();
}

void Client::advanceOutboundBuffer(size_t bytes) {
    std::string buffer = _outboundBuffer.str();
    _outboundBuffer.str(buffer.substr(bytes));
}

const std::string& Client::getNickname(void) const {
    return _nickname;
}

const std::string& Client::getUsername(void) const {
    return _username;
}

const std::string& Client::getRealname(void) const {
    return _realname;
}

const std::string& Client::getHostname(void) const {
    return _hostname;
}

bool Client::isAuthenticated(void) const {
    return _authenticated;
}

bool Client::isRegistered(void) const {
    return _isregistered;
}

void Client::setNickname(std::string nickname) {
    _nickname = nickname;
}

void Client::setUsername(std::string username) {
    _username = username;
}

void Client::setRealname(std::string realname) {
    _realname = realname;
}

void Client::setAuthenticated(bool authenticated) {
    _authenticated = authenticated;
}

void Client::setRegistered(bool registered) {
    _isregistered = registered;
}

void Client::newMessage(std::string message) {
    _outboundBuffer << message << "\r\n";
}

std::string Client::prefix(void) const {
    return ":" + getNetworkIdentifier() + " ";
}


