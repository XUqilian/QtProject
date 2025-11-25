// SettingItemView.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import QtQuick.Layouts 1.15
import localRS

Item {
    id: root

    // 输入
    property SettingItem itemModel
    // 鼠标
    property bool hovered: false        // 放在外部是为了监听而非改动
    // 外观配置
    property font font: Qt.font({  pixelSize: 14 })

    property color textColor: "#E0E0E0" //888888
    property color infoTextColor: "B0B0B0"  // Qt.rgba(textColor.r, textColor.g, textColor.b, 0.7)
    property color theme: "#121212" // 000000   // 背
    property color borderColor: "#444444"       // 边
    property color primaryColor: "#0078D7"      // 主
    property color accentColor: "#FF9800"       // 次

    // ==================== 大小 ====================
    width: parent ? parent.width : 100
    height: columnLayout.implicitHeight  // 自动计算高度

    // ==================== 鼠标悬停检测 ====================
    MouseArea {
            anchors.fill: root
            hoverEnabled: true
            onEntered: root.hovered = true
            onExited: root.hovered = false
        }

    // ==================== 背景 ====================
    Rectangle {
        id:rect
        anchors.fill: root
        color: theme
        border.color: hovered ? primaryColor : borderColor  // "#0078d7" : "#cc4771"
        border.width: hovered ? 2 : 0
        radius: 4
        clip: true  // 防止内容溢出圆角

        // ==================== 主布局：垂直排列 ====================
        ColumnLayout {
            id: columnLayout
            width: parent.width
            // height: implicitHeight
            spacing: 4

            // 第一行：标题（大、粗）
            Label {
                text: itemModel?.title ?? ""
                font: Qt.font({
                        family: root.font ? root.font.family : undefined,
                        pixelSize: (root.font && root.font.pixelSize) ? root.font.pixelSize + 4 : 18,
                        bold: true
                })
                color: textColor
                wrapMode: Text.NoWrap
                maximumLineCount: 2
                elide: Text.ElideRight
                Layout.topMargin: 4
                Layout.alignment: Qt.AlignLeft

            }

            // 第二行：说明文字（小、斜体、浅色）
            Label {
                text: itemModel?.info ?? ""
                font: Qt.font({
                    family: root.font ? root.font.family : undefined,
                    pixelSize: (root.font && root.font.pixelSize) ? root.font.pixelSize - 4 : 10,
                    italic: true
                })
                color: infoTextColor
                visible: itemModel ? (itemModel.info && itemModel.info.length > 0) : false
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                elide: Text.ElideRight
                Layout.topMargin: 4
                Layout.bottomMargin: 8
                Layout.alignment: Qt.AlignLeft
            }

            // 第三行：分隔符
            Rectangle{
                height: 1
                // width: parent.width
                Layout.fillWidth: true
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                color: "#ddd"
            }

            // 第四行：内容区（占满剩余宽度）
            // Loader 加载 Component 是直接"替换为"的方式 并且在loader中设置的属性会"替代"加载的组件顶级控件中的对应属性
            Loader {
                id: contentLoader

                // 不强制填满，但建议宽度接近父容器
                Layout.preferredWidth: parent ? parent.width - 16 : 300  // 减去左右 margin
                Layout.maximumWidth: parent ? parent.width - 16 : 400     // 最大也不能太宽

                // 居中对齐（可选，也可左对齐）
                Layout.alignment: Qt.AlignHCenter

                // 视觉缩进
                Layout.leftMargin: 8
                Layout.rightMargin: 8
                Layout.bottomMargin: 4

                // 高度自适应
                height: contentLoader.item ? contentLoader.item.implicitHeight : 0

                sourceComponent: componentForType(itemModel.type)
            }

            // 第五行：整体分隔符
            Rectangle{
                height: 3
                // width: parent.width
                Layout.fillWidth: true
                Layout.leftMargin: 2
                Layout.rightMargin: 2
                Layout.bottomMargin: 4
                color: "#00cc6a"    // 0078d4
            }
        }

    }


    // MouseArea如果放在这里是z轴最上层，会阻挡所有鼠标事件



    // ==================== 类型映射逻辑（保持不变）====================
    function componentForType(type) {
        switch (type) {
            case SettingItem.Boolean:
                return boolComponent
            case SettingItem.Integer:
            case SettingItem.Number:
                return numberComponent
            case SettingItem.String:
                return stringComponent
            case SettingItem.TextArea:
                return textAreaComponent

            case SettingItem.FilePath:
                return fileComponent
            case SettingItem.DirPath:
                return dirComponent

            case SettingItem.Color:
                return colorComponent
            case SettingItem.Font:
                return fontComponent

            case SettingItem.Enum:
                return enumComponent
            case SettingItem.Flag:
                return flagComponent
            case SettingItem.List:
                return listComponent

            case SettingItem.Action:
                return actionComponent

            case SettingItem.Separator:
                return separatorComponent
            case SettingItem.Group:
                return groupComponent

            default:
                return defaultComponent
        }
    }

    // ==================== 各 Component 定义（略）====================
    // 外界使用者应决定蓝图显示的最终大小，而蓝图应确定自己的实际大小(设置最小宽高以及implicit)且规划好布局(组件符合美学排列)
    Component {
        id: boolComponent
        Switch {
            checked: itemModel.value
            onCheckedChanged: itemModel.value = checked
        }
    }

    Component {
        id: numberComponent

        // 布局组合
        RowLayout {
            spacing: 10

            Slider {
                id: slider
                Layout.preferredWidth: 4
                Layout.fillWidth: true

                Layout.minimumWidth:80
                Layout.minimumHeight: 24

                value: itemModel.value
                from: itemModel.configs["from"] !== undefined ? itemModel.configs["from"] : 0
                to: itemModel.configs["to"] !== undefined ? itemModel.configs["to"] : 1
                stepSize: itemModel.configs["stepSize"] !== undefined
                       ? itemModel.configs["stepSize"]
                       : (itemModel.type === SettingItem.Integer ? 1 : 0.01)

                onValueChanged:  itemModel.value  = (itemModel.type === SettingItem.Integer) ? Math.round(value) : value

            }

            // TextField：用于精确输入
            TextField {
                Layout.preferredWidth: 1
                Layout.fillWidth: true
                color:textColor

                Layout.minimumWidth:20
                Layout.minimumHeight: 24

                text: itemModel.value.toLocaleString(Qt.locale(), "f", (itemModel.type === SettingItem.Integer) ? 0 : 2)

                validator: DoubleValidator {
                    bottom: (itemModel.configs["from"] !== undefined) ? itemModel.configs["from"] : -Infinity
                    top: (itemModel.configs["to"] !== undefined) ? itemModel.configs["to"] : Infinity
                }

                onAccepted: {
                    var input = Number.fromLocaleString(Qt.locale(),text)  // 正确解析本地化数字

                    if (!isNaN(input) && input >= 0.0 && input <= 1.0)
                    {
                        // 溢出验证
                        var min = (itemModel.configs["from"] !== undefined) ? itemModel.configs["from"] : -Infinity;
                        var max = (itemModel.configs["to"] !== undefined) ? itemModel.configs["to"] : Infinity;
                        if (input < min || input > max) return;

                        itemModel.value = (itemModel.type === SettingItem.Integer) ? Math.round(input) : input
                    }

                }

            }


        }
    }

    Component {
        id: stringComponent

        TextField {
            id: textField

            text: itemModel.value
            placeholderText: (itemModel.configs["placeholderText"] !== undefined) ? itemModel.configs["placeholderText"] : ""

            // 输入验证器：始终存在，但可“失效”
            validator: RegularExpressionValidator  {
                regularExpression: {
                    var pattern = itemModel.configs["validator"];
                    if (pattern && typeof pattern === "string" && pattern.trim() !== "") {
                        try {
                            return new RegExp(pattern);
                        } catch (e) {
                            console.warn("Invalid regex:", pattern);
                            return /(?:)/;
                        }
                    }
                    return /(?:)/;
                }
            }

            echoMode: {
                let mode = itemModel.configs["echoMode"];

                // 检查是否为 number 类型（且是整数）
                if (typeof mode !== 'number' || !isFinite(mode) || Math.floor(mode) !== mode) {
                    console.warn(
                        `[Config Warning] 'echoModeInput' is not a valid integer enum.`,
                        `Expected int (0-3), got: '${mode}' (type: ${typeof mode})`,
                        `Item: ${itemModel.title || 'unknown'}`
                    );
                    return TextInput.Normal;
                }

                // 检查是否在合法枚举范围内 [0, 3]
                if (mode < 0 || mode > 3) {
                    console.warn(
                        `[Config Warning] 'echoModeInput' value out of range.`,
                        `Valid: 0=Normal, 1=Password, 2=PasswordEchoOnEdit, 3=NoEcho, got: ${mode}`,
                        `Item: ${itemModel.title || 'unknown'}`
                    );
                    return TextInput.Normal;
                }

                // 安全 switch（虽然 mode 是 int，但仍明确处理）
                switch (mode) {
                    case TextInput.Password:
                        return TextInput.Password;
                    case TextInput.PasswordEchoOnEdit:
                        return TextInput.PasswordEchoOnEdit;
                    case TextInput.NoEcho:
                        return TextInput.NoEcho;
                    case TextInput.Normal: // 明确处理 0
                    default:
                        return TextInput.Normal;
                }
            }

            onTextChanged: itemModel.value = text
        }
    }

    Component {
        id: textAreaComponent
        TextArea {            
            text: value
            placeholderText: (itemModel.configs["placeholderText"] !== undefined) ? itemModel.configs["placeholderText"] : ""
            wrapMode: Text.Wrap
            font.family: "Consolas, Courier, monospace"
            font.pixelSize: 12

            onTextChanged: itemModel.value = text
        }
    }


    Component {
        id: fileComponent
        RowLayout {
            spacing: 10

            TextField {
                Layout.preferredWidth: 4
                Layout.fillWidth: true

                Layout.minimumWidth:80
                Layout.minimumHeight: 32

                text: itemModel.value
                placeholderText:  itemModel.configs["placeholderText"] !== undefined ? itemModel.configs["placeholderText"] : ""
                readOnly: true  // 推荐只读，防止误改
            }
            Button {
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                Layout.minimumWidth:20
                Layout.minimumHeight: 32

                text: "📁Select File"
                onClicked: fileDialog.open()
            }

            FileDialog {
                id: fileDialog
                title: "Select File"
                fileMode: FileDialog.OpenFile

                selectedFile: Qt.resolvedUrl(itemModel.value)
                // 正确的 nameFilters 格式
                nameFilters: itemModel.configs["nameFilters"] !== undefined ? itemModel.configs["nameFilters"] : []

                onAccepted: itemModel.value = selectedFile.toString()

            }
        }
    }

    Component {
        id: dirComponent
        RowLayout {
            spacing: 10

            TextField {
                Layout.preferredWidth: 4
                Layout.fillWidth: true

                Layout.minimumWidth:80
                Layout.minimumHeight: 32

                text: itemModel.value
                placeholderText: itemModel.configs["placeholderText"] !== undefined ? itemModel.configs["placeholderText"] : ""
                readOnly: true
            }
            Button {
                Layout.preferredWidth: 1
                Layout.fillWidth: true

                Layout.minimumWidth:20
                Layout.minimumHeight: 32

                text: "📂Select Dir"
                onClicked: dirDialog.open()
            }

            FolderDialog {
                id: dirDialog
                title: "Select Dir"
                selectedFolder: Qt.resolvedUrl(itemModel.value)

                onAccepted: itemModel.value = selectedFolder.toString()

            }
        }
    }

    Component {
        id: colorComponent

        RowLayout {
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 3
                Layout.fillWidth: true

                Layout.minimumWidth:80
                Layout.minimumHeight: 32

                radius: 4
                color: itemModel.value
                border.width: 1
                border.color: "#ccc"
                Layout.alignment: Qt.AlignCenter
            }

            Button {
                Layout.preferredWidth: 2
                Layout.fillWidth: true

                Layout.minimumWidth:20
                Layout.minimumHeight: 32

                text: "🎨 Select Color"

                onClicked: colorDialog.open()
            }

            ColorDialog {
                id: colorDialog
                title: "Select Color"
                // 注意：color 是 QColor，itemModel.value 应为 Qt.rgba() 或 "#rrggbb"
                selectedColor: itemModel.value

                onAccepted: itemModel.value = selectedColor
            }
        }
    }

    Component {
        id: fontComponent

        RowLayout {
            spacing: 8

            // 字体预览文本
            Label {
                Layout.preferredWidth: 3
                Layout.fillWidth: true

                Layout.minimumWidth:80
                Layout.minimumHeight: 32

                text: "Text"
                font: itemModel.value
            }

            Button {
                Layout.preferredWidth: 2
                Layout.fillWidth: true

                Layout.minimumWidth:20
                Layout.minimumHeight: 32

                // 显示字体信息
                text: itemModel.value ?
                    `font: ${itemModel.value.family}, ${itemModel.value.pointSize}pt` :
                    "Select Font"

                onClicked: fontDialog.open()
            }

            FontDialog {
                id: fontDialog
                title: "Select Font"
                selectedFont: itemModel.value

                onAccepted: itemModel.value = selectedFont
            }
        }

    }


    Component {
        id: enumComponent
        ComboBox {
            model: itemModel.options
            currentIndex: itemModel.options.indexOf(itemModel.value.toString())
            editable: false

            onCurrentIndexChanged:
            {
                if (currentIndex >= 0 && currentIndex < count) {
                       // 安全获取当前项的文本
                       var selectedValue = textAt(currentIndex);    //model[currentIndex];
                       itemModel.value = selectedValue;
                   }

                // itemModel.value = currentText    // 这种方式会有一瞬间的空白导致无法触发
            }
        }
    }


    Component {
        id: flagComponent

        RowLayout {
            id: column
            spacing: 4

            // 存储每个 checkbox 的选中状态（布尔数组）
            property var checkedState: []

            Repeater {
                Layout.preferredWidth: 3
                Layout.fillWidth: true

                Layout.minimumWidth:80
                Layout.minimumHeight: 32

                id: repeater
                model: itemModel.options  // ["Option A", "Option B", ...]

                delegate: CheckBox {
                        width: parent.width
                        anchors.margins: 8
                        spacing: 8

                        text: modelData
                        // 绑定到外部状态数组
                        checked: column.checkedState[index]
                        onCheckedChanged: column.checkedState[index] = checked
                    }

            }

            // 保存按钮：更新 itemModel.value
            Button {
                Layout.preferredWidth: 2
                Layout.fillWidth: true

                Layout.minimumWidth:80
                Layout.minimumHeight: 32

                text: "Save"
                onClicked: {
                    itemModel.value = getChecks()  // 直接赋值字符串数组
                }
            }


            // --- 公共函数：统一入口/出口 ---

            // 初始化：使用 itemModel.value 设置哪些项被选中
            function setChecks(selectedOptions) {
                if (!Array.isArray(selectedOptions)) return

                for (var i = 0; i < itemModel.options.length; i++) {
                    // 如果当前选项文本在 selectedOptions 中，则勾选
                    column.checkedState[i] = selectedOptions.includes(itemModel.options[i])
                }
            }

            // 获取当前选中项（返回字符串数组）
            function getChecks() {
                var result = []
                for (var i = 0; i < itemModel.options.length; i++) {
                    if (column.checkedState[i]) {
                        result.push(itemModel.options[i])
                    }
                }
                return result
            }

            // === 自动初始化：使用 itemModel.value ===
            Component.onCompleted: {
                // 初始化状态数组
                column.checkedState = new Array(itemModel.options.length).fill(false)

                // 使用 itemModel.value 初始化选中状态
                var initialValues = itemModel.value || []
                if (Array.isArray(initialValues)) {
                    setChecks(initialValues)
                } else {
                    console.warn("itemModel.value should be an array of strings. Got:", typeof initialValues)
                    setChecks([])
                }
            }
        }
    }

    Component {
        id: listComponent

        ColumnLayout{
            spacing: 8

            //临时数据（编辑区）
            property var tempValue: []

            //初始化：从 itemModel.value 加载
            Component.onCompleted: {
                tempValue = (itemModel.value && Array.isArray(itemModel.value))
                           ? itemModel.value.slice()
                           : [];
            }

            function cleanupNewItems(list) {
                var pattern = /^New Item\s*\d*$/;  // 匹配 "新条目", "新条目1", "新条目 2" 等
                var i = list.length;
                while (i--) {
                    if (pattern.test(list[i]) || list[i] === "") {
                        list.splice(i, 1);  // 倒序删除，避免索引错乱
                    }
                }
            }

            RowLayout {
                spacing: 8
                Layout.fillWidth: true

                Button {
                    text: "Add"
                    onClicked: tempValue.push("New Item")
                }
                Button {
                    text: "Save"
                    onClicked: {
                        cleanupNewItems(tempValue)
                        itemModel.value = tempValue;
                    }
                }
            }

            //可编辑列表（直接绑定 tempValue）
            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true

                Repeater {
                    model: tempValue
                    delegate:
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            TextField {
                                Layout.fillWidth: true
                                Layout.minimumWidth:80
                                Layout.minimumHeight: 32

                                implicitHeight: 32

                                text: modelData

                                onTextChanged: tempValue[index] = text
                            }

                            Button {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32

                                text: "X"

                                onClicked: tempValue.splice(index, 1)
                            }
                        }

                }
            }
        }
    }


    Component {
        id: actionComponent
        Button {
            text: "Exec" // itemModel.title
            onClicked: itemModel.value = true
            // 可 emit signal: itemModel.buttonClicked()
            // 直接itemModel.value = true; 这里的值类型需要适配 在WRITE函数中完善执行的Action
        }
    }


    Component {
        id: separatorComponent
        Rectangle {
            height: 2; width: parent.width
            color: "#ddd"
        }
    }

    Component {
        id: groupComponent
        Label {
            text: "【" + itemModel.title + "】"
            font.bold: true; color: "#555"
            horizontalAlignment: Text.AlignHCenter
        }
    }


    Component {
        id: defaultComponent
        Label {
            text: "⚠ Unknown Type: " + itemModel.type
            color: "red"
        }
    }
}
