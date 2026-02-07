#include <gtest/gtest.h>
#include "hft/db/PostgreSQLConnection.h"
#include <memory>

using namespace hft::db;

class PostgreSQLTest : public ::testing::Test {
protected:
    void SetUp() override {
        conn = std::make_shared<PostgreSQLConnection>();
    }
    
    void TearDown() override {
        if (conn->isOpen()) {
            conn->close();
        }
    }
    
    std::shared_ptr<PostgreSQLConnection> conn;
};

TEST_F(PostgreSQLTest, ConnectionOpenClose) {
    // Note: This test requires a running PostgreSQL instance
    // Connection string format: "host=localhost port=5432 dbname=test user=postgres password=postgres"
    // Skip if connection fails (no server available)
    bool opened = conn->open("host=localhost port=5432 dbname=postgres user=postgres password=postgres");
    
    if (opened) {
        EXPECT_TRUE(conn->isOpen());
        conn->close();
        EXPECT_FALSE(conn->isOpen());
    } else {
        GTEST_SKIP() << "PostgreSQL server not available: " << conn->getLastError();
    }
}

TEST_F(PostgreSQLTest, SimpleQuery) {
    bool opened = conn->open("host=localhost port=5432 dbname=postgres user=postgres password=postgres");
    
    if (!opened) {
        GTEST_SKIP() << "PostgreSQL server not available";
    }
    
    bool result = conn->execute("SELECT 1");
    EXPECT_TRUE(result);
}

TEST_F(PostgreSQLTest, PreparedStatement) {
    bool opened = conn->open("host=localhost port=5432 dbname=postgres user=postgres password=postgres");
    
    if (!opened) {
        GTEST_SKIP() << "PostgreSQL server not available";
    }
    
    auto stmt = conn->createStatement("SELECT $1::int + $2::int AS sum");
    ASSERT_NE(stmt, nullptr);
    
    stmt->bindInt(1, 10);
    stmt->bindInt(2, 20);
    
    auto result = stmt->executeQuery();
    if (result && result->next()) {
        EXPECT_EQ(result->getInt(0), 30);
    }
}

TEST_F(PostgreSQLTest, Transaction) {
    bool opened = conn->open("host=localhost port=5432 dbname=postgres user=postgres password=postgres");
    
    if (!opened) {
        GTEST_SKIP() << "PostgreSQL server not available";
    }
    
    auto txn = conn->beginTransaction();
    ASSERT_NE(txn, nullptr);
    EXPECT_TRUE(txn->isActive());
    
    bool result = conn->execute("SELECT 1");
    EXPECT_TRUE(result);
    
    EXPECT_TRUE(txn->commit());
    EXPECT_FALSE(txn->isActive());
}

TEST_F(PostgreSQLTest, TransactionRollback) {
    bool opened = conn->open("host=localhost port=5432 dbname=postgres user=postgres password=postgres");
    
    if (!opened) {
        GTEST_SKIP() << "PostgreSQL server not available";
    }
    
    auto txn = conn->beginTransaction();
    ASSERT_NE(txn, nullptr);
    EXPECT_TRUE(txn->isActive());
    
    EXPECT_TRUE(txn->rollback());
    EXPECT_FALSE(txn->isActive());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
