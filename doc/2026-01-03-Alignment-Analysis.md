Alignment Analysis
✅ Strengths of the plan:

ResourcePaths correctly positioned
    serves as "where to look" reference with immutable defaults + runtime path resolution base on expanding environment variables
    separates concerns: ResourcePaths (defaults), ResourceDiscovery (scanning), ResourceRegistry (inventory of resources)
    Tiers only come into it as a way to group default paths with different access levels (installation, machine, user)
    paths have access controls (installation and machine are readonly, user / personal folder are read/write by user) 
ResourceDiscovery - properly separated
    scans according to the resourcePath list
    returns locations, which are folder that contain resources (not the resources themselves)
    populates the resoruce inventory with the individual resources
    some resources will be read in to be able to use them, and possibly modify them
    others will just have their metadata recorded in the inventory so the parts 
    of the app that need them can find them
ResourceLocation 
    good encapsulation of individual location metadata
    not to be used to record which resource locations are disabled as that is for settigns persistence only
    It might be useful to make the class for Locations similar to resourceTypeInfo class
    and then have a storage class (based on a QList? or a tree?) but in any case it should be built to work with Qt model/view framework 

resourceSettings (new)
    handles persistence of user-enabled/disabled locations
    handles persistence of user-added custom locations
    once recorded metadata is kept until the user specifically removes a location
    no automatic removal of locations that no longer exist on disk but such will be marked as such in the GUI

ResourceRegistry (repurposed ResourceLocationManager) 
    holds the individual resources
      resources that can be modified (e.g. color schemes, templates)
        others held as metadata only (e.g. examples, tests, fonts, shaders)
    note that examples and tests are .scad scripts that the user may modify where they have r-w access, and where they do not (install and machine tiers) they can copy into their user tier to modify
    provides API to query resources by type, source location, etc
    metadata about all resources (path, type, source location, description, etc)
    provides resources and/or metadata from inventory to the rest of the app
    NB resources are not saved in settings 
Tier-based architecture 
    no longer primary in structuring resources
    ackknowledges truths about the way resources are made available and how they are to be used
    1 installation
        a provided with the app, read-only
        b a user (developer) may build their own installations but still need to use resources in the LTS or dev Snapshot installations

    2 machine - provided by admin for all users, read-only
    3 user - provided by user, read-write

Special cases in resource deployment
    * user may need to designate locations where 3rd-party libraries are installed
      - cloned git respositories
      - downloaded libraries 
    * such libraries may be installed for ONLY personal use or may be needed in the machine tier for all users
    * Developers working on new libraries will need to add their own custom locations for testing

COnclusions on User Added Locations
    * may be added at any tier in which case they have the access given their folders by the user .. the app will not enforce access controls on user-added locations
    * user-added locations are persisted in settings



Gaps/Clarifications Needed:
Environment Variable Expansion ⚠️

Plan doesn't explicitly mention env var expansion in ResourcePaths

ANSWER: the const s_defaultSearchPaths has now been modified to include env vars in the default paths where applicable
Only the env vars already used need to be supported (e.g., HOME, USERPROFILE, APPDATA, the XDG_* family) as that info is taken from the opendscad documentation

Editable Resources GUI ⚠️

Plan mentions "Update preferences tabs" but doesn't detail:
Which resources are editable (color-schemes, templates)?

ANSEWER: yes, as mentioned above .. these are the only two that we provide an editing GUI for
there is another project that has already created preference panes for color schemes that will be ported into OpenSCAD. Also . template editor from this app will be moved over as well.

How to display resource contents (trees/lists)?

templates are displayed as lists under branches of a tree
  as you will see in the GUI app
  level one - tiers
  level two - the Location FOlders - see shortend names - see ResourceLocation::displayName
  level three - the individual templates as list items

color schemes have two different preference panels in preferences

Connection between ResourceRegistry and resource editor
This is likely Phase 5 refinement

ANSWER: yes, this is definitely Phase 5 refinement 

Current ResourceLocation is minimal (path, displayName, description, enabled, exists, isWritable, hasResourceFolders)

you will see that there are two widget classes based on QTree and QList knowing that most res meta data will not have a tree structure
but for the few that need it (templates) we have a tree widget

BUT i want to improve both to use the Qt classes mentioned in 
  D:\repositories\cppsnippets\cppsnippets\doc\ResourceRefactoring-QDirListing-Design.md
and 
  D:\repositories\cppsnippets\cppsnippets\doc\ResourceRefactoring-ResourceMetadata-Design.md

oh .. and i added a drag and drop option for adding resources as explained in 
  D:\repositories\cppsnippets\cppsnippets\doc\NEWRESOURCES_CONTAINER_INTEGRATION.md
and now that i think about it .. dragging in a folder would be a good way to add a custom location .. we would just need to be able to mark which tier it should be in


Plan mentions validateLocation(path) and scanForTypes(path) -
OBSOLETE - validation is never needed 
  the basic paths are hardcoded
  user-added paths are taken as they are
  discovery will be done at start up for all paths

work items:

1 resourcePath methods will have to be reworked or replaced to match new intentions

2 resourceLocationManager will be repurposed as ResourceRegistry

