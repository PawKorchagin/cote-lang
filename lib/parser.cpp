#include "parser.h"
#include <fstream>

class AlwaysException {
    std::string what;
public:
    AlwaysException(std::string s) {
        what = s;    
    }
};

namespace {

}

std::unique_ptr<Expr> parse(std::istream &in) {
    throw new AlwaysException("sad trombone");
}
