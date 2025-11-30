# cppsnippets
making a comprehesive snippets tool for use in C++ programs
Defining and Parsing Snippets

    Snippet Definition & Parsing:
        src/vs/workbench/contrib/snippets/browser/snippetsFile.ts
            The SnippetFile class handles parsing of snippet files provided by users/extensions.
            See _parseSnippet: parses snippet objects, determines prefix/body/description/scopes, integrates extension source info.
                Code for _parseSnippet

2. Snippet Storage (User, Workspace, Extensions)

    Managing and Loading Snippets:
        src/vs/workbench/contrib/snippets/browser/snippetsService.ts
            The SnippetsService class manages loading snippets from user files, workspace files, and extensions.
                Methods like _initUserSnippets, _initWorkspaceSnippets, _initExtensionSnippets handle sources.
                _addSnippetFile stores snippet files by extension or .json type.
            Filtering and sorting of available snippets is also performed here.

    Profile & File Storage:
        src/vs/workbench/services/userDataProfile/browser/snippetsResource.ts
            SnippetsResource class is used to read/write user snippets profile data.
            Snippet file content is stored per user profile.

3. Writing (Creating/Editing) Snippets

    Creating Snippet Files:
        src/vs/workbench/contrib/snippets/browser/commands/configureSnippets.ts
            createSnippetFile prepares snippet JSON files interactively for users.
            ConfigureSnippetsAction is the editor command to access snippet configuration UI.

4. Using Snippets in the Editor (Insertion, Navigation, Completion)

    Insertion in the Editor:
        src/vs/editor/contrib/snippet/browser/snippetController2.ts
            SnippetController2 is integrated into the editor as a contribution that manages snippet session lifecycles, insertion, navigation.
                Methods: insert, apply, finish, cancel, prev, next.
            Registers editor commands for keybindings (Tab to jump next, etc.).
        src/vs/editor/contrib/snippet/browser/snippetSession.ts
            SnippetSession manages the current snippet instance in the editor (tabstops, edits, session state).

    Completion & Suggestion:
        src/vs/workbench/contrib/snippets/browser/snippetCompletionProvider.ts
            Provides snippet completions in the suggestion widget; defines SnippetCompletion and SnippetCompletionProvider.

    Commands to Insert Snippets:
        src/vs/workbench/contrib/snippets/browser/commands/insertSnippet.ts
            InsertSnippetAction is the editor action for invoking snippet insertion.

5. Tracking and Using Snippets/Telemetry

    In snippetCompletionProvider.ts, the command markSnippetAsUsed is registered, and snippets are marked as used in the service. This is for usage tracking and telemetry.

Summary Table
Purpose	Main File(s)	Classes/Functions
Define/Parse	snippetsFile.ts	SnippetFile, _parseSnippet
Store/Manage	snippetsService.ts, snippetsResource.ts	SnippetsService, SnippetsResource
Write	configureSnippets.ts	ConfigureSnippetsAction, createSnippetFile
Use/Insert	snippetController2.ts, snippetSession.ts, insertSnippet.ts, snippetCompletionProvider.ts	SnippetController2, SnippetSession, InsertSnippetAction, SnippetCompletionProvider

You can browse, create, and use snippets via the UI and commands, while the above files handle the logic for storage, definition, UI, and editor operations. Let me know if you need further details or specific code!
