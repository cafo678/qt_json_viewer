import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls.Basic

ApplicationWindow
{
    width: 640
    height: 480
    visible: true
    title: qsTr("File Viewer")
    id: root

    menuBar: MenuBar {
        anchors.left: parent.left
        anchors.right: parent.right

        Menu {
            title: "File"

            MenuItem {
                text: "Open"
                onTriggered: openDialog.open()
            }

            MenuSeparator {}

            MenuItem {
                text: "Quit"
                onTriggered: Qt.quit()
            }
        }
    }

    FileDialog {
        id: openDialog
        title: "Choose a file"
        nameFilters: ["JSON files (*.json)", "All files (*)"]

        onAccepted: {
            console.log("Selected:", selectedFile.toString())
            FileReader.readJsonFileAsync(selectedFile)//FileReader.readJsonFile(selectedFile)
        }

        onRejected: {
            console.log("Canceled")
        }
    }

    property int ticks: 0

    Timer {
        interval: 100
        running: true
        repeat: true
        onTriggered: root.ticks++
    }

    Column {
        id: topPanel
        spacing: 8
        anchors.margins: 12
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        z: 10

        Text {
            text: "UI heartbeat ticks: " + root.ticks
            font.pixelSize: 16
        }

        ProgressBar {
            indeterminate: true
            visible: FileReader.loading
            width: 200
        }

        Text {
            text: FileReader.statusText
        }
    }

    TreeView
    {
        id: tree
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topPanel.bottom
        anchors.bottom: parent.bottom
        model: FileReader.jsonModel

        delegate: TreeViewDelegate
        {
            indentation: 18

            contentItem: Row
            {
                spacing: 12

                Text
                {
                    width: 300
                    elide: Text.ElideRight
                    text: model.display
                }

                Text
                {
                    elide: Text.ElideRight
                    text: ""
                }
            }
        }
    }
}
