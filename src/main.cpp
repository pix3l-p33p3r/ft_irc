#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <csignal>

int main(int ac, char **av)
{
	if (ac != 3 || std::string(av[1]).find_first_not_of("0123456789") != std::string::npos)
	{
		std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl;
		return 1;
	}
	try
	{
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Error: signal");
		Server server(av[1], av[2]);
		server.createChannel("random", "1234");
		server.createChannel("announcements", "");
		server.getChannel("announcements").setMode(ChanTopicProtected, true);
		server.getChannel("announcements").setMode(ChanModerated, true);
		server.getChannel("announcements").setTopic("official Announcements channel");
		
		server.mainLoop();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}