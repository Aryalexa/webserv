CXX       = g++
CXXFLAGS  = -Wall -Wextra -std=c++98 -Iinclude

NAME      = webserver

# Colors
GREEN   = \033[0;32m
RED     = \033[0;31m
NC      = \033[0m

# === src files =======================================
SRC       = src/main.cpp \
            src/Server.cpp \
            src/ConfigParser.cpp \
            src/HttpRequest.cpp \
            src/HttpResponse.cpp

OBJ       = $(SRC:.cpp=.o)

# === test files  =======================================
TEST_SRC  = tests/test_configparser.cpp \
            tests/test_httpparser.cpp

TEST_BIN  = tests/test_configparser \
            tests/test_httpparser

# === Rules =======================================

# Default target
all: $(NAME)

# Link server binary
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Compile object files
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test binaries
test: $(TEST_BIN)
	@for bin in $(TEST_BIN); do echo "Running $$bin"; ./$$bin; done

tests/test_configparser: tests/test_configparser.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $<

tests/test_httpparser: tests/test_httpparser.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Clean object files and test binaries
clean:
	rm -f $(OBJ) $(TEST_BIN)

# Clean everything including main binary
fclean: clean
	rm -f $(NAME)

# Rebuild everything
re: fclean all

.PHONY: all clean fclean re test
