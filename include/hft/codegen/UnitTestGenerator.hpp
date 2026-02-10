#pragma once
#include <string>

class Catalog;

class UnitTestGenerator {
public:
    void generateTests(const Catalog& catalog, const std::string& outputDir);
};
