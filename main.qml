import QtQuick 2.0
import QtQuick.Controls 1.0

Rectangle {
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#ffffff"
        }

        GradientStop {
            position: 1
            color: "#000000"
        }
    }

    Button {
        id: button1
        objectName: "qmlButton"
        x: 62
        y: 30
        width: 126
        height: 36
        text: qsTr("Button")
    }


    Label {

        function setLtext()
        {
            label.text="QMetaObject::invokeMethod(...)"
        }

        function setLColor(color, fontsize)
        {
            label.color = color
            label.font.pointSize = fontsize
        }

        id: label
        objectName: "objNameL"
        x: 62
        y: 80
        width: 371
        height: 58
        color: "#9a2cf2"
        text: qsTr("Label")
        style: Text.Normal
        styleColor: "#f20d0d"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.family: "Verdana"
        font.pointSize: 12
    }

    Button {

        //signal <name>[([<type> <parameter name>[, ...]])]
        //A signal is emitted by invoking the signal as a method.
        signal qmlSignal(var anObject)
        id: button2
        objectName: "objNameB"
        x: 62
        y: 150
        width: 126
        height: 36
        text: "emit Item"

        onClicked:
        {
            button2.qmlSignal(button2);
        }

    }

}


