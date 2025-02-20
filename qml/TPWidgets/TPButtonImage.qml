import QtQuick

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPImage {
	source: imageSource
	opacity: parent.opacity
	enabled: parent.checkable ? !parent.checked : parent.enabled

	property string imageSource
	property bool bIconOnly: false
	property bool textUnderIcon: true

	Component.onCompleted: {
		if (!bIconOnly) {
			if (!textUnderIcon) {
				anchors.verticalCenter = parent.verticalCenter
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
