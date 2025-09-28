CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address

NAME = ircserv

SRC =	main.cpp \
		Server.cpp \
		Client.cpp \
		Channel.cpp \
		server_commands.cpp
obj_dir = obj
OBJ = $(addprefix $(obj_dir)/, $(SRC:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(obj_dir)/%.o: %.cpp
	@mkdir -p $(obj_dir)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(obj_dir)

fclean: clean
	rm -f $(NAME)

re: fclean all
