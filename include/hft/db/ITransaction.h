#pragma once

namespace hft {
namespace db {

/**
 * @brief Transaction interface
 */
class ITransaction {
public:
    virtual ~ITransaction() = default;

    /**
     * @brief Commit the transaction
     * @return true if successful
     */
    virtual bool commit() = 0;

    /**
     * @brief Rollback the transaction
     * @return true if successful
     */
    virtual bool rollback() = 0;

    /**
     * @brief Check if transaction is active
     * @return true if active
     */
    virtual bool isActive() const = 0;
};

} // namespace db
} // namespace hft
