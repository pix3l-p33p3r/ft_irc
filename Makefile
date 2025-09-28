CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Ofast

CXXFLAGS += -g -fsanitize=address -fsanitize=leak -fsanitize=undefined

NAME = ircserv

SRC =	src/main.cpp \
		src/Server.cpp \
		src/Client.cpp \
		src/Channel.cpp \
		src/server_commands.cpp
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
