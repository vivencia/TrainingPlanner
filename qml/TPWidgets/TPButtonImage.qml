import QtQuick

import "../"
import org.vivenciasoftware.TrainingPlanner.qmlcomponents

TPImage {
	source: imageSource
	opacity: parent.opacity
	enabled: parent.checkable ? !parent.checked : parent.enabled

	property string imageSource
}
