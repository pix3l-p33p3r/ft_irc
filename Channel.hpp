#pragma once
#include <string>
#include <vector>


/*
CHANNEL MODES:
	- i: invite only
	- t: topic settable by channel operator only
	- k: channel key
	- l: limit on number of users that may join the channel
	- o: operator
*/

enum ChannelMode {
	ChanInviteOnly = 1,
	ChanTopicProtected = 2,
	ChannelKey = 3,
	ChanLimit = 4,
	ChanSecret = 5, // means the channel won't be listed in /list
	ChanModerated = 6 // only operators can send messages
};


class Server;

class Channel {
	private:
		std::string			_name;
		std::string			_pass;
		std::string			_topic;
		std::vector<int>	_clients;
		std::vector<int>	_operators;
		std::vector<int>	_voiced;
		std::vector<int>_invites;
		int					_limit;
		int					_mode;
		Server				*_server;
		Channel&			operator=(const Channel &);
	public:
							Channel(void);
							Channel(std::string name, std::string pass, Server *server);

							Channel(const Channel &);
							~Channel(void);

		const std::string&	getName(void) const;
		void 				setName(std::string);

		const std::string&	getPass(void) const;
		void 				setPass(std::string);

		void				setMode(ChannelMode, bool);
		bool				getMode(ChannelMode) const;
		std::string			getModeString(void) const;


		int					getLimit(void) const;
		void				setLimit(int);

		void				setTopic(std::string);
		const std::string&	getTopic(void) const;

		void				addClient(int);
		const std::vector<int>&getClients(void) const;
		std::string 		getclientsNicknames(void) const;
		int					getClientCount(void) const;
		void				removeClient(int);
		bool				hasClient(int) const;

		void				addVoice(int);
		void				removeVoice(int);
		bool				hasVoice(int) const;
		
		void				addInvite(int);
		void				removeInvite(int);
		bool				hasInvite(int) const;

		void				addOperator(int);
		void				removeOperator(int);
		bool				isOperator(int) const;

		void				broadcast(std::string);
		void				broadcast(std::string, int); // broadcast to all except one: invoker
};