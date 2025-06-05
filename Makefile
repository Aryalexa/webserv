$(info ***  CXX = $(CXX)  ***)

NAME := webserv

SRCS := $(wildcard sources/*.cpp)

OBJS := $(SRCS:.cpp=.o)

CXX      := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98 -Iinc

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
