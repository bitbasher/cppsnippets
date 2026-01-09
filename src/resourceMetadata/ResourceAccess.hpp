/**
 * @file ResourceAccess.hpp
 * @brief Resource access permissions enumeration and tier mapping
 */

#pragma once

#include "ResourceTier.hpp"
#include <QMap>

namespace resourceMetadata {

/**
 * @brief Access permissions for resources
 */
enum class Access {
    Unknown = 0,  ///< Access level not yet determined
    ReadOnly,     ///< Read-only access
    ReadWrite,    ///< Read and write access
    WriteOnly,    ///< Write-only access
    FullAccess,   ///< All permissions (read, write, execute)
    NoAccess      ///< No access
};

/**
 * @brief Default access level for each resource tier
 * 
 * Maps resource tiers to their default access permissions.
 * Actual discovered resources may have different permissions.
 */
static const QMap<ResourceTier, Access> accessByTier = {
    { ResourceTier::Installation, Access::ReadOnly },
    { ResourceTier::Machine, Access::ReadOnly },
    { ResourceTier::User, Access::ReadWrite }
};

} // namespace resourceMetadata
