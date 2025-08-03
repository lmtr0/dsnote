/* Copyright (C) 2023-2024 Michal Kosciesza <michal@mkiol.net>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

SilicaItem {
    id: root

    property alias textArea: _textArea
    property alias flick: _flick
    property alias showTopBorder: topBorder.visible
    property alias showBottomBorder: bottomBorder.visible
    property bool canUndo: false
    property bool canRedo: false
    property bool canClear: true
    property bool canSwitch: false
    readonly property bool canScrollBeginning: !flick.atYBeginning && flick.contentHeight > 0
    readonly property bool canScrollBottom: !flick.atYEnd && flick.contentHeight > 0
    property alias placeholderLabel: _placeholderLabel.text
    property alias formatName: formatButton.text
    property bool formatInvalid: false

    signal clearClicked()
    signal copyClicked()
    signal undoClicked()
    signal formatClicked()
    signal switchClicked()

    function scrollToBottom() {
        if (canScrollBottom) {
            flick.contentY = flick.contentHeight - flick.height
            flick.returnToBounds()
        }
    }

    function scrollToBeginning() {
        if (canScrollBeginning) {
            flick.contentY = 0
            flick.returnToBounds()
        }
    }

    Rectangle {
        visible: root.highlighted
        anchors.fill: parent
        color: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)
        opacity: Theme.opacityLow
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.textArea.forceActiveFocus()
    }

    Flickable {
        id: _flick

        y: topBorder.height + Theme.paddingMedium
        width: parent.width
        height: Math.min(parent.height - bottomBorder.height - topBorder.height - Theme.paddingMedium, root.textArea.height)
        contentHeight: root.textArea.height
        contentWidth: width
        clip: true

        TextArea {
            id: _textArea

            enabled: root.enabled
            width: parent.width
            background: null
            labelComponent: null
            opacity: root.enabled ? 1.0 : 0.8
            Behavior on opacity { FadeAnimator {} }
        }

        VerticalScrollDecorator {
            flickable: root.flick
            opacity: 1.0
        }
    }

    Label {
        id: _placeholderLabel

        y: _flick.y + _textArea.textTopPadding + Theme.paddingSmall
        x: Theme.horizontalPageMargin
        visible: opacity > 0.0
        opacity: _textArea.text.length === 0 && text.length !== 0 ? 1.0 : 0.0
        Behavior on opacity { FadeAnimator {} }
        width: parent.width - 2 * Theme.horizontalPageMargin
        wrapMode: Text.Wrap
        color: _textArea.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
    }

    Item {
        id: toolsRow

        property double size: Theme.itemSizeSmall * 0.6

        opacity: root.textArea.highlighted ? 0.0 :
                 root.enabled && (flick.moving || copyButton.pressed || clearButton.pressed ||
                                  undoButton.pressed || bottomButton.pressed || beginningButton.pressed ||
                                  switchButton.pressed ||
                 flick.contentHeight <= (root.height - size)) ? 1.0 : 0.4
        Behavior on opacity { FadeAnimator {} }
        visible: opacity > 0.0
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: Theme.horizontalPageMargin
        anchors.left: parent.left
        anchors.leftMargin: Theme.horizontalPageMargin
        height: size + Theme.paddingLarge

        Row {
            spacing: Theme.paddingSmall
            height: parent.height
            anchors.left: parent.left

            Button {
                id: formatButton

                preferredWidth: Theme.buttonWidthExtraSmall
                height: toolsRow.size
                visible: _settings.subtitles_support && text.length !== 0
                onClicked: root.formatClicked()
                color: root.formatInvalid ? Theme.errorColor : Theme.primaryColor
                backgroundColor: Theme.rgba(color, 0.05)
            }

            IconButton {
                id: copyButton

                width: toolsRow.size
                height: toolsRow.size
                visible: root.textArea.text.length !== 0
                icon.width: width; icon.height: height
                icon.source: "image://theme/icon-m-clipboard?" + (pressed ? Theme.highlightColor : Theme.primaryColor)
                onClicked: root.copyClicked()
            }
        }

        Row {
            spacing: Theme.paddingSmall
            height: parent.height
            anchors.right: parent.right

            IconButton {
                id: clearButton

                width: toolsRow.size
                height: toolsRow.size
                icon.width: width; icon.height: height
                visible: root.canClear && !textArea.readOnly && textArea.text.length !== 0
                icon.source: "image://theme/icon-m-clear?" + (pressed ? Theme.highlightColor : Theme.primaryColor)
                onClicked: root.clearClicked()
            }
            IconButton {
                id: undoButton

                width: toolsRow.size
                height: toolsRow.size
                icon.width: width; icon.height: height
                visible: !root.textArea.readOnly && (root.canUndo || root.canRedo)
                icon.source: (root.canUndo ? "image://theme/icon-m-rotate-left?" : "image://theme/icon-m-rotate-right?") +
                             (pressed ? Theme.highlightColor : Theme.primaryColor)
                onClicked: root.undoClicked()
            }
            IconButton {
                id: bottomButton

                width: toolsRow.size
                height: toolsRow.size
                icon.width: width; icon.height: height
                visible: root.canScrollBottom
                icon.source: "image://theme/icon-m-page-down?" +
                             (pressed ? Theme.highlightColor : Theme.primaryColor)
                onClicked: {
                    root.scrollToBottom()
                }
            }
            IconButton {
                id: beginningButton

                width: toolsRow.size
                height: toolsRow.size
                icon.width: width; icon.height: height
                visible: root.canScrollBeginning
                icon.source: "image://theme/icon-m-page-up?"+
                             (pressed ? Theme.highlightColor : Theme.primaryColor)
                onClicked: {
                    root.scrollToBeginning()
                }
            }
            IconButton {
                id: switchButton

                width: toolsRow.size
                height: toolsRow.size
                visible: root.canSwitch
                icon.width: width; icon.height: height
                icon.source: "image://theme/icon-m-transfer?" + (pressed ? Theme.highlightColor : Theme.primaryColor)
                onClicked: root.switchClicked()
            }
        }

    }

    Rectangle {
        id: bottomBorder

        visible: false
        width: parent.width
        height: visible ? 2 : 0
        opacity: Theme.opacityLow
        color: Theme.highlightColor
        anchors.bottom: parent.bottom
    }

    Rectangle {
        id: topBorder

        visible: false
        width: parent.width
        height: visible ? 2 : 0
        opacity: Theme.opacityLow
        color: Theme.highlightColor
        anchors.top: parent.top
    }

    Component.onCompleted: scrollToBottom()
}
