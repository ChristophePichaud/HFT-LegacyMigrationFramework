#include "RepositoryGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>

void RepositoryGenerator::generateRepositories(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[RepositoryGenerator] Would generate repositories into: " << outputDir << "\n";
}
