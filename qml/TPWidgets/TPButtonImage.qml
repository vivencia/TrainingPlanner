import QtQuick

import "../"
import com.vivenciasoftware.qmlcomponents

TPImage {
	source: imageSource.indexOf("png") !== -1 ? AppSettings.iconFolder+imageSource : imageSource
	opacity: parent.opacity

	property string imageSource
	property bool bIconOnly: false
	property bool textUnderIcon: true

	Component.onCompleted: {
		if (!bIconOnly) {
			anchors.top = parent.top;
			anchors.topMargin = 5;
			if (!textUnderIcon) {
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
