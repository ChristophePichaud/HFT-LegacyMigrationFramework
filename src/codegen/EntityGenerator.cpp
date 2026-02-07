#include "EntityGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>

void EntityGenerator::generateEntities(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[EntityGenerator] Would generate entities into: " << outputDir << "\n";
}

void EntityGenerator::generateEntityTraits(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[EntityGenerator] Would generate EntityTraits into: " << outputDir << "\n";
}
