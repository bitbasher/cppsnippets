/**
 * @file Inventories.hpp
 * @brief External declarations for global inventory instances
 * 
 * These global pointers are defined in main.cpp and provide access to
 * the application's resource inventory models from any translation unit.
 */

#pragma once

namespace resourceInventory {
    class TemplatesInventory;
    class ExamplesInventory;
    class UnknownInventory;
}

// Global inventory instances defined in main.cpp
// Implemented inventories
extern resourceInventory::TemplatesInventory* g_templatesInventory;
extern resourceInventory::ExamplesInventory* g_examplesInventory;

// Placeholder inventories for unimplemented resource types
extern resourceInventory::UnknownInventory* g_fontsInventory;
extern resourceInventory::UnknownInventory* g_shadersInventory;
extern resourceInventory::UnknownInventory* g_librariesInventory;
extern resourceInventory::UnknownInventory* g_testsInventory;
extern resourceInventory::UnknownInventory* g_translationsInventory;
extern resourceInventory::UnknownInventory* g_colorSchemesInventory;
extern resourceInventory::UnknownInventory* g_unknownInventory;
