# Contributing to HFT Legacy Migration Framework

Thank you for your interest in contributing to the HFT Legacy Migration Framework! This document provides guidelines and instructions for contributing.

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help maintain a welcoming environment
- Follow project conventions and standards

## How to Contribute

### Reporting Bugs

When reporting bugs, please include:

1. **Description**: Clear description of the issue
2. **Steps to Reproduce**: Detailed steps to reproduce the bug
3. **Expected Behavior**: What you expected to happen
4. **Actual Behavior**: What actually happened
5. **Environment**: OS, compiler version, database version
6. **Code Sample**: Minimal code that reproduces the issue

Example bug report:

```
**Description**: PostgreSQL connection fails with invalid connection string

**Steps to Reproduce**:
1. Create PostgreSQLConnection
2. Call open() with connection string "invalid"
3. Check isOpen()

**Expected**: isOpen() returns false, getLastError() provides details
**Actual**: Segmentation fault

**Environment**: Ubuntu 22.04, GCC 11.2, PostgreSQL 14
```

### Suggesting Features

For feature requests, please:

1. Check if the feature already exists
2. Describe the use case clearly
3. Explain why this feature would be valuable
4. Provide example usage if possible

### Pull Request Process

1. **Fork the repository**
   ```bash
   git clone https://github.com/ChristophePichaud/HFT-LegacyMigrationFramework.git
   cd HFT-LegacyMigrationFramework
   git checkout -b feature/my-new-feature
   ```

2. **Make your changes**
   - Follow the coding style (see below)
   - Add tests for new functionality
   - Update documentation as needed
   - Ensure all tests pass

3. **Test your changes**
   ```bash
   mkdir build && cd build
   cmake .. -DWITH_POSTGRESQL=ON -DBUILD_TESTS=ON
   cmake --build .
   ctest
   ```

4. **Commit your changes**
   ```bash
   git add .
   git commit -m "Add feature: description of feature"
   ```

5. **Push to your fork**
   ```bash
   git push origin feature/my-new-feature
   ```

6. **Create Pull Request**
   - Go to the original repository
   - Click "New Pull Request"
   - Select your branch
   - Fill in the PR template
   - Wait for review

## Coding Standards

### C++ Style Guide

#### Naming Conventions

- **Classes/Structs**: PascalCase (e.g., `PostgreSQLConnection`, `EntityTraits`)
- **Functions/Methods**: camelCase (e.g., `createStatement`, `executeQuery`)
- **Variables**: camelCase (e.g., `connectionString`, `lastError`)
- **Constants**: UPPER_CASE (e.g., `MAX_CONNECTIONS`)
- **Private Members**: camelCase with trailing underscore (e.g., `conn_`, `dbHandle_`)

#### Code Organization

```cpp
#pragma once

// System includes
#include <string>
#include <memory>

// Project includes
#include "hft/db/IConnection.h"

namespace hft {
namespace db {

/**
 * @brief Brief description
 * 
 * Detailed description of the class.
 */
class MyClass {
public:
    // Public methods
    MyClass();
    ~MyClass();
    
    void publicMethod();

private:
    // Private methods
    void privateMethod();
    
    // Member variables
    int memberVariable_;
    std::string anotherMember_;
};

} // namespace db
} // namespace hft
```

#### Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Execute a SQL query and return results
 * 
 * @param sql SQL query string
 * @return Shared pointer to result set, nullptr on error
 * 
 * Example:
 * @code
 * auto stmt = conn->createStatement("SELECT * FROM users");
 * auto result = stmt->executeQuery();
 * @endcode
 */
std::shared_ptr<IResultSet> executeQuery(const std::string& sql);
```

#### Error Handling

- Return nullptr or std::optional for failures
- Provide getLastError() method for details
- Don't throw exceptions in core library code
- Log errors when appropriate

```cpp
std::optional<User> findById(int64_t id) {
    auto stmt = connection_->createStatement("SELECT * FROM users WHERE id = $1");
    if (!stmt) {
        // Log error
        return std::nullopt;
    }
    
    stmt->bindLong(1, id);
    auto result = stmt->executeQuery();
    
    if (!result || !result->next()) {
        return std::nullopt;
    }
    
    return mapResultToEntity(result);
}
```

### CMake Conventions

- Use modern CMake (3.15+)
- Add options for optional features
- Support cross-platform builds
- Use FetchContent for dependencies

```cmake
option(WITH_FEATURE "Enable feature X" OFF)

if(WITH_FEATURE)
    find_package(FeatureLib REQUIRED)
    target_link_libraries(hft-legacy-migration FeatureLib::FeatureLib)
endif()
```

### Testing Requirements

#### Unit Tests

Every new feature must have unit tests:

```cpp
#include <gtest/gtest.h>
#include "MyNewFeature.h"

TEST(MyNewFeatureTest, BasicFunctionality) {
    MyNewFeature feature;
    EXPECT_TRUE(feature.works());
}

TEST(MyNewFeatureTest, EdgeCase) {
    MyNewFeature feature;
    feature.setParameter(nullptr);
    EXPECT_FALSE(feature.works());
}
```

#### Integration Tests

For database-related features:

```cpp
TEST(DatabaseIntegration, ConnectionWorks) {
    auto conn = std::make_shared<PostgreSQLConnection>();
    
    // Skip if no database available
    if (!conn->open("host=localhost ...")) {
        GTEST_SKIP() << "Database not available";
    }
    
    EXPECT_TRUE(conn->isOpen());
    conn->close();
}
```

#### Test Coverage

- Aim for >80% code coverage
- Test happy paths and error cases
- Test edge cases and boundary conditions
- Use mocks for external dependencies

## Documentation Requirements

### Update Documentation

When adding features, update:

1. **README.md**: Add to feature list if major feature
2. **DEVELOPER_GUIDE.md**: Add usage examples
3. **Code comments**: Add Doxygen documentation
4. **Examples**: Add example if applicable

### Documentation Style

- Keep it concise but complete
- Provide code examples
- Explain why, not just what
- Keep examples runnable

## Database Adapter Guidelines

When adding a new database adapter:

1. Implement all interface methods
2. Handle connection pooling if possible
3. Support prepared statements
4. Implement transactions with RAII
5. Provide clear error messages
6. Add CMake detection logic
7. Add unit tests
8. Add integration tests
9. Update README with dependencies
10. Add example usage

## Review Process

### What Reviewers Look For

1. **Correctness**: Does it work as intended?
2. **Tests**: Are there adequate tests?
3. **Documentation**: Is it well documented?
4. **Style**: Does it follow conventions?
5. **Performance**: Are there performance concerns?
6. **Security**: Are there security issues?
7. **Compatibility**: Does it work cross-platform?

### Response Time

- Initial review: Within 1 week
- Follow-up reviews: Within 3 days
- Simple fixes: Can be merged quickly
- Major features: May require multiple review cycles

## Release Process

1. Update version in CMakeLists.txt
2. Update CHANGELOG.md
3. Tag release: `git tag v1.0.0`
4. Push tag: `git push origin v1.0.0`
5. Create GitHub release with notes

## Getting Help

- **Issues**: Open a GitHub issue for bugs or questions
- **Discussions**: Use GitHub Discussions for general questions
- **Email**: Contact maintainers directly for private concerns

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Recognition

Contributors will be:
- Listed in the README
- Mentioned in release notes
- Credited in CONTRIBUTORS.md (if significant contributions)

## Questions?

If you have any questions about contributing, please:
1. Check existing documentation
2. Search closed issues
3. Ask in GitHub Discussions
4. Contact the maintainers

Thank you for contributing to making the HFT Legacy Migration Framework better!
