import QtQuick

import "../"
import com.vivenciasoftware.qmlcomponents

TPImage {
	height: imgSize + 10
	width: imgSize + 10
	source: imageSource.indexOf("png") !== -1 ? AppSettings.iconFolder+imageSource : imageSource
	opacity: parent.opacity

	property string imageSource
	property bool bIconOnly: false
	property bool textUnderIcon: true

	Component.onCompleted: {
		if (!bIconOnly) {
			if (!textUnderIcon) {
				anchors.verticalCenter = parent.verticalCenter;
				if (leftAlign) {
					anchors.right = parent.right
					anchors.rightMargin = 5;
				}
				else {
					anchors.left = parent.left
					anchors.leftMargin = 5;
				}
			}
			else {
				anchors.top = parent.top;
				anchors.topMargin = 0;
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
