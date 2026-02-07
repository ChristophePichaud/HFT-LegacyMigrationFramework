#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace hft {
namespace db {

class IResultSet;

/**
 * @brief Prepared statement interface
 */
class IStatement {
public:
    virtual ~IStatement() = default;

    /**
     * @brief Bind an integer parameter
     * @param index Parameter index (1-based)
     * @param value Integer value
     */
    virtual void bindInt(int index, int32_t value) = 0;

    /**
     * @brief Bind a long parameter
     * @param index Parameter index (1-based)
     * @param value Long value
     */
    virtual void bindLong(int index, int64_t value) = 0;

    /**
     * @brief Bind a double parameter
     * @param index Parameter index (1-based)
     * @param value Double value
     */
    virtual void bindDouble(int index, double value) = 0;

    /**
     * @brief Bind a string parameter
     * @param index Parameter index (1-based)
     * @param value String value
     */
    virtual void bindString(int index, const std::string& value) = 0;

    /**
     * @brief Bind a NULL parameter
     * @param index Parameter index (1-based)
     */
    virtual void bindNull(int index) = 0;

    /**
     * @brief Execute the statement and return results
     * @return Shared pointer to result set
     */
    virtual std::shared_ptr<IResultSet> executeQuery() = 0;

    /**
     * @brief Execute an update/insert/delete statement
     * @return Number of affected rows
     */
    virtual int executeUpdate() = 0;

    /**
     * @brief Reset the statement for reuse
     */
    virtual void reset() = 0;
};

} // namespace db
} // namespace hft
