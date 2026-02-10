#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include "db/IDBPreparedStatement.hpp"

// Prepared statement cache to avoid re-preparing the same SQL
class PreparedStatementCache {
public:
    static PreparedStatementCache& instance() {
        static PreparedStatementCache cache;
        return cache;
    }

    // Get or create a prepared statement
    std::shared_ptr<IDBPreparedStatement> get(
        const std::string& sql,
        std::function<std::unique_ptr<IDBPreparedStatement>()> factory) {
        
        std::lock_guard<std::mutex> lock(_mutex);
        
        auto it = _cache.find(sql);
        if (it != _cache.end() && !it->second.expired()) {
            return it->second.lock();
        }
        
        // Create new statement
        auto stmt = factory();
        auto sharedStmt = std::shared_ptr<IDBPreparedStatement>(std::move(stmt));
        _cache[sql] = sharedStmt;
        
        return sharedStmt;
    }

    // Clear cache
    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.clear();
    }

    // Get cache statistics
    size_t size() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cache.size();
    }

private:
    PreparedStatementCache() = default;
    ~PreparedStatementCache() = default;
    
    // Delete copy constructor and assignment operator
    PreparedStatementCache(const PreparedStatementCache&) = delete;
    PreparedStatementCache& operator=(const PreparedStatementCache&) = delete;

    mutable std::mutex _mutex;
    std::unordered_map<std::string, std::weak_ptr<IDBPreparedStatement>> _cache;
};
