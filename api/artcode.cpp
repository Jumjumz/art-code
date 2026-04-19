#include "artcode.hpp"

#include <memory>
#include <string>
#include <vector>

// Build system
// TODO: create source struct implementation
struct Sources::Source {
    std::vector<std::string> includes;
};

Sources::Sources() : _sources(std::make_unique<Source>()) {};

Sources::~Sources() = default;

void Sources::add() {

};

void Sources::build() {

};
