#pragma once

#include <string>
#include <cstdint>

namespace hft {
namespace db {

/**
 * @brief Result set interface for query results
 */
class IResultSet {
public:
    virtual ~IResultSet() = default;

    /**
     * @brief Move to the next row
     * @return true if there is a next row, false if end reached
     */
    virtual bool next() = 0;

    /**
     * @brief Get integer value from column
     * @param index Column index (0-based)
     * @return Integer value
     */
    virtual int32_t getInt(int index) const = 0;

    /**
     * @brief Get long value from column
     * @param index Column index (0-based)
     * @return Long value
     */
    virtual int64_t getLong(int index) const = 0;

    /**
     * @brief Get double value from column
     * @param index Column index (0-based)
     * @return Double value
     */
    virtual double getDouble(int index) const = 0;

    /**
     * @brief Get string value from column
     * @param index Column index (0-based)
     * @return String value
     */
    virtual std::string getString(int index) const = 0;

    /**
     * @brief Check if column value is NULL
     * @param index Column index (0-based)
     * @return true if NULL, false otherwise
     */
    virtual bool isNull(int index) const = 0;

    /**
     * @brief Get column count
     * @return Number of columns
     */
    virtual int getColumnCount() const = 0;

    /**
     * @brief Get column name
     * @param index Column index (0-based)
     * @return Column name
     */
    virtual std::string getColumnName(int index) const = 0;
};

} // namespace db
} // namespace hft
