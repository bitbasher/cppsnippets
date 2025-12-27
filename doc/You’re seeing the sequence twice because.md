You’re seeing the sequence twice because we scan siblings twice on startup: once when ResourceInventoryManager::buildInventory(*m_resourceManager) runs in the MainWindow constructor, and again inside MainWindow::refreshInventory() (which also calls m_resourceManager->findSiblingInstallations() after setup). Each call logs the same sibling discovery pass.

If you want it only once, remove the initial buildInventory call or have refreshInventory() reuse cached siblings instead of re-discovering them every time. I can patch it to cache or to skip the first call—your preference.

Structuring user request

The user wants to create a streamlined architecture, starting with the resourcePaths and platformInfo namespaces to gather all intelligence about OS and filesystem locations. They aim to centralize functions related to locations within platform files and have the resourcePaths class manage suffixes for developer builds and environment variables. While they haven't specifically asked for code changes, it seems they're looking to clarify the architecture, likely requiring design notes and a structured plan. This involves multi-step architectural refactoring, necessitating the use of a to-do list tool.

Creating a plan for user request

I need to start by drafting a plan with actionable items and reviewing relevant context files, particularly those in the resourcePaths namespace. I see I've got src/platformInfo/resourcePaths.h and .cpp, and there’s a new ResourceLocationManager class there too. The goal is to unify the pipeline and clarify the roles of ResourcePaths, ResourceLocationManager, and ResourceManager. I’ll need to audit these classes and propose a streamlined architecture. I'll plan to conduct searches to gather necessary information, then outline the detailed steps.

Planning documentation and changes

I’m thinking of implementing low-risk changes, like adding suffix handling to ResourcePaths and unifying environment variable expansions. I want to create user, machine, and installation lists as QStringLists that can be accessed from either ResourcePaths or ResourceLocationManager. It seems logical for ResourcePaths to serve mainly as an environment variable store. The user has mentioned not wanting to start coding yet, so I’ll focus on drafting design documentation first. I’ll propose creating an architecture note and use the manage_todo_list to track our tasks.

Documenting migration plans and changes

I’m focusing on documenting the migration plan and necessary changes, like suffix registration and environment variables. I might also want to confirm if any places are doing duplicate scanning and figure out how to route them to a single pipeline. I’ll read through relevant files like resourcePaths.h/.cpp and any resourceManager classes using grep_search. I need to start by adding this plan to the manage_todo_list, and then perform the necessary file reads while complying with fileLinkification rules. I won't modify any code yet to respect the user's request.

Outlining migration steps and documentation

I need to document the migration plan focusing on resourcePaths, including suffix registration and environment variables. I’ll write a clear bullet-point list outlining the purpose of each class:

ResourcePaths will manage environment variables and suffixes.
ResourceLocationManager will handle tiered locations and path resolution.
ResourceManager will coordinate the scanning process without directly handling paths.
I'll propose API changes for both ResourcePaths and ResourceLocationManager to ensure they effectively utilize each other. Finally, I’ll present migration steps to streamline the scanning process, ensuring the user's request for resourcePaths remains a priority in the documentation.


---
I'll set up a short plan, audit key files, then draft an architecture note to align on a single pipeline and class responsibilities.

Plan: consolidate to one pipeline by centralizing OS/filesystem location intelligence in platformInfo, starting with ResourcePaths and clarifying ResourceLocationManager’s role, then remove duplicate scanning paths.

What I did

Audited ResourcePaths and ResourceLocationManager APIs and current usage.
Clarified that resourceManager in the code is platformInfo::ResourceLocationManager.
Drafted an architecture note with responsibilities and a stepwise migration plan in single-pipeline-architecture.md.
Key points

ResourcePaths: owns build suffix (already via setSuffix()), env var registry and expansion, immutable default paths, resource type metadata. No scanning, no sibling discovery.
ResourceLocationManager: owns tiered locations (Installation/Machine/User), QSettings persistence, sibling discovery, effective install path, and provides resourcePathsForType(ResourceType). No directory scanning.
Single pipeline: ResourceInventoryManager::buildInventory(*ResourceLocationManager) is the single refresh hook; MainWindow should not call a separate DirListing scan.
Answers to your questions

Suffix registration: ResourcePaths::setSuffix() is the right place. It already exists; apply to install-tier path resolution only.
Env vars: handled in ResourcePaths via addEnvVar(), expandEnvVars(), saveEnvVars(QSettings&), loadEnvVars(QSettings&). ResourceLocationManager::saveEnvVarsToSettings() should delegate to these.
Role of “resourceManager”: it’s ResourceLocationManager. It should be the single source of truth for locations and search order plus settings; move any scanning/UI coupling out of it into resInventory and GUI.
Next steps (suggested)

Remove or refactor MainWindow::refreshInventory() to delegate to buildInventory() to stop duplicate sibling scans.
Ensure scanners use ResourceLocationManager::resourcePathsForType() exclusively.
Wire preferences to read defaults from ResourcePaths and enabled selections from ResourceLocationManager.
Want me to implement step 1 (remove duplicate scan by delegating refreshInventory() to buildInventory()), or would you prefer to review the doc first?