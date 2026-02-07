#pragma once

#include <memory>
#include <string>

namespace hft {
namespace db {

class IStatement;
class ITransaction;

/**
 * @brief Database connection interface
 */
class IConnection {
public:
    virtual ~IConnection() = default;

    /**
     * @brief Open a database connection
     * @param connectionString Database-specific connection string
     * @return true if successful, false otherwise
     */
    virtual bool open(const std::string& connectionString) = 0;

    /**
     * @brief Close the database connection
     */
    virtual void close() = 0;

    /**
     * @brief Check if connection is open
     * @return true if connected, false otherwise
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Create a prepared statement
     * @param sql SQL query string
     * @return Shared pointer to statement
     */
    virtual std::shared_ptr<IStatement> createStatement(const std::string& sql) = 0;

    /**
     * @brief Begin a transaction
     * @return Shared pointer to transaction
     */
    virtual std::shared_ptr<ITransaction> beginTransaction() = 0;

    /**
     * @brief Execute a simple SQL command without result
     * @param sql SQL command
     * @return true if successful
     */
    virtual bool execute(const std::string& sql) = 0;

    /**
     * @brief Get last error message
     * @return Error message string
     */
    virtual std::string getLastError() const = 0;
};

} // namespace db
} // namespace hft
