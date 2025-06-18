#include <cassert>

int main() {
    ConfigParser parser;
    assert(parser.load("test.conf") == true);
    assert(parser.getValue("server", "listen") == "8080");
    return 0;
}

