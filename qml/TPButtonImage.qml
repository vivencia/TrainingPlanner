import QtQuick

Image {
		height: size
		width: size
		fillMode: Image.PreserveAspectFit
		asynchronous: true
		source: "qrc:/images/"+AppSettings.iconFolder+imageSource
		opacity: parent.opacity

		property string imageSource
		property bool bIconOnly: false
		property bool textUnderIcon: true
		property int size: 20

		Component.onCompleted: {
			if (!bIconOnly) {
				if (!textUnderIcon) {
					anchors.verticalCenter = parent.verticalCenter;
					if (leftAlign) {
						anchors.right = parent.right
						anchors.rightMargin = parent.rounded ? 5 : 0;
					}
					else {
						anchors.left = parent.left
						anchors.leftMargin = parent.rounded ? 5 : 10;
					}
				}
				else {
					anchors.top = parent.top;
					anchors.topMargin = 5;
					anchors.horizontalCenter = parent.horizontalCenter;
					anchors.bottomMargin = 10;
				}
			}
			else {
				anchors.horizontalCenter = parent.horizontalCenter
				anchors.verticalCenter = parent.verticalCenter
			}
		}
	}
