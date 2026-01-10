/**
 * @file ResourceTier.hpp
 * @brief Resource location tier/level enumeration
 */

#pragma once

#include <QMap>
#include <QString>

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

/**
 * @brief Display names for resource tier enum values
 * 
 * Use this static const lookup table for displaying tier names in the GUI.
 * This follows the pattern of using static lookup tables for enum display names.
 */
inline static const QMap<ResourceTier, QString> tierDisplayNames = {
    { ResourceTier::Installation, QStringLiteral("Installation") },
    { ResourceTier::Machine, QStringLiteral("Machine") },
    { ResourceTier::User, QStringLiteral("User") }
};

} // namespace resourceMetadata
