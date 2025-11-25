import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


    Button
    {
        property alias text: label.text
        property alias imageSource: image.source
        property alias iconAlignment: contentItem.iconAlign

        contentItem: Item{
            property Qt.Alignment iconAlign: Qt.AlignLeft

            Image{
                id: image

                anchors.left:   parent.iconAlign === Qt.AlignLeft ? parent.left : undefined
                anchors.right:  parent.iconAlign === Qt.AlignRight ? parent.right : undefined
                anchors.top:    parent.iconAlign === Qt.AlignTop ? parent.top : undefined
                anchors.bottom: parent.iconAlign === Qt.AlignBottom ? parent.bottom : undefined

                anchors.verticalCenter: (parent.iconAlign === Qt.AlignLeft || parent.iconAlign === Qt.AlignRight) ? parent.verticalCenter : undefined
                anchors.horizontalCenter: (parent.iconAlign === Qt.AlignTop || parent.iconAlign === Qt.AlignBottom) ? parent.horizontalCenter : undefined
            }

            Label{
                id: label

                anchors.Margin:1

                anchors.left:   parent.iconAlign === Qt.AlignLeft ? parent.right : undefined
                anchors.right:  parent.iconAlign === Qt.AlignRight ? parent.left : undefined
                anchors.top:    parent.iconAlign === Qt.AlignTop ? parent.bottom : undefined
                anchors.bottom: parent.iconAlign === Qt.AlignBottom ? parent.top : undefined

                anchors.verticalCenter: (parent.iconAlign === Qt.AlignLeft || parent.iconAlign === Qt.AlignRight) ? parent.verticalCenter : undefined
                anchors.horizontalCenter: (parent.iconAlign === Qt.AlignTop || parent.iconAlign === Qt.AlignBottom) ? parent.horizontalCenter : undefined
            }

        }
    }

