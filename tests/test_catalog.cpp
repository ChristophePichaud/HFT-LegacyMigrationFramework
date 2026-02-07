#include <gtest/gtest.h>
#include "hft/catalog/Catalog.h"
#include "hft/reflection/EntityTraits.h"

using namespace hft::catalog;
using namespace hft::reflection;

// Test entity
struct TestUser {
    int64_t id = 0;
    std::string name = "";
    std::string email = "";
    int32_t age = 0;
};

// Define traits for TestUser
namespace hft { namespace reflection {

template<>
struct EntityTraits<TestUser> {
    static constexpr const char* tableName() { return "test_users"; }
    static constexpr size_t fieldCount() { return 4; }
    
    template<typename Func>
    static void forEachField(TestUser& entity, Func&& func) {
        func("id", getFieldType<decltype(entity.id)>(), &entity.id, true, false);
        func("name", getFieldType<decltype(entity.name)>(), &entity.name, false, false);
        func("email", getFieldType<decltype(entity.email)>(), &entity.email, false, false);
        func("age", getFieldType<decltype(entity.age)>(), &entity.age, false, false);
    }
};

}} // namespace hft::reflection

TEST(CatalogTest, RegisterTable) {
    Catalog::instance().clear();
    
    TableInfo table("users");
    table.addColumn(ColumnInfo("id", FieldType::INT64, true, false));
    table.addColumn(ColumnInfo("name", FieldType::STRING, false, false));
    table.addColumn(ColumnInfo("email", FieldType::STRING, false, false));
    
    Catalog::instance().registerTable(table);
    
    const TableInfo* retrieved = Catalog::instance().getTable("users");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->name, "users");
    EXPECT_EQ(retrieved->columns.size(), 3);
}

TEST(CatalogTest, GetColumn) {
    Catalog::instance().clear();
    
    TableInfo table("products");
    table.addColumn(ColumnInfo("id", FieldType::INT64, true, false));
    table.addColumn(ColumnInfo("price", FieldType::DOUBLE, false, false));
    
    Catalog::instance().registerTable(table);
    
    const TableInfo* retrieved = Catalog::instance().getTable("products");
    ASSERT_NE(retrieved, nullptr);
    
    const ColumnInfo* col = retrieved->getColumn("price");
    ASSERT_NE(col, nullptr);
    EXPECT_EQ(col->name, "price");
    EXPECT_EQ(col->type, FieldType::DOUBLE);
}

TEST(CatalogTest, GetPrimaryKeys) {
    Catalog::instance().clear();
    
    TableInfo table("orders");
    table.addColumn(ColumnInfo("order_id", FieldType::INT64, true, false));
    table.addColumn(ColumnInfo("customer_id", FieldType::INT64, false, false));
    table.addColumn(ColumnInfo("total", FieldType::DOUBLE, false, false));
    
    Catalog::instance().registerTable(table);
    
    const TableInfo* retrieved = Catalog::instance().getTable("orders");
    ASSERT_NE(retrieved, nullptr);
    
    auto pks = retrieved->getPrimaryKeyColumns();
    ASSERT_EQ(pks.size(), 1);
    EXPECT_EQ(pks[0], "order_id");
}

TEST(CatalogTest, GenerateCreateTableSQL_PostgreSQL) {
    Catalog::instance().clear();
    
    TableInfo table("employees");
    table.addColumn(ColumnInfo("id", FieldType::INT64, true, false));
    table.addColumn(ColumnInfo("name", FieldType::STRING, false, false));
    table.addColumn(ColumnInfo("salary", FieldType::DOUBLE, false, false));
    
    Catalog::instance().registerTable(table);
    
    std::string sql = Catalog::instance().generateCreateTableSQL("employees", "postgresql");
    
    EXPECT_NE(sql.find("CREATE TABLE employees"), std::string::npos);
    EXPECT_NE(sql.find("id BIGINT PRIMARY KEY"), std::string::npos);
    EXPECT_NE(sql.find("name TEXT NOT NULL"), std::string::npos);
    EXPECT_NE(sql.find("salary DOUBLE PRECISION NOT NULL"), std::string::npos);
}

TEST(CatalogTest, GenerateCreateTableSQL_Sybase) {
    Catalog::instance().clear();
    
    TableInfo table("customers");
    table.addColumn(ColumnInfo("id", FieldType::INT32, true, false));
    table.addColumn(ColumnInfo("name", FieldType::STRING, false, false));
    
    Catalog::instance().registerTable(table);
    
    std::string sql = Catalog::instance().generateCreateTableSQL("customers", "sybase");
    
    EXPECT_NE(sql.find("CREATE TABLE customers"), std::string::npos);
    EXPECT_NE(sql.find("id INT PRIMARY KEY"), std::string::npos);
    EXPECT_NE(sql.find("name VARCHAR(255) NOT NULL"), std::string::npos);
}

TEST(CatalogTest, GetTableNames) {
    Catalog::instance().clear();
    
    TableInfo table1("table1");
    TableInfo table2("table2");
    
    Catalog::instance().registerTable(table1);
    Catalog::instance().registerTable(table2);
    
    auto names = Catalog::instance().getTableNames();
    EXPECT_EQ(names.size(), 2);
}

TEST(CatalogTest, RegisterEntity) {
    Catalog::instance().clear();
    
    registerEntity<TestUser>();
    
    const TableInfo* table = Catalog::instance().getTable("test_users");
    ASSERT_NE(table, nullptr);
    EXPECT_EQ(table->name, "test_users");
    EXPECT_EQ(table->columns.size(), 4);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
