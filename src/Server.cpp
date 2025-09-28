#include "Server.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include "Client.hpp"
#include <algorithm>

Server::Server(void) {}

Server::Server(const Server &) {}

Server &Server::operator=(const Server &) { return *this; }

Server::~Server(void)
{
	close(_socket);
}

void Server::_initCommandHandlers(void)
{
	_commandHandlers["PASS"] = &Server::PASS;
	_commandHandlers["NICK"] = &Server::NICK;
	_commandHandlers["USER"] = &Server::USER;
	_commandHandlers["PING"] = &Server::PING;
	_commandHandlers["PONG"] = &Server::PING;
	
	_commandHandlers["LIST"] = &Server::LIST;
	_commandHandlers["JOIN"] = &Server::JOIN;
	
	_commandHandlers["PRIVMSG"] = &Server::PRIVMSG;
	
	_commandHandlers["WHO"] = &Server::WHO;

	_commandHandlers["WHOIS"] = &Server::WHOIS;
	_commandHandlers["BOT"] = &Server::BOT;
	
	_commandHandlers["PART"] = &Server::PART;
	
	_commandHandlers["QUIT"] = &Server::QUIT;
	_commandHandlers["KICK"] = &Server::KICK;

	_commandHandlers["TOPIC"] = &Server::TOPIC;
	_commandHandlers["INVITE"] = &Server::INVITE;
	_commandHandlers["NOTICE"] = &Server::PRIVMSG;
	_commandHandlers["ISON"] = &Server::ISON;
	_commandHandlers["MODE"] = &Server::MODE;
}

