#pragma once
#include <string>

class Catalog;

class RepositoryGenerator {
public:
    void generateRepositories(const Catalog& catalog, const std::string& outputDir);
};
