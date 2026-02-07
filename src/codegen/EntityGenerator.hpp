#pragma once
#include <string>

class Catalog;

class EntityGenerator {
public:
    void generateEntities(const Catalog& catalog, const std::string& outputDir);
    void generateEntityTraits(const Catalog& catalog, const std::string& outputDir);
};
