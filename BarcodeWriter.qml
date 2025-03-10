import QtQuick
import QtQuick.Controls.Material
import WriteBarcode 1.0

Rectangle {
    id: readerRect
    anchors.fill: parent

    WriteBarcode{
        id: barcodeWriter
    }

    TextField{
        id: barcodeText
        height: 50
        width: parent.width * .60
        anchors{
            top: parent.top
            left: parent.left
            leftMargin: 10
            topMargin: 10
        }

        placeholderText: "Enter text to generate barcode..."
        verticalAlignment: TextField.AlignVCenter
        clip: true
    }

    Button{
        id: generateButton
        width: 120
        height: 50
        text: "Generate"
        anchors{
            left: barcodeText.right
            leftMargin: 20
            verticalCenter: barcodeText.verticalCenter
        }

        onClicked: {
            barcodeWriter.writeAndSaveBarcode("qrcode", barcodeText.text)
        }
    }

    Image {
        id: barcodeImage
        source: barcodeWriter.imageUrl

        anchors{
            left: parent.left
            top: barcodeText.bottom
            topMargin: 20
            bottom: parent.bottom
            bottomMargin: 10
            right: parent.right
        }

        fillMode: Image.PreserveAspectFit
    }
}












