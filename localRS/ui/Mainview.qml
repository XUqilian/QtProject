import QtQuick
import QtQuick.VirtualKeyboard
import QtQuick.Layouts 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Dialogs

import localRS

Window {
    id: window
    width: 640
    height: 480
    visible: false
    title: qsTr("localRS")
    opacity: agc?.sets?.item("ui.opacity")?.value ?? 1.0 // Gss.Gopacity.value
    color: "black" // Gss.Gtheme.value    // color: rgba(0, 0, 0, 0)

    ColumnLayout {
        anchors.fill: parent

        ColumnLayout {
            height:window.height/10
            Layout.fillWidth: true

            MenuBar {
                Layout.fillWidth: true

                // background: Rectangle{color: "#000000"}
                Menu {
                    id: pagesMenu
                    title: "Pages"

                    Component.onCompleted: {
                            console.log("Initial pagelist:", vm.pagelist)
                        }

                    onAboutToShow: {
                        console.log("Menu about to show, pagelist:", vm.pagelist)
                    }

                    Repeater {
                        id: pageRepeater
                        model: vm.pagelist

                        MenuItem {
                            text: modelData
                            onClicked: {
                                vm.selectPage(modelData)
                            }
                        }
                    }

                    MenuSeparator {}

                    MenuItem {
                        id: addPage
                        text: "addPage"
                        onClicked: selectQmlFile.open()
                    }
                }

                // Item { Layout.fillWidth: true } // 挤占剩余内容，将剩下的放到最右侧

                // Action 作为数据+动作的集合体，可以赋予button/menuitem/等组件 而且自带状态和显示数据，再添加进入ActionGroup后还能主动与其它组员实现互斥
            }

            // 可以设置工具栏
        }

        StackView {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true

            Connections {
                target: vm
                function onCurrentPageChanged() {
                                          viewModel = vm.currentPage
                                          // stack.pop()
                                          // stack.push( { "item": viewModel.rootComponent().create(), "context": viewModel.rootContext() })
                                          // stack.replace( { "item": viewModel.rootComponent().create(), "context": viewModel.rootContext() })
                                      }
            }

        }

        ToolBar {
            Layout.fillWidth: true
            height: 20

            // background: Rectangle{color: Gss.Gtheme.value}
            RowLayout {
                width: parent.width

                Label {
                    Layout.alignment: Qt.AlignLeft
                    text: "Learning is like rowing upstream; not to advance is to drop back."
                }

                Label {
                    Layout.alignment: Qt.AlignRight
                    text: "by qilianxu"
                }
            }
        }
    }

    NotificationView{
        window:window
        manager:vm.notify
    }

    FileDialog {
        id: selectQmlFile
        title: "Select a QML file"
        // currentFolder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        nameFilters: ["QML files (*.qml)", "All files (*)"]
        onAccepted: {

            var filePath = selectQmlFile.file
            var fileNameWithExt = filePath.split("/").pop()        // "page.qml"
            var fileName = filePath.split("/").pop().split(".")[0]  // 取第一个点前的部分    // fileNameWithExt.split(".").slice(0, -1).join(".")  // "page"（兼容多点文件名）
            // vm.addPageFromFile(fileUrl.toString())
        }
    }
}
