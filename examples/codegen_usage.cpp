#include "hft/catalog/Catalog.h"
#include "hft/codegen/CodeGenerator.h"
#include "hft/reflection/EntityTraits.h"
#include <iostream>

int main() {
    std::cout << "=== HFT Legacy Migration Framework - Code Generation ===" << std::endl;
    
    using namespace hft::catalog;
    using namespace hft::codegen;
    using namespace hft::reflection;
    
    // Define table metadata
    TableInfo productTable("products");
    productTable.addColumn(ColumnInfo("id", FieldType::INT64, true, false));
    productTable.addColumn(ColumnInfo("name", FieldType::STRING, false, false));
    productTable.addColumn(ColumnInfo("description", FieldType::STRING, false, true));
    productTable.addColumn(ColumnInfo("price", FieldType::DOUBLE, false, false));
    productTable.addColumn(ColumnInfo("quantity", FieldType::INT32, false, false));
    
    // Register in catalog
    Catalog::instance().registerTable(productTable);
    
    // Generate entity class
    std::cout << "\n=== Generated Entity Class ===" << std::endl;
    std::string entityCode = CodeGenerator::generateEntity(productTable);
    std::cout << entityCode << std::endl;
    
    // Generate repository class
    std::cout << "\n=== Generated Repository Class ===" << std::endl;
    std::string repoCode = CodeGenerator::generateRepository(productTable);
    std::cout << repoCode << std::endl;
    
    // Generate CREATE TABLE SQL
    std::cout << "\n=== Generated CREATE TABLE (PostgreSQL) ===" << std::endl;
    std::string sqlPG = Catalog::instance().generateCreateTableSQL("products", "postgresql");
    std::cout << sqlPG << std::endl;
    
    std::cout << "\n=== Generated CREATE TABLE (Sybase) ===" << std::endl;
    std::string sqlSybase = Catalog::instance().generateCreateTableSQL("products", "sybase");
    std::cout << sqlSybase << std::endl;
    
    return 0;
}
