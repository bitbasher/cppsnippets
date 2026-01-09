/**
 * @file ResourceTier.hpp
 * @brief Resource location tier/level enumeration
 */

#pragma once

namespace resourceMetadata {

/**
 * @brief Resource location tier/level
 * 
 * Resources are organized into three tiers based on their scope and accessibility:
 * - Installation: Built-in resources from the application installation (read-only)
 * - Machine: System-wide resources available to all users (read-only)
 * - User: Personal resources in the user's home directory (read-write)
 */
enum class ResourceTier {
    Installation,   ///< Built-in application resources (read-only)
    Machine,        ///< System-wide resources for all users (read-only)
    User            ///< Per-user resources (read-write)
};

} // namespace resourceMetadata
