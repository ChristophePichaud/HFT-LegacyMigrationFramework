#pragma once
#include "db/IDBConnection.hpp"
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>

class ConnectionPool {
public:
    using ConnectionFactory = std::function<std::unique_ptr<IDBConnection>()>;
    
    ConnectionPool(ConnectionFactory factory, size_t poolSize = 10, 
                   std::chrono::seconds connectionTimeout = std::chrono::seconds(30))
        : _factory(factory), 
          _poolSize(poolSize), 
          _connectionTimeout(connectionTimeout),
          _shutdown(false) {
        // Pre-create connections
        for (size_t i = 0; i < poolSize; ++i) {
            try {
                auto conn = _factory();
                if (conn) {
                    _availableConnections.push(std::move(conn));
                }
            } catch (...) {
                // Log error but continue
            }
        }
    }
    
    ~ConnectionPool() {
        shutdown();
    }
    
    // Get a connection from the pool
    std::unique_ptr<IDBConnection> acquire() {
        std::unique_lock<std::mutex> lock(_mutex);
        
        // Wait for a connection to become available
        if (!_cv.wait_for(lock, _connectionTimeout, [this] { 
            return !_availableConnections.empty() || _shutdown; 
        })) {
            throw std::runtime_error("Connection pool timeout");
        }
        
        if (_shutdown) {
            throw std::runtime_error("Connection pool is shutting down");
        }
        
        // Get connection from pool
        auto conn = std::move(_availableConnections.front());
        _availableConnections.pop();
        
        // Validate connection health
        if (!isConnectionHealthy(conn.get())) {
            // Try to create a new connection
            conn = _factory();
        }
        
        return conn;
    }
    
    // Return a connection to the pool
    void release(std::unique_ptr<IDBConnection> conn) {
        if (!conn) return;
        
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (_shutdown) {
            return;  // Don't return connections during shutdown
        }
        
        // Check if connection is still healthy
        if (isConnectionHealthy(conn.get())) {
            _availableConnections.push(std::move(conn));
        } else {
            // Create a new connection to replace the bad one
            try {
                auto newConn = _factory();
                if (newConn) {
                    _availableConnections.push(std::move(newConn));
                }
            } catch (...) {
                // Log error
            }
        }
        
        _cv.notify_one();
    }
    
    // Get pool statistics
    size_t availableConnections() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _availableConnections.size();
    }
    
    size_t poolSize() const {
        return _poolSize;
    }
    
    // Shutdown the pool
    void shutdown() {
        std::lock_guard<std::mutex> lock(_mutex);
        _shutdown = true;
        
        // Clear all connections
        while (!_availableConnections.empty()) {
            _availableConnections.pop();
        }
        
        _cv.notify_all();
    }

private:
    // Check if connection is healthy (simple implementation)
    bool isConnectionHealthy(IDBConnection* conn) {
        if (!conn) return false;
        
        try {
            // Try a simple query to test connection
            // This is a simplified check - real implementation would be database-specific
            return true;  // Assume healthy for now
        } catch (...) {
            return false;
        }
    }
    
    ConnectionFactory _factory;
    size_t _poolSize;
    std::chrono::seconds _connectionTimeout;
    bool _shutdown;
    
    mutable std::mutex _mutex;
    std::condition_variable _cv;
    std::queue<std::unique_ptr<IDBConnection>> _availableConnections;
};

// RAII wrapper for automatic connection release
class PooledConnection {
public:
    PooledConnection(ConnectionPool& pool)
        : _pool(pool), _conn(_pool.acquire()) {}
    
    ~PooledConnection() {
        if (_conn) {
            _pool.release(std::move(_conn));
        }
    }
    
    // Disable copy
    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;
    
    // Enable move
    PooledConnection(PooledConnection&& other) noexcept
        : _pool(other._pool), _conn(std::move(other._conn)) {}
    
    IDBConnection* get() { return _conn.get(); }
    IDBConnection* operator->() { return _conn.get(); }
    IDBConnection& operator*() { return *_conn; }

private:
    ConnectionPool& _pool;
    std::unique_ptr<IDBConnection> _conn;
};
