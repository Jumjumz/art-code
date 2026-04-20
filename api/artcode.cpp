#include "artcode.hpp"

// Build system
// TODO: create source struct implementation
struct Sources::Source {
    ArrayString includes;
};

Sources::Sources() : _sources(std::make_unique<Source>()) {};

Sources::~Sources() = default;

void Sources::add(const ArrayString &includes) const {
    this->_sources->includes = includes;
};

ArrayString Sources::get() const { return this->_sources->includes; };

void Sources::build() {};
