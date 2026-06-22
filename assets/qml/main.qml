import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 1440
    height: 900
    minimumWidth: 1024
    minimumHeight: 600
    visible: true
    visibility: Window.Maximized
    title: "CYBER SENTINEL X — AISOS // 2077 COMMAND NODE"
    color: "#020408"

    readonly property real chromeHeight: topBar.height + bottomBar.height + 20
    readonly property real mainAreaHeight: Math.max(0, height - chromeHeight)

    Shortcut {
        sequence: "F11"
        onActivated: root.visibility = root.visibility === Window.FullScreen
                       ? Window.Maximized : Window.FullScreen
    }

    readonly property color neonCyan: "#00E5FF"
    readonly property color neonGreen: "#00E676"
    readonly property color neonRed: "#FF1744"
    readonly property color neonAmber: "#FF6D00"
    readonly property color neonMagenta: "#E040FB"
    readonly property color voidBg: "#050A12"
    readonly property color panelBg: "#0A101C"
    readonly property var threatColors: ["#00E676", "#FFEB3B", "#FF6D00", "#FF1744", "#D500F9"]

    function threatColor(level) {
        return threatColors[Math.min(Math.max(level, 0), threatColors.length - 1)]
    }

    function identityColor(status) {
        if (status.indexOf("AUTHORIZED") >= 0) return neonGreen
        if (status.indexOf("UNAUTHORIZED") >= 0 || status.indexOf("FOE") >= 0) return neonRed
        if (status.indexOf("ANALYZING") >= 0 || status.indexOf("UNKNOWN") >= 0) return neonAmber
        return neonCyan
    }

    readonly property var sectorLabels: ["N", "NE", "E", "SE", "S", "SW", "W", "NW"]
    readonly property var sectorColors: [neonCyan, neonMagenta, neonGreen, neonAmber, neonCyan, neonMagenta, neonGreen, neonAmber]

    function sectorHits(index) {
        if (!dashboard.sectorActivity || dashboard.sectorActivity.length <= index) return 0
        return dashboard.sectorActivity[index]
    }

    function lockColor() {
        if (dashboard.lockPhase === "LOCKED") return neonGreen
        if (dashboard.lockPhase === "HOLDING") return neonAmber
        if (dashboard.lockPhase === "ACQUIRING") return neonCyan
        return "#546E7A"
    }

    // ── Animated background grid + floating particles (2077) ──
    Item {
        anchors.fill: parent
        z: -2
        Repeater {
            model: 24
            Rectangle {
                x: index * (root.width / 24)
                width: 1
                height: parent.height
                color: neonCyan
                opacity: 0.04
            }
        }
        Repeater {
            model: 16
            Rectangle {
                y: index * (root.height / 16)
                width: parent.width
                height: 1
                color: neonCyan
                opacity: 0.04
            }
        }
    }

    // ── Scanline sweep ──
    Rectangle {
        id: scanBeam
        width: parent.width
        height: 3
        color: neonCyan
        opacity: 0.12
        y: 0
        z: 50
        SequentialAnimation on y {
            loops: Animation.Infinite
            NumberAnimation { from: -10; to: root.height + 10; duration: 3200; easing.type: Easing.InOutQuad }
            PauseAnimation { duration: 400 }
        }
    }

    // ── TOP COMMAND BAR ──
    Rectangle {
        id: topBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 58
        color: "#060C16"
        border.color: Qt.rgba(0, 0.9, 1, 0.25)

        RowLayout {
            anchors.fill: parent
            anchors.margins: 14
            spacing: 18

            Text {
                text: "◈ CYBER SENTINEL X"
                color: neonCyan
                font.pixelSize: 20
                font.bold: true
                font.family: "Consolas"
            }
            Text {
                text: "NEO-TKY // 2077"
                color: neonMagenta
                font.pixelSize: 10
                font.family: "Consolas"
                opacity: 0.85
            }
            Text {
                text: "AISOS v1.0"
                color: Qt.rgba(0.7, 0.9, 1, 0.6)
                font.pixelSize: 11
                font.family: "Consolas"
            }

            Rectangle { Layout.fillWidth: true; color: "transparent" }

            Row {
                spacing: 24
                Repeater {
                    model: ["OPTICS", "NEURAL", "BIOMETRIC", "PERIMETER"]
                    Text {
                        text: modelData + ": " + (index === 0 ? "ONLINE" : index === 1 ? "ACTIVE" : index === 2 ? dashboard.lockPhase : dashboard.radarBlipCount + " BLIP")
                        color: index === 2 ? lockColor() : neonCyan
                        opacity: 0.85
                        font.pixelSize: 11
                        font.family: "Consolas"
                    }
                }
            }

            Text {
                text: "DEFCON " + dashboard.defconLevel
                color: threatColor(dashboard.threatLevel)
                font.pixelSize: 18
                font.bold: true
                font.family: "Consolas"
            }
            Text {
                text: "AUTH DB " + dashboard.authorizedCount
                color: neonGreen
                font.pixelSize: 12
                font.family: "Consolas"
            }
            Button {
                text: "◈ ENROLL"
                flat: true
                font.family: "Consolas"
                contentItem: Text {
                    text: parent.text
                    color: neonCyan
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                }
                onClicked: enrollDrawer.open()
            }
            Text {
                text: "T+" + dashboard.uptime
                color: "#90A4AE"
                font.pixelSize: 12
                font.family: "Consolas"
            }
        }
    }

    // ── MAIN LAYOUT ──
    RowLayout {
        id: mainLayout
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: bottomBar.top
        anchors.margins: 10
        spacing: 10
        clip: true

        // LEFT COLUMN — optical feed + neural pipeline
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.52
            spacing: 10

            // OPTICAL SENSOR FEED
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: voidBg
                radius: 2
                border.color: neonCyan
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "▸ OPTICAL SENSOR ARRAY"
                            color: neonCyan
                            font.pixelSize: 12
                            font.bold: true
                            font.family: "Consolas"
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: dashboard.cameraStatus
                            color: "#B0BEC5"
                            font.pixelSize: 10
                            font.family: "Consolas"
                        }
                    }

                    // Viewport with corner brackets
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Image {
                            id: liveFrameView
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit
                            cache: false
                            source: dashboard.frameId > 0
                                    ? "image://frames/live?" + dashboard.frameId
                                    : ""
                        }

                        // Corner HUD brackets
                        Repeater {
                            model: 4
                            Item {
                                readonly property int sz: 28
                                readonly property int t: 2
                                x: index === 0 || index === 2 ? 4 : parent.width - sz - 4
                                y: index === 0 || index === 1 ? 4 : parent.height - sz - 4
                                width: sz; height: sz
                                Rectangle { width: sz; height: t; color: neonCyan; opacity: 0.9 }
                                Rectangle { width: t; height: sz; color: neonCyan; opacity: 0.9 }
                                Rectangle {
                                    visible: index >= 1
                                    anchors.right: parent.right
                                    width: sz; height: t; color: neonCyan; opacity: 0.9
                                }
                                Rectangle {
                                    visible: index >= 2
                                    anchors.bottom: parent.bottom
                                    width: t; height: sz; color: neonCyan; opacity: 0.9
                                }
                                Rectangle {
                                    visible: index === 3
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    width: sz; height: t; color: neonCyan; opacity: 0.9
                                }
                            }
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: Math.min(parent.width, parent.height) * 0.72
                            height: width
                            radius: width / 2
                            color: "transparent"
                            border.color: lockColor()
                            border.width: 2
                            visible: dashboard.lockStrength > 0
                            opacity: 0.75
                        }

                        // Face lock indicator
                        Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 8
                            width: faceLabel.width + 24
                            height: 30
                            radius: 2
                            color: Qt.rgba(0, 0, 0, 0.72)
                            border.color: lockColor()
                            visible: dashboard.bootComplete
                            Row {
                                anchors.centerIn: parent
                                spacing: 8
                                Rectangle {
                                    width: 8; height: 8; radius: 4
                                    color: lockColor()
                                    SequentialAnimation on opacity {
                                        loops: Animation.Infinite
                                        NumberAnimation { from: 0.4; to: 1.0; duration: dashboard.lockPhase === "LOCKED" ? 800 : 400 }
                                        NumberAnimation { from: 1.0; to: 0.4; duration: dashboard.lockPhase === "LOCKED" ? 800 : 400 }
                                    }
                                }
                                Text {
                                    id: faceLabel
                                    text: dashboard.faceStatus
                                    color: lockColor()
                                    font.pixelSize: 11
                                    font.family: "Consolas"
                                }
                                Text {
                                    text: dashboard.lockStrength + "%"
                                    color: "#78909C"
                                    font.pixelSize: 9
                                    font.family: "Consolas"
                                    visible: dashboard.lockStrength > 0
                                }
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: voidBg
                            visible: dashboard.frameId === 0
                            Text {
                                anchors.centerIn: parent
                                text: "INITIALIZING OPTICAL LINK..."
                                color: "#546E7A"
                                font.family: "Consolas"
                                font.pixelSize: 14
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "FPS " + dashboard.fps.toFixed(1); color: "#B0BEC5"; font.family: "Consolas"; font.pixelSize: 11 }
                        Text { text: "LAT " + dashboard.frameTimeMs.toFixed(1) + "ms"; color: "#B0BEC5"; font.family: "Consolas"; font.pixelSize: 11 }
                        Text { text: "FACES " + dashboard.faceCount; color: dashboard.faceCount > 0 ? neonGreen : "#B0BEC5"; font.family: "Consolas"; font.pixelSize: 11 }
                        Text { text: "TRACKS " + dashboard.trackCount; color: "#B0BEC5"; font.family: "Consolas"; font.pixelSize: 11 }
                        Item { Layout.fillWidth: true }
                        Text { text: "NEURAL " + dashboard.neuralLoad.toFixed(1) + "ms"; color: neonCyan; font.family: "Consolas"; font.pixelSize: 11 }
                    }
                }
            }

            // NEURAL PIPELINE STATUS
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(110, mainAreaHeight * 0.14)
                Layout.maximumHeight: 110
                color: panelBg
                border.color: Qt.rgba(0, 0.9, 1, 0.3)

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    columns: 4
                    rowSpacing: 8
                    columnSpacing: 16

                    Repeater {
                        model: [
                            { label: "CAMERA", val: "ONLINE", col: neonGreen },
                            { label: "MOTION", val: "MOG2", col: neonCyan },
                            { label: "TRACKER", val: dashboard.trackCount + " OBJ", col: neonCyan },
                            { label: "YuNet DNN", val: dashboard.recognitionEngine, col: neonGreen },
                            { label: "BEHAVIOR", val: "LEARNING", col: neonCyan },
                            { label: "THREAT AI", val: dashboard.threatScore.toFixed(0), col: threatColor(dashboard.threatLevel) },
                            { label: "RADAR", val: "SWEEP", col: neonCyan },
                            { label: "VOICE", val: "ARMED", col: neonAmber }
                        ]
                        Column {
                            Layout.fillWidth: true
                            Text { text: modelData.label; color: "#607D8B"; font.pixelSize: 9; font.family: "Consolas" }
                            Text {
                                text: modelData.val
                                color: modelData.col
                                font.pixelSize: 11
                                font.bold: true
                                font.family: "Consolas"
                                elide: Text.ElideRight
                                width: 140
                            }
                        }
                    }
                }
            }
        }

        // RIGHT COLUMN — threat, radar, targets, events (scrollable on small screens)
        ScrollView {
            id: rightScroll
            Layout.preferredWidth: parent.width * 0.46
            Layout.fillHeight: true
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AsNeeded
            contentWidth: availableWidth

            ColumnLayout {
                id: rightColumn
                width: rightScroll.availableWidth
                spacing: 6

            // THREAT MATRIX + LIVE CHART
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(128, mainAreaHeight * 0.16)
                color: panelBg
                border.color: threatColor(dashboard.threatLevel)
                border.width: 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 4
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "THREAT MATRIX // 72-FRAME TELEMETRY"; color: neonCyan; font.pixelSize: 10; font.bold: true; font.family: "Consolas" }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: dashboard.threatScore.toFixed(1)
                            color: threatColor(dashboard.threatLevel)
                            font.pixelSize: 24
                            font.bold: true
                            font.family: "Consolas"
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        Rectangle {
                            anchors.fill: parent
                            color: "#060C14"
                            border.color: Qt.rgba(0, 0.9, 1, 0.15)
                        }
                        Row {
                            anchors.fill: parent
                            anchors.margins: 3
                            spacing: 1
                            Repeater {
                                model: dashboard.threatHistory
                                Rectangle {
                                    width: Math.max(2, (parent.width - parent.spacing * Math.max(1, dashboard.threatHistory.length)) / Math.max(1, dashboard.threatHistory.length))
                                    height: parent.height * Math.min(1, modelData / 100)
                                    anchors.bottom: parent.bottom
                                    color: threatColor(dashboard.threatLevel)
                                    opacity: 0.65
                                }
                            }
                        }
                    }
                    Text {
                        text: dashboard.threatReason.length > 0 ? dashboard.threatReason : "PERIMETER NOMINAL — NO HOSTILE SIGNATURES"
                        color: "#CFD8DC"
                        font.pixelSize: 9
                        font.family: "Consolas"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            // NEURAL LOAD — live sparkline + gauge
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(80, mainAreaHeight * 0.1)
                color: "#080E18"
                border.color: Qt.rgba(0.88, 0.25, 0.98, 0.35)

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "NEURAL CORE LOAD (ms)"; color: neonMagenta; font.pixelSize: 9; font.bold: true; font.family: "Consolas" }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: dashboard.neuralLoad.toFixed(1) + " ms"
                            color: neonMagenta
                            font.pixelSize: 14
                            font.bold: true
                            font.family: "Consolas"
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 34
                        Rectangle {
                            anchors.fill: parent
                            color: "#060C14"
                            border.color: Qt.rgba(0.88, 0.25, 0.98, 0.2)
                        }
                        Row {
                            anchors.fill: parent
                            anchors.margins: 3
                            spacing: 1
                            Repeater {
                                model: dashboard.neuralHistory
                                Rectangle {
                                    width: Math.max(2, (parent.width - parent.spacing * Math.max(1, dashboard.neuralHistory.length)) / Math.max(1, dashboard.neuralHistory.length))
                                    height: parent.height * Math.min(1, modelData / 100)
                                    anchors.bottom: parent.bottom
                                    color: neonMagenta
                                    opacity: 0.75
                                }
                            }
                        }
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 6
                        color: "#060C14"
                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            width: parent.width * Math.min(1, dashboard.neuralLoad / 100)
                            color: neonMagenta
                            opacity: 0.6
                        }
                    }
                }
            }

            // IDENTITY DOSSIER — live target intel
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(108, mainAreaHeight * 0.13)
                color: "#080E18"
                border.color: dashboard.identityStatus.indexOf("AUTHORIZED") >= 0 ? neonGreen
                              : dashboard.identityStatus.indexOf("UNAUTHORIZED") >= 0 ? neonRed
                              : Qt.rgba(0, 0.9, 1, 0.35)
                border.width: dashboard.faceCount > 0 ? 2 : 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "◈ IDENTITY DOSSIER"; color: neonCyan; font.pixelSize: 11; font.bold: true; font.family: "Consolas" }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: dashboard.lockPhase + " // " + dashboard.identityStatus
                            color: identityColor(dashboard.identityStatus)
                            font.pixelSize: 10
                            font.bold: true
                            font.family: "Consolas"
                        }
                    }
                    Text {
                        text: dashboard.identityDossier
                        color: "#E0F7FA"
                        font.pixelSize: 10
                        font.family: "Consolas"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    Text {
                        text: "MATCH CONFIDENCE: " + dashboard.identityConfidence + "%  |  VAULT: " + dashboard.authorizedCount + " profiles"
                        color: "#78909C"
                        font.pixelSize: 9
                        font.family: "Consolas"
                    }
                }
            }

            // BIOMETRIC VAULT + TARGET MANIFEST
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(88, mainAreaHeight * 0.11)
                color: panelBg
                border.color: Qt.rgba(0, 1, 0.46, 0.4)

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    Text { text: "BIOMETRIC SCAN — YuNet + SFace"; color: neonGreen; font.pixelSize: 11; font.bold: true; font.family: "Consolas" }
                    Text {
                        text: dashboard.faceCount > 0
                              ? dashboard.targetManifest
                              : "◌ SCANNING FOR HUMAN BIO-SIGNATURES...\n   Add photos via ENROLL or drop in assets/faces/authorized/"
                        color: dashboard.faceCount > 0 ? "#E0F7FA" : "#78909C"
                        font.pixelSize: 10
                        font.family: "Consolas"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }

            // PERIMETER RADAR — tactical scope + sector analytics
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(180, mainAreaHeight * 0.34)
                color: voidBg
                border.color: neonCyan
                border.width: 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 6
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "PERIMETER RADAR // 8-SECTOR MESH"; color: neonCyan; font.pixelSize: 11; font.bold: true; font.family: "Consolas" }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: dashboard.radarBlipCount + " CONTACTS"
                            color: dashboard.radarBlipCount > 0 ? neonAmber : "#546E7A"
                            font.pixelSize: 9
                            font.family: "Consolas"
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 120

                        Image {
                            id: radarFrameView
                            anchors.fill: parent
                            fillMode: Image.PreserveAspectFit
                            cache: false
                            source: dashboard.frameId > 0
                                    ? "image://radar/scope?" + dashboard.frameId
                                    : ""
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 72
                        columns: 4
                        rowSpacing: 3
                        columnSpacing: 3

                        Repeater {
                            model: 8
                            Rectangle {
                                property int sectorIndex: index
                                Layout.fillWidth: true
                                Layout.preferredHeight: 34
                                color: "#060C14"
                                border.color: sectorColors[sectorIndex]

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 2
                                    RowLayout {
                                        Layout.fillWidth: true
                                        Text {
                                            text: sectorLabels[sectorIndex]
                                            color: sectorColors[sectorIndex]
                                            font.pixelSize: 9
                                            font.bold: true
                                            font.family: "Consolas"
                                        }
                                        Item { Layout.fillWidth: true }
                                        Text {
                                            text: sectorHits(sectorIndex)
                                            color: "#B0BEC5"
                                            font.pixelSize: 8
                                            font.family: "Consolas"
                                        }
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        Row {
                                            anchors.bottom: parent.bottom
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            spacing: 2
                                            Repeater {
                                                model: Math.min(6, Math.max(1, sectorHits(sectorIndex)))
                                                Rectangle {
                                                    width: 5
                                                    height: 6 + index * 4
                                                    color: sectorColors[sectorIndex]
                                                    opacity: 0.55 + index * 0.08
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // EVENT LOG
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(64, mainAreaHeight * 0.08)
                color: "#060810"
                border.color: Qt.rgba(0, 0.9, 1, 0.2)

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    Text { text: "EVENT STREAM"; color: "#607D8B"; font.pixelSize: 9; font.family: "Consolas" }
                    Text {
                        text: "[" + dashboard.uptime + "] " + dashboard.cameraStatus + "\n"
                              + "[" + dashboard.uptime + "] " + dashboard.faceStatus + "\n"
                              + "[" + dashboard.uptime + "] THREAT=" + dashboard.threatScore.toFixed(1) + " NEURAL=" + dashboard.neuralLoad.toFixed(1) + "ms"
                        color: neonCyan
                        font.pixelSize: 9
                        font.family: "Consolas"
                        opacity: 0.8
                        Layout.fillWidth: true
                    }
                }
            }
            }  // rightColumn
        }  // rightScroll
    }

    // ── BOTTOM TELEMETRY STRIP ──
    Rectangle {
        id: bottomBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 28
        color: "#060C16"
        border.color: Qt.rgba(0, 0.9, 1, 0.2)

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            Text { text: "CPU " + dashboard.cpuPercent.toFixed(0) + "%"; color: "#90A4AE"; font.family: "Consolas"; font.pixelSize: 10 }
            Text { text: "RAM " + dashboard.ramPercent.toFixed(0) + "%"; color: "#90A4AE"; font.family: "Consolas"; font.pixelSize: 10 }
            Text { text: "FRAME " + dashboard.frameId; color: "#90A4AE"; font.family: "Consolas"; font.pixelSize: 10 }
            Item { Layout.fillWidth: true }
            Text {
                text: "OBSERVE · ANALYZE · PREDICT · PROTECT"
                color: neonCyan
                opacity: 0.5
                font.family: "Consolas"
                font.pixelSize: 10
            }
        }
    }

    // ── FOE GLITCH OVERLAY (only confirmed FOE, not analyzing) ──
    Rectangle {
        anchors.fill: parent
        color: neonRed
        opacity: dashboard.identityStatus === "FOE" ? 0.07 : 0
        z: 40
        visible: opacity > 0
        SequentialAnimation on opacity {
            running: dashboard.identityStatus === "FOE"
            loops: Animation.Infinite
            NumberAnimation { from: 0.03; to: 0.11; duration: 180 }
            NumberAnimation { from: 0.11; to: 0.03; duration: 180 }
        }
    }

    // ── ENROLLMENT DRAWER ──
    Drawer {
        id: enrollDrawer
        edge: Qt.RightEdge
        width: 400
        height: parent.height
        modal: true
        background: Rectangle { color: "#0A101C"; border.color: neonCyan }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            Text {
                text: "BIOMETRIC VAULT — ENROLLMENT"
                color: neonCyan
                font.pixelSize: 14
                font.bold: true
                font.family: "Consolas"
            }

            Text {
                text: "Register authorized personnel with name, country of origin, and face photo."
                color: "#90A4AE"
                font.pixelSize: 10
                font.family: "Consolas"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Text { text: "FULL NAME"; color: "#607D8B"; font.pixelSize: 9; font.family: "Consolas" }
            TextField {
                id: enrollName
                Layout.fillWidth: true
                placeholderText: "e.g. John Smith"
                font.family: "Consolas"
                color: "#E0F7FA"
                background: Rectangle { color: "#060C16"; border.color: neonCyan; radius: 2 }
            }

            Text { text: "COUNTRY OF ORIGIN"; color: "#607D8B"; font.pixelSize: 9; font.family: "Consolas" }
            TextField {
                id: enrollCountry
                Layout.fillWidth: true
                placeholderText: "e.g. USA, Bangladesh, Japan"
                font.family: "Consolas"
                color: "#E0F7FA"
                background: Rectangle { color: "#060C16"; border.color: neonCyan; radius: 2 }
            }

            Text { text: "ROLE / CLEARANCE"; color: "#607D8B"; font.pixelSize: 9; font.family: "Consolas" }
            TextField {
                id: enrollRole
                Layout.fillWidth: true
                text: "operator"
                font.family: "Consolas"
                color: "#E0F7FA"
                background: Rectangle { color: "#060C16"; border.color: neonCyan; radius: 2 }
            }

            Text { text: "PHOTO FILE PATH"; color: "#607D8B"; font.pixelSize: 9; font.family: "Consolas" }
            RowLayout {
                Layout.fillWidth: true
                TextField {
                    id: enrollPath
                    Layout.fillWidth: true
                    placeholderText: "assets/faces/authorized/photo.jpg"
                    font.family: "Consolas"
                    font.pixelSize: 10
                    color: "#E0F7FA"
                    background: Rectangle { color: "#060C16"; border.color: neonCyan; radius: 2 }
                }
                Button {
                    text: "BROWSE"
                    font.family: "Consolas"
                    onClicked: faceFileDialog.open()
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Button {
                    Layout.fillWidth: true
                    text: "◈ ENROLL FROM FILE"
                    font.family: "Consolas"
                    onClicked: {
                        enrollment.enrollFromFile(enrollPath.text, enrollName.text, enrollCountry.text, enrollRole.text)
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: "◉ CAPTURE FROM CAMERA"
                    font.family: "Consolas"
                    onClicked: {
                        enrollment.enrollFromCamera(enrollName.text, enrollCountry.text, enrollRole.text)
                    }
                }
            }

            Button {
                Layout.fillWidth: true
                text: "↻ IMPORT assets/faces/authorized FOLDER"
                font.family: "Consolas"
                onClicked: enrollment.importFolder("assets/faces/authorized")
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: Qt.rgba(0, 0.9, 1, 0.2) }

            Text {
                text: "REGISTERED PROFILES:\n" + enrollment.databaseSummary()
                color: neonGreen
                font.pixelSize: 9
                font.family: "Consolas"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Text {
                text: enrollment.statusMessage
                color: enrollment.statusMessage.indexOf("Enrolled") >= 0 ? neonGreen : neonAmber
                font.pixelSize: 10
                font.family: "Consolas"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
    }

    FileDialog {
        id: faceFileDialog
        title: "Select authorized face photo"
        nameFilters: ["Image files (*.jpg *.jpeg *.png *.bmp)"]
        onAccepted: {
            var path = selectedFile.toString()
            if (path.startsWith("file:///")) {
                path = path.substring(8)
            } else if (path.startsWith("file://")) {
                path = path.substring(7)
            }
            enrollPath.text = decodeURIComponent(path)
        }
    }

    // ── BOOT SEQUENCE OVERLAY ──
    Rectangle {
        id: bootOverlay
        anchors.fill: parent
        color: "#03050A"
        visible: !dashboard.bootComplete
        z: 100

        Column {
            anchors.centerIn: parent
            spacing: 16

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "CYBER SENTINEL X"
                color: neonCyan
                font.pixelSize: 36
                font.bold: true
                font.family: "Consolas"
            }

            Text {
                id: bootPhase
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#B0BEC5"
                font.pixelSize: 13
                font.family: "Consolas"
                property int phase: 0
                text: [
                    "INITIALIZING NEO-GRID POWER...",
                    "WIRING NEURAL CORE // AISOS...",
                    "HANDSHAKING OPTICAL SENSOR ARRAY...",
                    "LOADING BIOMETRIC VAULT (YuNet + SFace)...",
                    "ARMING PERIMETER MESH // 2077...",
                    "CALIBRATING THREAT TELEMETRY...",
                    "CYBER SENTINEL ONLINE — WELCOME TO 2077"
                ][Math.min(phase, 6)]

                Timer {
                    interval: 600
                    repeat: true
                    running: bootOverlay.visible
                    onTriggered: bootPhase.phase = Math.min(bootPhase.phase + 1, 6)
                }
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 320
                height: 4
                color: "#1A2332"
                Rectangle {
                    width: parent.width * (bootPhase.phase / 6)
                    height: parent.height
                    color: neonCyan
                    Behavior on width { NumberAnimation { duration: 300 } }
                }
            }
        }
    }
}
