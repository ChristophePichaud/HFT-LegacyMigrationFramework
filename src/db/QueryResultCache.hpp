#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <any>

// Simple query result cache with TTL
class QueryResultCache {
public:
    static QueryResultCache& instance() {
        static QueryResultCache cache;
        return cache;
    }

    // Cache entry with expiration
    struct CacheEntry {
        std::any data;
        std::chrono::steady_clock::time_point expiresAt;
        
        bool isExpired() const {
            return std::chrono::steady_clock::now() >= expiresAt;
        }
    };

    // Put a result in the cache
    void put(const std::string& key, const std::any& data, 
             std::chrono::seconds ttl = std::chrono::seconds(300)) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        CacheEntry entry;
        entry.data = data;
        entry.expiresAt = std::chrono::steady_clock::now() + ttl;
        
        _cache[key] = entry;
    }

    // Get a result from the cache
    std::any get(const std::string& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        auto it = _cache.find(key);
        if (it == _cache.end()) {
            return std::any();
        }
        
        // Check if expired
        if (it->second.isExpired()) {
            _cache.erase(it);
            return std::any();
        }
        
        return it->second.data;
    }

    // Check if key exists and is valid
    bool contains(const std::string& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        auto it = _cache.find(key);
        if (it == _cache.end()) {
            return false;
        }
        
        if (it->second.isExpired()) {
            _cache.erase(it);
            return false;
        }
        
        return true;
    }

    // Clear cache
    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.clear();
    }

    // Remove expired entries
    void cleanup() {
        std::lock_guard<std::mutex> lock(_mutex);
        
        auto it = _cache.begin();
        while (it != _cache.end()) {
            if (it->second.isExpired()) {
                it = _cache.erase(it);
            } else {
                ++it;
            }
        }
    }

    // Get cache statistics
    size_t size() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cache.size();
    }

    // Enable/disable caching
    void setEnabled(bool enabled) {
        std::lock_guard<std::mutex> lock(_mutex);
        _enabled = enabled;
        if (!enabled) {
            _cache.clear();
        }
    }

    bool isEnabled() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _enabled;
    }

private:
    QueryResultCache() : _enabled(true) {}
    ~QueryResultCache() = default;
    
    // Delete copy constructor and assignment operator
    QueryResultCache(const QueryResultCache&) = delete;
    QueryResultCache& operator=(const QueryResultCache&) = delete;

    mutable std::mutex _mutex;
    std::unordered_map<std::string, CacheEntry> _cache;
    bool _enabled;
};
