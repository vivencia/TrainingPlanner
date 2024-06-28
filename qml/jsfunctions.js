function checkWhetherCanCreatePlan() {
	var ok = true;
	if (mesoSplit.indexOf('A') !== -1) {
		ok &= (txtSplitA.length >= 3);
		txtSplitA.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('B') !== -1) {
		ok &= (txtSplitB.length >= 3);
		txtSplitB.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('C') !== -1) {
		ok &= (txtSplitC.length >= 3);
		txtSplitC.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('D') !== -1) {
		ok &= (txtSplitD.length >= 3);
		txtSplitD.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('E') !== -1) {
		ok &= (txtSplitE.length >= 3);
		txtSplitE.cursorPosition = 0;
	}
	if (mesoSplit.indexOf('F') !== -1) {
		ok &= (txtSplitF.length >= 3);
		txtSplitF.cursorPosition = 0;
	}
	btnCreateExercisePlan.enabled = !bNewMeso && ok;
	return ok;
}

function moveFocusToNextField(from) {
	switch (from) {
		case '0':
			if (txtSplitA.visible)
				txtSplitA.forceActiveFocus();
			else if (txtSplitB.visible)
				txtSplitB.forceActiveFocus();
			else if (txtSplitC.visible)
				txtSplitC.forceActiveFocus();
			else if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				btnCreateExercisePlan.forceActiveFocus();
		break;
		case 'A':
			if (txtSplitB.visible)
				txtSplitB.forceActiveFocus();
			else if (txtSplitC.visible)
				txtSplitC.forceActiveFocus();
			else if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				btnCreateExercisePlan.forceActiveFocus();
		break;
		case 'B':
			if (txtSplitC.visible)
				txtSplitC.forceActiveFocus();
			else if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				btnCreateExercisePlan.forceActiveFocus();
		break;
		case 'C':
			if (txtSplitD.visible)
				txtSplitD.forceActiveFocus();
			else if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				btnCreateExercisePlan.forceActiveFocus();
		break;
		case 'D':
			if (txtSplitE.visible)
				txtSplitE.forceActiveFocus();
			else if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				btnCreateExercisePlan.forceActiveFocus();
		break;
		case 'E':
			if (txtSplitF.visible)
				txtSplitF.forceActiveFocus();
			else
				btnCreateExercisePlan.forceActiveFocus();
		break;
		case 'F':
			btnCreateExercisePlan.forceActiveFocus();
		break;
	}
}
