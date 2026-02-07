#include <gtest/gtest.h>
#include "hft/orm/Repository.h"
#include "hft/reflection/EntityTraits.h"
#include <memory>

using namespace hft::orm;

// Mock connection for testing
class MockConnection : public hft::db::IConnection {
public:
    bool open(const std::string&) override { return true; }
    void close() override {}
    bool isOpen() const override { return true; }
    
    std::shared_ptr<hft::db::IStatement> createStatement(const std::string& sql) override {
        lastSQL = sql;
        return nullptr;
    }
    
    std::shared_ptr<hft::db::ITransaction> beginTransaction() override { return nullptr; }
    bool execute(const std::string&) override { return true; }
    std::string getLastError() const override { return ""; }
    
    std::string lastSQL;
};

// Test entity
struct Product {
    int64_t id = 0;
    std::string name = "";
    double price = 0.0;
    int32_t quantity = 0;
};

// Define traits for Product
namespace hft { namespace reflection {

template<>
struct EntityTraits<Product> {
    static constexpr const char* tableName() { return "products"; }
    static constexpr size_t fieldCount() { return 4; }
    
    template<typename Func>
    static void forEachField(Product& entity, Func&& func) {
        func("id", getFieldType<decltype(entity.id)>(), &entity.id, true, false);
        func("name", getFieldType<decltype(entity.name)>(), &entity.name, false, false);
        func("price", getFieldType<decltype(entity.price)>(), &entity.price, false, false);
        func("quantity", getFieldType<decltype(entity.quantity)>(), &entity.quantity, false, false);
    }
};

}} // namespace hft::reflection

TEST(RepositoryTest, FindByIdGeneratesCorrectSQL) {
    auto mockConn = std::make_shared<MockConnection>();
    Repository<Product> repo(mockConn);
    
    // This will fail to execute but we can check SQL generation
    repo.findById(123);
    
    EXPECT_NE(mockConn->lastSQL.find("SELECT * FROM products"), std::string::npos);
    EXPECT_NE(mockConn->lastSQL.find("WHERE id = $1"), std::string::npos);
}

TEST(RepositoryTest, FindAllGeneratesCorrectSQL) {
    auto mockConn = std::make_shared<MockConnection>();
    Repository<Product> repo(mockConn);
    
    repo.findAll();
    
    EXPECT_EQ(mockConn->lastSQL, "SELECT * FROM products");
}

TEST(RepositoryTest, InsertGeneratesCorrectSQL) {
    auto mockConn = std::make_shared<MockConnection>();
    Repository<Product> repo(mockConn);
    
    Product product;
    product.name = "Test Product";
    product.price = 99.99;
    product.quantity = 10;
    
    repo.insert(product);
    
    EXPECT_NE(mockConn->lastSQL.find("INSERT INTO products"), std::string::npos);
    EXPECT_NE(mockConn->lastSQL.find("name"), std::string::npos);
    EXPECT_NE(mockConn->lastSQL.find("price"), std::string::npos);
    EXPECT_NE(mockConn->lastSQL.find("quantity"), std::string::npos);
    // id should not be in insert (auto-increment)
    EXPECT_EQ(mockConn->lastSQL.find("(id,"), std::string::npos);
}

TEST(RepositoryTest, UpdateGeneratesCorrectSQL) {
    auto mockConn = std::make_shared<MockConnection>();
    Repository<Product> repo(mockConn);
    
    Product product;
    product.id = 1;
    product.name = "Updated Product";
    product.price = 149.99;
    product.quantity = 5;
    
    repo.update(product);
    
    EXPECT_NE(mockConn->lastSQL.find("UPDATE products SET"), std::string::npos);
    EXPECT_NE(mockConn->lastSQL.find("WHERE id ="), std::string::npos);
}

TEST(RepositoryTest, DeleteByIdGeneratesCorrectSQL) {
    auto mockConn = std::make_shared<MockConnection>();
    Repository<Product> repo(mockConn);
    
    repo.deleteById(42);
    
    EXPECT_NE(mockConn->lastSQL.find("DELETE FROM products"), std::string::npos);
    EXPECT_NE(mockConn->lastSQL.find("WHERE id = $1"), std::string::npos);
}

TEST(RepositoryTest, DeleteAllGeneratesCorrectSQL) {
    auto mockConn = std::make_shared<MockConnection>();
    Repository<Product> repo(mockConn);
    
    repo.deleteAll();
    
    EXPECT_EQ(mockConn->lastSQL, "DELETE FROM products");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
