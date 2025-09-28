#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <poll.h>
#include "Channel.hpp"

class Client;

class Server {
	private:
		typedef void (Server::*commandHandler)(int, std::string);
		std::string								_port;
		int										_socket;
		std::map<int, Client*>					_clients;
		std::map<std::string, Channel *>		_channels;

		std::vector<pollfd>						_pollfds;
		std::queue<std::pair<int, std::string> >_commandsqueue;
		std::string								_password;
		std::map<std::string, commandHandler>	_commandHandlers;

												Server(void);
												Server(const Server&);
		Server&									operator=(const Server&);

		std::string								prefix(void);
		void 									_initCommandHandlers(void);

	public:
												Server(std::string, std::string);
												~Server(void);

		void									mainLoop(void);
		void									processCommands(void);
												
		pollfd&									getPollfd(int);
		void									removePollfd(int);

		void									acceptNewClient(void);
		Client&									getClient(int); // by fd
		Client&									getClient(std::string); // by nickname
		void									removeClient(int);

		void									readFromClient(int);
		void									writeToClient(int);
		void									sendMessageToClient(int, std::string);

		void									createChannel(std::string, std::string, std::string = "No topic"); // "No topic
		Channel&								getChannel(std::string);
		std::vector<Channel *>					getClientChannels(int);
		void									sendMessageToClientChannels(int, std::string);
		void									removeClientFromChannels(int);

		void									registerNewClient(int);
		void									PASS(int, std::string);
		void									BOT(int, std::string);
		void									NICK(int, std::string);
		void									USER(int, std::string);
		void									PING(int, std::string);
		void									LIST(int, std::string);
		void									JOIN(int, std::string);
		void									PART(int, std::string);
		void									WHO(int, std::string);
		void									WHOIS(int, std::string);
		void									PRIVMSG(int, std::string);
		void									QUIT(int, std::string);
		void									KICK(int, std::string);
		void									TOPIC(int, std::string);
		void									INVITE(int, std::string);
		void									NOTICE(int, std::string);
		void									ISON(int, std::string);
		void									MODE(int, std::string);
};