Server::Server(std::string port, std::string password)
	: _port(port), _password(password)
{
	if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
	int enable = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
		throw std::runtime_error("Failed to set socket options to reuse address: " + std::string(strerror(errno)));
	if (fcntl(_socket, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("Failed to set socket to non-blocking: " + std::string(strerror(errno)));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(std::stoi(port));
	addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	if (bind(_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
	if (listen(_socket, SOMAXCONN) == -1)
		throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
	getPollfd(_socket); // create pollfd if not exists
	_initCommandHandlers();
	std::cout << "Server started on " << "0.0.0.0" << ":" << port << std::endl;
	// create default channel
}

void Server::acceptNewClient(void)
{
	int client_socket;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	if ((client_socket = accept(_socket, (struct sockaddr *)&addr, &addr_len)) == -1)
	{
		std::cout << "Failed to accept new client: " + std::string(strerror(errno)) << std::endl;
		return;
	}
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1)
	{
		close(client_socket);
		std::cerr << "Failed to set client socket to non-blocking: " + std::string(strerror(errno)) << std::endl;
		return;
	}
	std::string client_ip = inet_ntoa(addr.sin_addr);
	std::string client_port = std::to_string(ntohs(addr.sin_port));
	std::string client_hostname = gethostbyaddr(&addr.sin_addr, sizeof(addr.sin_addr), AF_INET)->h_name;
	_clients.insert(std::make_pair(client_socket, new Client(client_socket, client_ip, client_port, client_hostname)));
	getPollfd(client_socket); // create pollfd if not exists
}

void Server::removeClient(int socket)
{
	std::map<int, Client *>::iterator it = _clients.find(socket);
	if (it != _clients.end())
	{
		std::vector<Channel *> channels = getClientChannels(socket);
		for (size_t i = 0; i < channels.size(); i++)
			channels[i]->removeClient(socket);
		delete it->second;
		_clients.erase(it);
	}

	std::vector<pollfd>::iterator it2 = _pollfds.begin();
	for (; it2 != _pollfds.end() && it2->fd != socket; it2++)
		;
	if (it2 != _pollfds.end())
		_pollfds.erase(it2);
	close(socket);
}

void Server::readFromClient(int socket)
{
	char buffer[1024];
	int read_bytes;
	Client &client = getClient(socket);
	if ((read_bytes = recv(socket, buffer, 1024, 0)) <= 0)
	{
		if (errno != EWOULDBLOCK)
			QUIT(socket, "Client disconnected");
		return;
	}
	buffer[read_bytes] = 0;
	client.appendToInboundBuffer(buffer);
	if (client.inboundReady())
	{
		std::vector<std::string> commands = client.getCompleteCommands();
		for (size_t i = 0; i < commands.size(); i++)
			_commandsqueue.push(std::make_pair(socket, commands[i]));
	}
}

// void Server::readFromClient(int socket)
// {
//     char buffer[1024];
//     int read_bytes;
//     Client &client = getClient(socket);
//     if ((read_bytes = recv(socket, buffer, 1024, 0)) <= 0)
//     {
//         if (errno != EWOULDBLOCK)
//             QUIT(socket, "Client disconnected");
//         return;
//     }
//     buffer[read_bytes] = 0;
//     std::cerr << "Raw reading from client (" << socket << "):\n"
//     << buffer
//     << "----------------------------------------\n";
//     client.appendToInboundBuffer(buffer);
//     if (client.inboundReady())
//     {
//         std::vector<std::string> commands = client.getCompleteCommands();
//         for (size_t i = 0; i < commands.size(); i++)
//             _commandsqueue.push(std::make_pair(socket, commands[i]));
//     }
// }

// void Server::writeToClient(int socket)
// {
//     Client &client = getClient(socket);
//     if (!client.outboundReady())
//         return;
//     std::string data = client.getOutboundBuffer();
//     ssize_t bytes_sent;
//     std::cerr << "Writing to client (" << socket << "):\n"
//     << data
//     << "----------------------------------------\n";
//     if ((bytes_sent = send(socket, data.c_str(), data.size(), 0)) == -1)
//     {
//         if (errno != EWOULDBLOCK)
//             QUIT(socket, "Client disconnected");
//         return;
//     }
//     client.advanceOutboundBuffer(bytes_sent);
//     if (!client.outboundReady()) // no more data to send
//         getPollfd(socket).events &= ~POLLOUT; // disable POLLOUT -> POLLIN | POLLERR | POLLHUP
// }

void Server::writeToClient(int socket)
{
	Client &client = getClient(socket);
	if (!client.outboundReady())
		return;
	std::string data = client.getOutboundBuffer();
	ssize_t bytes_sent;
	if ((bytes_sent = send(socket, data.c_str(), data.size(), 0)) == -1)
	{
		if (errno != EWOULDBLOCK)
			QUIT(socket, "Client disconnected");
		return;
	}
	client.advanceOutboundBuffer(bytes_sent);
	if (!client.outboundReady()) // no more data to send
		getPollfd(socket).events &= ~POLLOUT; // disable POLLOUT -> POLLIN | POLLERR | POLLHUP
}

void Server::sendMessageToClient(int socket, std::string message)
{
	Client &client = getClient(socket);
	client.newMessage(message);
	if (client.outboundReady())
		getPollfd(socket).events |= POLLOUT; // enable POLLOUT -> POLLIN | POLLOUT | POLLERR | POLLHUP
}

void Server::mainLoop(void)
{
	while (_pollfds.size())
	{
		if (poll(_pollfds.data(), _pollfds.size(), -1) == -1)
			throw std::runtime_error("Failed to poll sockets: " + std::string(strerror(errno)));
		if (_pollfds[0].revents & POLLIN)
			acceptNewClient();
		for (size_t i = 1; i < _pollfds.size(); i++)
		{
			if (_pollfds[i].revents & (POLLERR | POLLHUP))
				QUIT(_pollfds[i].fd, "Client disconnected");
			else if (_pollfds[i].revents & POLLOUT)
				writeToClient(_pollfds[i].fd);
			else if (_pollfds[i].revents & POLLIN)
				readFromClient(_pollfds[i].fd);
		}
		if (_commandsqueue.size())
			processCommands();
	}
}

void Server::createChannel(std::string name,std::string pass, std::string topic)
{
	std::string key = name;
	if (key[0] == '#')
		key = key.substr(1);
	for (size_t i = 0; i < key.size(); i++)
		key[i] = std::tolower(key[i]); // make channel name case-insensitive
	if (_channels.find(key) != _channels.end())
		throw std::runtime_error("Channel already exists");
	Channel *channel = new Channel(name, pass, this);
	channel->setTopic(topic);
	_channels.insert(std::make_pair(key, channel));
}

Client &Server::getClient(int socket)
{
	std::map<int, Client *>::iterator it = _clients.find(socket);
	if (it == _clients.end())
		throw std::runtime_error("Client not found in getClient");
	return *it->second;
}

std::string Server::prefix(void)
{
	return ":ircserv ";
}

Client &Server::getClient(std::string nickname)
{
	std::map<int, Client *>::iterator it = _clients.begin();
	for (; it != _clients.end() && it->second->getNickname() != nickname; it++)
		;
	if (it == _clients.end())
		throw std::runtime_error("Client not found in getClient");
	return *it->second;
}

void Server::processCommands(void)
{
	while (_commandsqueue.size())
	{
		std::pair<int, std::string> command = _commandsqueue.front();
		_commandsqueue.pop();
		Client &client = getClient(command.first);
		std::string command_name;
		std::string command_args;
		std::stringstream ss(command.second);
		ss >> command_name >> std::ws;
		std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::toupper);
		std::getline(ss, command_args, '\0');
		if (command_name != "PASS" && !client.isAuthenticated())
			sendMessageToClient(command.first, prefix() + "451 : You have not registered");
		else if (command_name != "PASS" && command_name != "NICK" && command_name != "USER" && !client.isRegistered())
			sendMessageToClient(command.first, prefix() + "451 : You have not registered");
		else if (_commandHandlers.find(command_name) == _commandHandlers.end())
			sendMessageToClient(command.first, prefix() + "421 " + command_name + " : Unknown command");
		else
			(this->*_commandHandlers[command_name])(command.first, command_args);
	}
}

pollfd &Server::getPollfd(int socket)
{
	std::vector<pollfd>::iterator it = _pollfds.begin();
	for (; it != _pollfds.end() && it->fd != socket; it++)
		;
	if (it != _pollfds.end())
		return *it;
	pollfd new_pollfd = {.fd = socket, .events = POLLIN | POLLERR | POLLHUP};
	_pollfds.push_back(new_pollfd);
	return _pollfds.back();
}

Channel &Server::getChannel(std::string name)
{
	if (name[0] == '#')
		name = name.substr(1);
	for (size_t i = 0; i < name.size(); i++)
		name[i] = std::tolower(name[i]); // make channel name case-insensitive
	std::map<std::string, Channel *>::iterator it = _channels.find(name);
	if (it == _channels.end())
		throw std::runtime_error("Channel not found in getChannel");
	return *it->second;
}

std::vector<Channel *> Server::getClientChannels(int socket)
{
	std::vector<Channel *> channels;
	std::map<std::string, Channel *>::iterator it = _channels.begin();
	for (; it != _channels.end(); it++)
		if (it->second->hasClient(socket))
			channels.push_back(it->second);
	return channels;
}

void Server::sendMessageToClientChannels(int socket, std::string message)
{
	std::vector<Channel *> channels = getClientChannels(socket);
	for (size_t i = 0; i < channels.size(); i++)
		channels[i]->broadcast(message, socket);
}

