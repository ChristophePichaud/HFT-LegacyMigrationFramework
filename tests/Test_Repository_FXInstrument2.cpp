#include <gtest/gtest.h>
#include "MockConnection.hpp"
#include "entity/generated/FXInstrument2.hpp"
#include "repository/generated/Repository_FXInstrument2.hpp"

TEST(RepositoryFXInstrument2, GetAllBuildsSelect) {
    MockConnection conn;
    Repository_FXInstrument2 repo(conn);

    auto all = repo.getAll();
    (void)all;

    EXPECT_EQ(conn.lastQuery(), "SELECT * FROM FXInstrument2");
}
