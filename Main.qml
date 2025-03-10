/*
 * Copyright 2025 Libro LMS
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window{
    id: barcodeMainWindow
    visible: true
    width: 640
    height: 480

    Loader{
        id: mainLoader
        anchors.fill: parent
        source: "BarcodeWriter.qml"
    }
}
