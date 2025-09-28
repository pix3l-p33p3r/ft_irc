#pragma once

#include <iostream>
#include <sstream>
#include <vector>

class Client {
	private:
		std::string				_ip;
		std::string				_port;
		int						_socket;
		std::string				_hostname;
		std::stringstream		_inboundBuffer;
		std::stringstream		_outboundBuffer;

		std::string				_nickname;
		std::string 			_username;
		std::string 			_realname;
		bool					_authenticated;
		bool					_isregistered;
		
								Client(void); // can't be empty constructed or copied
		Client&					operator=(const Client&);
								Client(const Client&);
	public:
								Client(int, std::string, std::string, std::string);
								~Client(void);

		int						getSocket(void) const;
		std::string				getNetworkIdentifier(void) const;

		void					appendToInboundBuffer(std::string);
		bool					inboundReady(void) const;
		std::vector<std::string>getCompleteCommands(void); // splits inbound on "\r\n"s

		void					newMessage(std::string);
		bool					outboundReady(void) const;
		std::string				getOutboundBuffer(void);
		void					advanceOutboundBuffer(size_t);

		const std::string&		getNickname(void) const;
		const std::string&		getUsername(void) const;
		const std::string&		getRealname(void) const;
		const std::string&		getHostname(void) const;

		bool					isAuthenticated(void) const;
		bool					isRegistered(void) const;

		void					setNickname(std::string);
		void					setUsername(std::string);
		void					setRealname(std::string);
		void					setAuthenticated(bool authenticated = true);
		void					setRegistered(bool isregistered = true);
		std::string				prefix(void) const;
};