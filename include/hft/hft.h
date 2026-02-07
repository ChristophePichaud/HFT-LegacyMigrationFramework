#pragma once

/**
 * @file hft.h
 * @brief Main header for HFT Legacy Migration Framework
 * 
 * This header provides convenient access to all framework components.
 */

// Database abstraction layer
#include "hft/db/IConnection.h"
#include "hft/db/IStatement.h"
#include "hft/db/IResultSet.h"
#include "hft/db/ITransaction.h"

// Database implementations
#ifdef WITH_POSTGRESQL
#include "hft/db/PostgreSQLConnection.h"
#endif

#ifdef WITH_SYBASE
#include "hft/db/SybaseConnection.h"
#endif

// ORM layer
#include "hft/orm/Repository.h"

// Reflection system
#include "hft/reflection/EntityTraits.h"

// Catalog
#include "hft/catalog/Catalog.h"

// Code generation
#include "hft/codegen/CodeGenerator.h"

/**
 * @namespace hft
 * @brief High Frequency Trading Legacy Migration Framework
 * 
 * This namespace contains all components of the legacy migration framework
 * including database abstraction, ORM, reflection, and code generation utilities.
 */
namespace hft {

/**
 * @namespace hft::db
 * @brief Database abstraction layer
 * 
 * Contains interfaces and implementations for database connectivity,
 * prepared statements, result sets, and transaction management.
 */
namespace db {}

/**
 * @namespace hft::orm
 * @brief Object-Relational Mapping layer
 * 
 * Provides repository pattern implementation and automatic CRUD operations
 * for entity types with compile-time reflection.
 */
namespace orm {}

/**
 * @namespace hft::reflection
 * @brief Compile-time reflection system
 * 
 * Entity traits and field metadata for compile-time type introspection
 * without runtime overhead.
 */
namespace reflection {}

/**
 * @namespace hft::catalog
 * @brief Runtime schema catalog
 * 
 * Runtime metadata management for tables, columns, and schema information.
 * Supports SQL generation for multiple database dialects.
 */
namespace catalog {}

/**
 * @namespace hft::codegen
 * @brief Code generation utilities
 * 
 * Automatic generation of entity classes, repositories, and SQL DDL
 * from schema metadata.
 */
namespace codegen {}

} // namespace hft
