import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Item {
    id:root
    property Session ssob

    DropArea{
        anchors.fill: parent
        // 文件拖放发送

        onDropped: {
                    // drag.urls 是文件 URL 列表（file:///path/to/file）
                for (let url of drag.urls) {
                    let decodedFileUrl = Qt.decodeUrl(url)  // 正确解码
                    console.log("path:", decodedFileUrl)
                    // fileModel.append({ path: decodedFileUrl })
                    ssob.send(decodedFileUrl)
                }
            }

            // 可选：进入拖拽区域
            onEntered: {
                drag.visualize()  // 可选：显示拖拽反馈
            }
            onExited: {

            }
    }

    RowLayout{
        // head
        Text{text: ssob.name + ssob.status}
        // msg View
        ListView{
            // model: SessionMsgListModel
        }
        // ctrl Menu
        ToolBar{
            // some ctrl command ...
            Button{
                anchors.right: true

                text: "Send"
                onClicked: if(inputArea.text.trim() === "") ssob.send(inputArea.text.trim());
            }
        }
        // enter msg
        TextArea{
            id:inputArea
            text:""
        }
    }
}
