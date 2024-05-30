// Copyright (c) 2017-2024 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

pragma Singleton

import QtQuick 2.4
import WebOS.Global 1.0
import WebOSCompositorBase 1.0
import WebOSCompositor 1.0
import WebOSCompositorExtended 1.0

Item {
    id: root
    objectName: "Settings"

    // NOTE: Define properties readonly as they're shared globally

    readonly property string appId: LS.appId
    readonly property alias local: localSettings.settings
    readonly property alias system: systemSettings.settings
    readonly property alias l10n: systemSettings.l10n
    readonly property alias currentLocale: systemSettings.currentLocale

    function subscribe(type, method, key, returnRaw, sessionId) {
        if (typeof returnRaw != "boolean")
            returnRaw = false;
        return systemSettings.subscribeOnDemand(type, method, key, returnRaw, sessionId);
    }

    function updateLocalSettings(obj) {
        console.info('updateLocalSettings:', JSON.stringify(obj));
        localSettings.signalReady(obj);
    }

    DefaultSettings {
        id: defaultSettingsData
    }

    LocalSettingsFiles {
        id: localSettingsFiles
    }

    LocalSettings {
        id: localSettings

        rootElement: root.appId
        defaultSettings: defaultSettingsData.settings
        files: localSettingsFiles.list

        Connections {
            target: compositor
            function onReloadConfig() {
                localSettings.reloadAllFiles();
                console.info("All settings are reloaded.");
            }
        }
    }

    DefaultSubscriptions {
        id: defaultSubscriptionsData
    }

    LunaServiceSubscriptions {
        id: extraSubscriptionsData
    }

    SystemSettings {
        id: systemSettings
        appId: root.appId
        defaultSubscriptions: defaultSubscriptionsData.list
        extraSubscriptions: extraSubscriptionsData.list
        l10nFileNameBase: "luna-surfacemanager"
        l10nDirName: WebOS.localizationDir + "/" + l10nFileNameBase
        l10nPluginImports: root.local.localization.imports
    }

    Component.onCompleted: {
        Utils.pmLog.info("PMLOG_START", {}, "");
        Utils.pmTrace.traceLog("Settings onCompleted");
        console.info("Constructed a singleton type:", root);
    }
}
