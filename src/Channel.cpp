#include "Channel.hpp"
#include "Server.hpp"
#include "Client.hpp"

Channel::Channel(void)
:_name(""),_pass(""),_limit(0),_mode(0),_server(NULL)
{}

Channel::Channel(std::string name, std::string pass, Server *server)
:_name(name),_pass(pass), _limit(0), _mode(0),_server(server)
{
	if (!_pass.empty())
		setMode(ChannelKey, true);
	if (_name[0] != '#')
		_name = "#" + _name;
}


Channel::Channel(const Channel &){}

Channel::~Channel(void) {}

Channel& Channel::operator=(const Channel &){return *this;}

const std::string& Channel::getName(void) const {
	return _name;
}

const std::string& Channel::getPass(void) const {
	return _pass;
}

void Channel::addClient(int fd) {
	if (!hasClient(fd))
		_clients.push_back(fd);
}

void Channel::removeClient(int fd) {
	std::vector<int>::iterator it = std::find(_clients.begin(), _clients.end(), fd);
	if (it != _clients.end())
		_clients.erase(it);
	removeOperator(fd);
}

bool Channel::hasClient(int fd) const {
	return std::find(_clients.begin(), _clients.end(), fd) != _clients.end();
}

void Channel::setMode(ChannelMode key, bool value) {
	if (value)
		_mode |= (1 << key);
	else
		_mode &= ~(1 << key);
}

bool Channel::getMode(ChannelMode key) const {
	return _mode & (1 << key);
}

std::string Channel::getModeString(void) const {
	std::string modes = "+n"; // default mode, prevents external messages
	if (getMode(ChanInviteOnly))
		modes += "i";
	if (getMode(ChanTopicProtected))
		modes += "t";
	if (getMode(ChannelKey))
		modes += "k";
	if (getMode(ChanLimit))
		modes += "l";
	if (getMode(ChanSecret))
		modes += "s";
	if (getMode(ChanModerated))
		modes += "m";
	return modes;
}

int Channel::getLimit(void) const {
	return _limit;
}

void Channel::setLimit(int limit) {
	_limit = limit;
}


void Channel::setTopic(std::string topic) {
	_topic = topic;
}

const std::string& Channel::getTopic(void) const {
	return _topic;
}

void Channel::addOperator(int fd) {
	if (!isOperator(fd))
		_operators.push_back(fd);
}

void Channel::removeOperator(int fd) {
	std::vector<int>::iterator it = std::find(_operators.begin(), _operators.end(), fd);
	if (it != _operators.end())
		_operators.erase(it);
}

bool Channel::isOperator(int fd) const {
	return std::find(_operators.begin(), _operators.end(), fd) != _operators.end();
}

void Channel::broadcast(std::string message) {
	for (std::vector<int>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		_server->sendMessageToClient(*it, message);
	}
}

void Channel::broadcast(std::string message, int fd) {
	for (std::vector<int>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		if (*it != fd)
			_server->sendMessageToClient(*it, message);
	}
}

int Channel::getClientCount(void) const {
	return _clients.size();
}

void Channel::setName(std::string name) {
	_name = name;
	if (_name[0] != '#')
		_name = "#" + _name;
}

void Channel::setPass(std::string pass) {
	_pass = pass;
}

const std::vector<int>& Channel::getClients(void) const {
	return _clients;
}

std::string Channel::getclientsNicknames(void) const {
	std::string nicks;
	for (std::vector<int>::const_iterator it = _clients.begin(); it != _clients.end(); it++) {
		if (isOperator(*it))
			nicks += "@"; // operator
		nicks += _server->getClient(*it).getNickname() + " ";
	}
	return nicks;
}

void Channel::addVoice(int fd) {
	if (!hasVoice(fd))
		_voiced.push_back(fd);
}

void Channel::removeVoice(int fd) {
	std::vector<int>::iterator it = std::find(_voiced.begin(), _voiced.end(), fd);
	if (it != _voiced.end())
		_voiced.erase(it);
}

bool Channel::hasVoice(int fd) const {
	return std::find(_voiced.begin(), _voiced.end(), fd) != _voiced.end();
}

void Channel::addInvite(int fd) {
	if (!hasInvite(fd))
		_invites.push_back(fd);
}

void Channel::removeInvite(int fd) {
	std::vector<int>::iterator it = std::find(_invites.begin(), _invites.end(), fd);
	if (it != _invites.end())
		_invites.erase(it);
}

bool Channel::hasInvite(int fd) const {
	return std::find(_invites.begin(), _invites.end(), fd) != _invites.end();
}

