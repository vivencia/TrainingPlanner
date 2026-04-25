import QtQuick
import QtQuick.Controls

import TpQml
import TpQml.Widgets

Page {
	id: _page
	width: AppSettings.pageWidth
	height: AppSettings.pageHeight

	property string imageSource
	property double backgroundOpacity: 0
	readonly property int headerHeight: 0.08 * height
	readonly property int footerHeight: 0.10 * height

	signal pageActivated();
	signal pageDeActivated();

	background: TPBackRec {
		useImage: _page.imageSource.length > 0
		image_size: Qt.size(AppSettings.pageWidth, AppSettings.pageHeight * 1.1)
		sourceImage: _page.imageSource
		overlayOpacity: _page.backgroundOpacity
	}
}
