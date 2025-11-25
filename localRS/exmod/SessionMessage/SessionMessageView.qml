import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtMultimedia

Item {
    id: root

    property SessionMessage msgData

    width: parent.width
    height: loader.item ? loader.item.height + 10 : 20 // Adjust height based on content

    // 主内容区域
    Loader {
        id: loader
        anchors.left: !msgData.isSender ? parent.left : undefined
        anchors.right: msgData.isSender ? parent.right : undefined
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        sourceComponent: {
            switch (msgData.type) {
            case "Text":
                return textContent
            case "File":
                return fileContent
            case "Image":
                return imageContent
            case "Audio":
                return audioContent
            case "Video":
                return videoContent
            default:
                return textContent
            }
        }
    }

    // --- 各种消息类型的 Component ---
    Component {
        id: textContent
        Rectangle {
            implicitWidth: Math.min(messageText.paintedWidth + 30,
                                    root.width * 0.7)
            implicitHeight: messageText.paintedHeight + 20
            color: msgData.isSender ? "#0B84FF" : "#E5E5EA"
            radius: 16
            border.color: msgData.isSender ? "#0A7BDE" : "#DDDDDD"
            border.width: 1

            Text {
                id: messageText
                anchors.centerIn: parent
                text: msgData.text
                color: msgData.isSender ? "white" : "black"
                font.pixelSize: 14
                wrapMode: Text.Wrap
                width: parent.width - 30 // 留边距
            }
        }
    }

    Component {
        id: fileContent
        Rectangle {
            implicitWidth: Math.min(btn.paintedWidth + 30, root.width * 0.7)
            implicitHeight: btn.paintedHeight + 20
            color: msgData.isSender ? "#0B84FF" : "#E5E5EA"
            radius: 16
            border.color: msgData.isSender ? "#0A7BDE" : "#DDDDDD"
            border.width: 1

            Button {
                id: btn
                anchors.centerIn: parent
                text: msgData.text
                onClicked: saveDialog.open()
            }

            FileDialog {
                id: saveDialog
                title: "保存对话记录"
                currentFolder: StandardPaths.url(StandardPaths.Documents)
                fileMode: FileDialog.SaveFile

                onAccepted: {
                    SessionMessageTools.copyFile(msgData.content, selectedFile)
                }
            }
        }
    }

    Component {
        id: imageContent
        Rectangle {
            width: Math.min(300, root.width * 0.7)
            height: 200
            color: msgData.isSender ? "#0B84FF" : "#E5E5EA"
            radius: 16
            border.color: msgData.isSender ? "#0A7BDE" : "#DDDDDD"
            border.width: 1

            Image {
                source: msgData.content
                fillMode: Image.PreserveAspectFit
                anchors.fill: parent
                asynchronous: true
            }
        }
    }

    Component {
        id: audioContent
        Rectangle {
            width: Math.min(200, root.width * 0.7)
            height: 50
            color: msgData.isSender ? "#0B84FF" : "#E5E5EA"
            radius: 16
            border.color: msgData.isSender ? "#0A7BDE" : "#DDDDDD"
            border.width: 1

            Row {
                anchors.centerIn: parent
                spacing: 10

                Button {
                    text: audioPlayer.playbackState === Audio.PlayingState ? "❚❚" : "▶"
                    onClicked: {
                        if (audioPlayer.playbackState === Audio.PlayingState)
                            audioPlayer.pause()
                        else
                            audioPlayer.play()
                    }
                }

                Label {
                    text: msgData.duration + "\""
                }
            }

            MediaPlayer {
                id: audioPlayer
                source: msgData.content
                onPlaybackStateChanged: audioContent.state = playbackState
            }
        }
    }

    Component {
        id: videoContent
        Rectangle {
            width: Math.min(200, root.width * 0.7)
            height: 200 // 视频需要更高空间
            color: msgData.isSender ? "#0B84FF" : "#E5E5EA"
            radius: 16
            border.color: msgData.isSender ? "#0A7BDE" : "#DDDDDD"
            border.width: 1

            VideoOutput {
                id: videoPlayer
                anchors.fill: parent
                fillMode: VideoOutput.PreserveAspectFit
            }

            MediaPlayer {
                id: mediaPlayer
                source: msgData.content
                videoOutput: videoPlayer
            }

            // 播放按钮叠加在视频上
            Button {
                anchors.centerIn: parent
                text: mediaPlayer.playbackState === MediaPlayer.PlayingState ? "❚❚" : "▶"
                onClicked: {
                    if (mediaPlayer.playbackState === MediaPlayer.PlayingState)
                        mediaPlayer.pause()
                    else
                        mediaPlayer.play()
                }
            }
        }
    }
}
