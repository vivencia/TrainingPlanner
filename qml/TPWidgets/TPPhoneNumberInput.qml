import QtQuick
import QtQuick.Controls

TPTextInput {
    inputMethodHints: Qt.ImhDigitsOnly
    validator: RegularExpressionValidator { regularExpression: /\+?[0-9]{0,2} ?(\([0-9]{0,2}\))? ?[0-9]{0,5}-?[0-9]{0,4}/ }
    placeholderText: "+55 (XX) XXXXX-XXXX"
    maximumLength: 19
    ToolTip.text: qsTr("Invalid phone number")

    property bool phoneNumberOK: false

    onTextEdited: {
        let oldText = text;
        let oldCursor = cursorPosition;
        text = formatPhoneNumber(text);
        cursorPosition = adjustCursorPosition(oldText, text, oldCursor);
        if (text.length === 19) {
            ToolTip.visible = false;
            phoneNumberOK = true;
        }
        else {
            ToolTip.visible = true;
            phoneNumberOK = false;
        }
    }

    function formatPhoneNumber(digits: string) : string {
        // Remove all non-digits
        digits = digits.replace(/\D/g, '');

        // Format based on length
        let formatted = "";
        if (digits.length === 0)
            return ""; // Empty input shows nothing
        else if (digits.length <= 2)
            formatted = "+" + digits;
        else if (digits.length <= 4)
            formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2) + ")";
        else if (digits.length <= 9)
            formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2, 4) + ") " + digits.substring(4);
        else
            formatted = "+" + digits.substring(0, 2) + " (" + digits.substring(2, 4) + ") " + digits.substring(4, 9) + "-" + digits.substring(9);
        return formatted;
    }

    // Function to calculate cursor position after formatting
    function adjustCursorPosition(oldText: string, newText: string, oldCursor: int) : int {
        // Count non-digit characters (formatting chars) up to old cursor position
        let nonDigitsBeforeCursor = oldText.substring(0, oldCursor).replace(/[0-9]/g, '').length;
        // Count digits up to old cursor position
        let digitsBeforeCursor = oldText.substring(0, oldCursor).replace(/\D/g, '').length;
        // Calculate new cursor position in formatted text
        let newFormatted = formatPhoneNumber(newText);
        let digitCount = 0;
        let nonDigitCount = 0;
        for (let i = 0; i < newFormatted.length; i++) {
            if (/\d/.test(newFormatted[i])) {
                digitCount++;
                if (digitCount === digitsBeforeCursor)
                    return i + 1; // Place cursor after the current digit
            } else
                nonDigitCount++;
        }
        return newFormatted.length; // Fallback to end of text
    }
}
