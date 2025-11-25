import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts


Item{
    id:root
    required property Window window
    property NotificationQueue manager

    // // must to register NotificationQueue & NotificationData
    Connections {
            target: manager
            function onDoTask(task) {
                // 弹出通知
                popup.data = task
                popup.level = task.level
                popup.content = task.content
                popup.needRet = task.needRet
                startpop.start()
            }
        }

    Connections {
            target: manager
            function onDoQuickTask(task) {
                // 弹出通知
                popQuick.data = task
                popQuick.level = task.level
                popQuick.content = task.content
                popQuick.needRet = task.needRet
                startpopquick.start()
            }
        }

// 严重等级  / 内容  / 反馈  /
    Popup {

        id: popup
        property Window window : root.window
        property NotificationData data

        // NotificationData
        // 0 info / 1 warning  / 2 err
        property int level: 0
        property string content: "Hello Popup!"
        property bool needRet: false

         // property Font font ： width / (3 * 1.5)  // 总高度 ≈ 行数 × (字体大小 × 行高系数1.5)  font.pixelSize

        // need bind window w/h
        width: window.width < 500 ? window.width - 10 : 500
        height: window.height < 100 ? window.height / 10 : 100

        x: (window.width - width) / 2
        y: -height
        opacity: 0.5
        modal: needRet
        focus: true
        closePolicy: needRet ? Popup.NoAutoClose : Popup.CloseOnEscape | Popup.CloseOnPressOutside
        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0

        // if you need see 2D ,please comment script
        ScriptAction {
            id: startpop
            script: {
                popup.open()
                if (popup.needRet)
                    timer.interval = 5000
                timer.start()
            }
        }

        ScriptAction {
            id: closepop
            script: {
                timer.stop()
                popup.close()
                // cancel & timeover & default & outside : return 0
                data.finishedTask(0)
                // run return result to emit signals
            }
        }

        ScriptAction {
            id: okclosepop
            script: {
                timer.stop()
                popup.close()
                // ok return 1
                data.finishedTask(1)
                // run return result to emit signals
            }
        }

        enter: Transition {
            NumberAnimation {
                target: popup
                property: "y"
                to: 10
                duration: 300
                easing.type: Easing.OutQuad
            }
        }
        exit: Transition {
            NumberAnimation {
                target: popup
                property: "y"
                to: -height
                duration: 250
                easing.type: Easing.InQuad
            }
        }

        Timer {
            id: timer
            interval: 2000
        }

        Connections {
            target: timer
            function onTriggered() {
                closepop.start()
            }
        }

        background: Rectangle {
            color: popup.level === 0 ? "green" : popup.level === 1 ? "yellow" : popup.level === 2 ? "red" : "gray" // : popup.level === 4 ? "blue"
            /*(function() {
                switch (colorMode) {
                    case "success": return "green"
                    case "warning": return "yellow"
                    case "error":   return "red"
                    case "info":    return "blue"
                    default:        return "gray"
                }
            })()*/
            radius: popup.width > popup.height ? (popup.height / 5) : (popup.width / 5)
            border.color: "#6ec1ee"
        }

        ColumnLayout {
            anchors.fill: parent

            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 6
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: TextEdit.Wrap // 允许换行
                text: popup.content

                /*TextArea {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: 5
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignTop //Text.AlignVCenter
                    wrapMode: TextEdit.Wrap // 允许换行
                    readOnly: true
                    text: popup.content
                }*/
            }

            RowLayout {
                visible: popup.needRet

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 4
                spacing: 0 // 按钮之间间距

                // --- OK 按钮 ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.width: okarea.containsMouse ? 2 : 0
                    border.color: "blue"
                    radius: 6

                    Text {
                        anchors.centerIn: parent
                        text: "OK"
                        font.bold: true
                    }

                    MouseArea {
                        id: okarea
                        anchors.fill: parent
                        hoverEnabled: true
                        Connections {
                            target: okarea
                            function onClicked() {
                                // emit ok signals
                                okclosepop.start()
                            }
                        }
                    }
                }

                // --- Cancel 按钮 ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.width: cancelarea.containsMouse ? 2 : 0
                    border.color: "gray"
                    radius: 6

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        font.bold: true
                    }

                    MouseArea {
                        id: cancelarea
                        anchors.fill: parent
                        hoverEnabled: true
                        Connections {
                            target: cancelarea
                            function onClicked() {
                                // emit cancel signals
                                closepop.start()
                            }
                        }
                    }
                }
            }
        }

        MouseArea {
            id: mousearea
            visible: !popup.needRet
            anchors.fill: parent
            // onClicked:
            Connections {
                target: mousearea
                function onClicked(){ closepop.start()}
            }
        }
    }

    Popup {

        id: popQuick
        property Window window : root.window
        property NotificationData data

        // NotificationData
        // 0 info / 1 warning  / 2 err
        property int level: 0
        property string content: "Hello Popup!"
        property bool needRet: true

        // need bind window w/h
        width: window.width < 500 ? window.width - 10 : 500
        height: window.height / 10

        x: (window.width - width) / 2
        y: (window.height - height) / 2
        opacity: 0
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose

        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0

        // if you need see 2D ,please comment script
        ScriptAction {
            id: startpopquick
            script: {
                popQuick.open()
                pqtimer.start()
            }
        }

        ScriptAction {
            id: closepopquick
            script: {
                pqtimer.stop()
                popQuick.close()
                // cancel & timeover & default & outside : return 0
                data.finishedTask(0)
                // run return result or emit signals
            }
        }

        ScriptAction {
            id: okclosepopquick
            script: {
                pqtimer.stop()
                popQuick.close()
                // ok return 1
                data.finishedTask(1)
                // run return result or emit signals
            }
        }

        // horizontalPadding: 0
        // verticalPadding: 0
        enter: Transition {
            NumberAnimation {
                target: popQuick
                property: "opacity"
                to: 0.8
                duration: 300
                easing.type: Easing.OutQuad
            }
        }
        exit: Transition {
            NumberAnimation {
                target: popQuick
                property: "opacity"
                to: 0
                duration: 250
                easing.type: Easing.InQuad
            }
        }

        Timer {
            id: pqtimer
            interval: 5000
        }

        Connections {
            target: pqtimer
            function onTriggered() {
                closepopquick.start()
            }
        }

        background: Rectangle {
            id: popquickback
            anchors.fill: parent
            color: popQuick.level === 0 ? "green" : popQuick.level === 1 ? "yellow" : popQuick.level === 2 ? "red" : "gray" // : popQuick.level === 4 ? "blue"


            /*(function() {
                switch (colorMode) {
                    case "success": return "green"
                    case "warning": return "yellow"
                    case "error":   return "red"
                    case "info":    return "blue"
                    default:        return "gray"
                }
            })()*/
            radius: popQuick.width > popQuick.height ? (popQuick.height
                                                        / 5) : (popQuick.width / 5)
            border.color: "#6ec1ee"
            clip: true
        }

        ColumnLayout {
            anchors.fill: parent

            Text {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 6
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: TextEdit.Wrap // 允许换行
                text: popQuick.content

                // TextArea {
                //     Layout.fillWidth: true
                //     Layout.fillHeight: true
                //     Layout.preferredHeight: 5
                //     horizontalAlignment: Text.AlignHCenter
                //     verticalAlignment: Text.AlignTop //Text.AlignVCenter
                //     wrapMode: TextEdit.Wrap // 允许换行
                //     readOnly: true
                //     text: popQuick.content
                // }
            }

            RowLayout {

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: 4
                spacing: 0 // 按钮之间间距

                // --- OK 按钮 ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.width: okarea.containsMouse ? 2 : 0
                    border.color: "blue"
                    radius: 6

                    Text {
                        anchors.centerIn: parent
                        text: "OK"
                        font.bold: true
                    }

                    MouseArea {
                        id: popquickOk
                        anchors.fill: parent
                        hoverEnabled: true
                        Connections {
                            target: popquickOk
                            function onClicked() {
                                // emit ok signals
                                okclosepopquick.start()
                            }
                        }
                    }
                }

                // --- Cancel 按钮 ---
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.width: cancelarea.containsMouse ? 2 : 0
                    border.color: "gray"
                    radius: 6

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        font.bold: true
                    }

                    MouseArea {
                        id: popquickCancel
                        anchors.fill: parent
                        hoverEnabled: true
                        Connections {
                            target: popquickCancel
                            function onClicked() {
                                // emit cancel signals
                                closepopquick.start()
                            }
                        }
                    }
                }
            }
        }
    }

}
