import QtQuick
import QtQuick.Controls

TPComboBox {
	id: cboMuscularGroup
	selectable: false

	model: ListModel { id: groupsModel }

	function fillMuscularGroupsModel(groups_str: string): void {
		let groups = groups_str.split('|');
		groupsModel.clear();
		if (groups.length > 0)
		{
			let display_text = "";
			for (let i = 0; i < groups.length; ++i) {
				if (groups[i] !== "") {
					groupsModel.append({ text: groups[i], value: "", enabled: true });
					display_text += groups[i] + ", ";
				}
			}
			displayText = display_text.substring(0, display_text.length - 2);
			currentIndex = 0;
		}
		else
			displayText = qsTr("<- Choose muscle groups...");
	}
}
