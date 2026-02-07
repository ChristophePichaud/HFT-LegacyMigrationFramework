#include "UnitTestGenerator.hpp"
#include "catalog/Catalog.hpp"
#include <iostream>

void UnitTestGenerator::generateTests(const Catalog& catalog, const std::string& outputDir) {
    (void)catalog;
    std::cout << "[UnitTestGenerator] Would generate tests into: " << outputDir << "\n";
}
