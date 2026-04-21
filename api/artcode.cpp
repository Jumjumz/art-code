#include "artcode.hpp"
#include "json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

// namespace
namespace fs = std::filesystem;
// Build system
struct Sources::Source {
    ArrayString includes;
};

Sources::Sources() : _sources(std::make_unique<Source>()) {};

Sources::~Sources() = default;

void Sources::add(const ArrayString &includes) const {
    this->_sources->includes = includes;
};

void Sources::build() const {
    if (!this->_sources->includes.empty()) {
        fs::path solution_file;
        { // get executable dir
            const auto exe_dir = fs::canonical("/proc/self/exe").parent_path();

            for (const auto &file :
                 fs::directory_iterator(exe_dir.parent_path())) {
                if (file.path().extension() == this->EXTENSION) {
                    solution_file = file;
                }
            }
        }

        nlohmann::json js;
        {
            std::ifstream read(solution_file);
            js = nlohmann::json::parse(read);
            read.close();
        }

        nlohmann::json sources = js["sources"];
        if (std::find(sources.begin(), sources.end(), solution_file.string()) ==
            sources.end()) {
            for (const auto &source : this->_sources->includes) {
                js["sources"].push_back(source);

                std::ofstream write(solution_file);
                write << js.dump(4);
            }
        }
    }
};
