// Source: https://github.com/microsoft/vscode/blob/main/src/vs/workbench/api/browser/mainThreadEditor.ts
/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *  Licensed under the MIT License. See License.txt in the project root for license information.
 *--------------------------------------------------------------------------------------------*/

import { Emitter, Event } from '../../../base/common/event.js';
import { DisposableStore } from '../../../base/common/lifecycle.js';
import { ICodeEditor } from '../../../editor/browser/editorBrowser.js';
import { RenderLineNumbersType, TextEditorCursorStyle, cursorStyleToString, EditorOption } from '../../../editor/common/config/editorOptions.js';
import { IRange, Range } from '../../../editor/common/core/range.js';
import { ISelection, Selection } from '../../../editor/common/core/selection.js';
import { IDecorationOptions, ScrollType } from '../../../editor/common/editorCommon.js';
import { ITextModel, ITextModelUpdateOptions } from '../../../editor/common/model.js';
import { ISingleEditOperation } from '../../../editor/common/core/editOperation.js';
import { IModelService } from '../../../editor/common/services/model.js';
import { SnippetController2 } from '../../../editor/contrib/snippet/browser/snippetController2.js';
// ...existing code...
