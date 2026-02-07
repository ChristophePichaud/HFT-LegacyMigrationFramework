#include "hft/db/PostgreSQLConnection.h"
#include "hft/orm/Repository.h"
#include "hft/reflection/EntityTraits.h"
#include "hft/catalog/Catalog.h"
#include <iostream>

// Define a User entity
struct User {
    int64_t id = 0;
    std::string name = "";
    std::string email = "";
    int32_t age = 0;
};

// Define entity traits
namespace hft { namespace reflection {

template<>
struct EntityTraits<User> {
    static constexpr const char* tableName() { return "users"; }
    static constexpr size_t fieldCount() { return 4; }
    
    template<typename Func>
    static void forEachField(User& entity, Func&& func) {
        func("id", getFieldType<decltype(entity.id)>(), &entity.id, true, false);
        func("name", getFieldType<decltype(entity.name)>(), &entity.name, false, false);
        func("email", getFieldType<decltype(entity.email)>(), &entity.email, false, false);
        func("age", getFieldType<decltype(entity.age)>(), &entity.age, false, false);
    }
};

}} // namespace hft::reflection

int main() {
    std::cout << "=== HFT Legacy Migration Framework - ORM Usage ===" << std::endl;
    
    // Create connection
    auto conn = std::make_shared<hft::db::PostgreSQLConnection>();
    
    std::cout << "Connecting to PostgreSQL..." << std::endl;
    if (!conn->open("host=localhost port=5432 dbname=test user=postgres password=postgres")) {
        std::cerr << "Failed to connect: " << conn->getLastError() << std::endl;
        return 1;
    }
    
    // Register entity in catalog
    hft::catalog::registerEntity<User>();
    
    // Generate and execute CREATE TABLE
    std::string createSQL = hft::catalog::Catalog::instance().generateCreateTableSQL("users", "postgresql");
    std::cout << "Creating table:" << std::endl;
    std::cout << createSQL << std::endl;
    
    conn->execute("DROP TABLE IF EXISTS users");
    conn->execute(createSQL);
    
    // Create repository
    hft::orm::Repository<User> userRepo(conn);
    
    // Insert a user
    std::cout << "\nInserting user..." << std::endl;
    User newUser;
    newUser.name = "John Doe";
    newUser.email = "john@example.com";
    newUser.age = 30;
    
    if (userRepo.insert(newUser)) {
        std::cout << "User inserted successfully!" << std::endl;
    }
    
    // Find all users
    std::cout << "\nFinding all users..." << std::endl;
    auto users = userRepo.findAll();
    
    std::cout << "Found " << users.size() << " user(s):" << std::endl;
    for (const auto& user : users) {
        std::cout << "  ID: " << user.id << ", Name: " << user.name 
                  << ", Email: " << user.email << ", Age: " << user.age << std::endl;
    }
    
    // Update user
    if (!users.empty()) {
        std::cout << "\nUpdating user..." << std::endl;
        User& user = users[0];
        user.age = 31;
        
        if (userRepo.update(user)) {
            std::cout << "User updated successfully!" << std::endl;
        }
    }
    
    // Delete user
    if (!users.empty()) {
        std::cout << "\nDeleting user..." << std::endl;
        if (userRepo.deleteById(users[0].id)) {
            std::cout << "User deleted successfully!" << std::endl;
        }
    }
    
    conn->close();
    std::cout << "\nConnection closed." << std::endl;
    
    return 0;
}
