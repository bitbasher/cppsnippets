#pragma once

#include <QString>
#include <QList>

namespace resourceInfo {

/**
 * @brief Resource location tier/level
 * 
 * Resources are organized into three tiers based on their scope and accessibility:
 * - Installation: Built-in resources from the application installation (read-only)
 * - Machine: System-wide resources available to all users (admin-managed)
 * - User: Personal resources in the user's home directory (user-managed)
 */
enum class ResourceTier {
    Installation,   ///< Built-in application resources (read-only, hardcoded)
    Machine,        ///< System-wide resources for all users (admin-managed)
    User            ///< Per-user resources (user-managed)
};


// Access modes applicable to tiers and resources
enum class Access {
    Undefined = 0,
    FullAccess,   // Read + Write + Execute
    ReadWrite,    // Read + Write
    ReadOnly,     // Read only
    WriteOnly,    // Write only
    NoAccess      // No access
};

class ResourceTierInfo {
public:
    // Display names for GUI
    static QString displayName(ResourceTier tier) {
        switch (tier) {
            case ResourceTier::Installation: return QStringLiteral("Installation");
            case ResourceTier::Machine:      return QStringLiteral("Machine");
            case ResourceTier::User:         return QStringLiteral("User");
        }
        return QString();
    }

    // Default access rules by tier
    static Access defaultAccess(ResourceTier tier) {
        switch (tier) {
            case ResourceTier::Installation: return Access::ReadOnly;
            case ResourceTier::Machine:      return Access::ReadOnly;
            case ResourceTier::User:     return Access::ReadWrite;
        }
        return Access::Undefined;
    }

    static bool isReadOnly(ResourceTier tier) {
        return defaultAccess(tier) == Access::ReadOnly;
    }

    static bool isWritable(ResourceTier tier) {
        const Access a = defaultAccess(tier);
        return a == Access::ReadWrite || a == Access::WriteOnly || a == Access::FullAccess;
    }

    static const QList<ResourceTier>& allTiers() {
        return s_allTiers;
    }

private:
    inline static const QList<ResourceTier> s_allTiers = {
        ResourceTier::Installation,
        ResourceTier::Machine,
        ResourceTier::User
    };
};

} // namespace resourceInfo
