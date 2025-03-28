#include "parser.h"

class AlwaysException {
    std::string what;
public:
    AlwaysException(std::string s) {
        what = s;    
    }
};

void parse(const std::string& s) {
    throw new AlwaysException("sad thrombone");
}
