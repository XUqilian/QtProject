import QtQuick
import QtQuick.Layouts 2.15
import QtQuick.Controls 2.15
import localRS

Rectangle {
    id: settings
    anchors.fill: parent
    opacity: agc.sets?.item("ui.opacity")?.value
             ?? 1.0 //Gss?.item("ui.opacity")?.value ?? 1.0
    color: "black"

    RowLayout {
        anchors.fill: parent

        ListView {
            Layout.preferredWidth: 1
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: listGroup

            highlight: Rectangle {
                color: "lightsteelblue"
                radius: 3
            }
            focus: true
            clip: true

            model: vm?.serviceNames
            delegate: Button {
                width: parent.width
                text: modelData
                onClicked: listGroup.currentIndex = index
            }

            onCurrentIndexChanged: if (currentIndex >= 0)
                                       vm?.currentChanged(model[currentIndex])
        }

        ListView {
            Layout.preferredWidth: 4
            Layout.fillWidth: true
            Layout.fillHeight: true
            id: listArea

            model: vm?.currentModel

            delegate: SettingItemView {
                itemModel: model.display
            }
        }
    }
}
