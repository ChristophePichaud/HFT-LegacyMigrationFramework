#include <gtest/gtest.h>
#include "hft/db/SybaseConnection.h"
#include <memory>

using namespace hft::db;

class SybaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        conn = std::make_shared<SybaseConnection>();
    }
    
    void TearDown() override {
        if (conn->isOpen()) {
            conn->close();
        }
    }
    
    std::shared_ptr<SybaseConnection> conn;
};

TEST_F(SybaseTest, ConnectionOpenClose) {
    // Note: This test requires a running Sybase server
    // Connection string format: "server=hostname;user=username;password=pwd;database=dbname"
    // Skip if connection fails (no server available)
    bool opened = conn->open("server=localhost;user=sa;password=password;database=master");
    
    if (opened) {
        EXPECT_TRUE(conn->isOpen());
        conn->close();
        EXPECT_FALSE(conn->isOpen());
    } else {
        GTEST_SKIP() << "Sybase server not available: " << conn->getLastError();
    }
}

TEST_F(SybaseTest, SimpleQuery) {
    bool opened = conn->open("server=localhost;user=sa;password=password;database=master");
    
    if (!opened) {
        GTEST_SKIP() << "Sybase server not available";
    }
    
    bool result = conn->execute("SELECT 1");
    EXPECT_TRUE(result);
}

TEST_F(SybaseTest, Transaction) {
    bool opened = conn->open("server=localhost;user=sa;password=password;database=master");
    
    if (!opened) {
        GTEST_SKIP() << "Sybase server not available";
    }
    
    auto txn = conn->beginTransaction();
    ASSERT_NE(txn, nullptr);
    EXPECT_TRUE(txn->isActive());
    
    EXPECT_TRUE(txn->commit());
    EXPECT_FALSE(txn->isActive());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
