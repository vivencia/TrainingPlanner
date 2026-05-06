//pragma ComponentBehavior: Bound

import QtQuick
import QtMultimedia
import QtQuick.Pdf

import TpQml

VideoOutput {
	id: _control
	clip: true

//public:
	required property string mediaUrl
	property int windowState: TPFileViewer.WS_NORMAL
	property FileOperations fileOps
	readonly property MediaPlayer mediaPlayer: _mediaPlayer
	readonly property AudioOutput audioOutput: _audioOutput

	property list<int> previewControls: [MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Mute]
	property list<int> fullScreenControls: [MediaControls.CT_Play, MediaControls.CT_Stop, MediaControls.CT_Rewind,
						MediaControls.CT_FastForward, MediaControls.CT_VolumeUp, MediaControls.CT_VolumeDown, MediaControls.CT_Mute]

//private:
	property string _media_volume
	property bool _media_playing

	MediaPlayer {
		id: _mediaPlayer
		videoOutput: _control
		source: _control.mediaUrl

		audioOutput: AudioOutput {
			id: _audioOutput
			volume: 0.5
			muted: true

			Component.onCompleted: {
				_control._media_volume = Qt.binding(function() { return _audioOutput.muted ?
																qsTr("Muted") : parseInt(_audioOutput.volume * 100, 10); });
			}
		}

		onMediaStatusChanged: {
			if (mediaStatus === MediaPlayer.EndOfMedia)
				mediaControls.controlLimitReached(MediaControls.CT_Play);
		}
		onPositionChanged: lblTime.text = AppUtils.formatTime(duration - _mediaPlayer.position);
	} //MediaPlayer

	TPLabel {
		id: lblTime
		useBackground: true
		horizontalAlignment: Text.AlignHCenter
		x: 10
		y: 10
	} //lblTime

	TPLabel {
		id: lblVolume
		text: qsTr("Volume: ") + _control._media_volume
		useBackground: true
		horizontalAlignment: Text.AlignHCenter
		x: 10
		y: 15 + lblTime.height
	} //lblVolume

	Timer {
		id: pressedTimer
		interval: 500
		repeat: true
		triggeredOnStart: true
	}

	Rectangle {
		radius: 8
		color: AppSettings.paneBackgroundColor
		border.color: AppSettings.fontColor
		opacity: 0.8
		width: mediaControls.controlSize.width
		height: mediaControls.controlSize.height

		anchors {
			horizontalCenter: parent.horizontalCenter
			bottom: parent.bottom
			bottomMargin: _control.fileOps !== null ? AppSettings.itemDefaultHeight + 30 : 10
		}

		MediaControls {
			id: mediaControls
			fileOps: _control.fileOps
			availableControls: _control.windowState === TPFileViewer.WS_NORMAL ? _control.previewControls : _control.fullScreenControls

			onControlClicked: (type) => {
				switch (type) {
				case MediaControls.CT_Play:
					_control.play(false);
					setEnabled(MediaControls.CT_Mute, true);
					break;
				case MediaControls.CT_Stop:
					_control.stop(false);
					break;
				case MediaControls.CT_Mute:
					_control.mute();
					break;
				}
			}

			onControlPressed: (type) => _control.pressedOperations(type, true);
			onControlReleased: (type) => _control.pressedOperations(type, false);

			Component.onCompleted: setEnabled(MediaControls.CT_Mute, false);
		}		
	}

	function play(emulate_click: bool): void {
		if (emulate_click)
			mediaControls.emulateControlClick(MediaControls.CT_Play);
		else {
			if (!mediaPlayer.playing)
				mediaPlayer.play();
			else
				mediaPlayer.pause();
		}
	}

	function stop(emulate_click: bool): void {
		if (emulate_click)
			mediaControls.emulateControlClick(MediaControls.CT_Stop);
		else
			mediaPlayer.stop();
	}

	function mute(): void {
		audioOutput.muted = !audioOutput.muted;
	}

	function pressedOperations(op: int, begin: bool) : void {
		let pressed_function;
		switch (op) {
			case MediaControls.CT_FastForward:
				pressed_function = function () {
					mediaPlayer.position += 5000;
					if (mediaPlayer.position >= mediaPlayer.duration)
						mediaControls.controlLimitReached(MediaControls.CT_FastForward);
				};
			break;
			case MediaControls.CT_Rewind:
				pressed_function = function () {
					mediaPlayer.position -= 5000;
					if (mediaPlayer.position <= 0)
						mediaControls.controlLimitReached(MediaControls.CT_Rewind);
				};
			break;
			case MediaControls.CT_VolumeUp:
				pressed_function = function () {
					audioOutput.volume += 0.1;
					if (audioOutput.volume >= 1.0)
						mediaControls.controlLimitReached(MediaControls.CT_VolumeUp);
				};
			break;
			case MediaControls.CT_VolumeDown:
				pressed_function = function () {
					audioOutput.volume -= 0.1;
					if (audioOutput.volume === 0.0)
						mediaControls.controlLimitReached(MediaControls.CT_VolumeDown);
				};
			break;
			default: return;
		}

		if (begin) {
			pressedTimer.triggered.connect(pressed_function);
			pressedTimer.start();
		}
		else {
			pressedTimer.stop();
			pressedTimer.triggered.disconnect(pressed_function);
		}
	}

	function changeState(window_state: int): void {
		windowState = window_state;
		_media_playing = _mediaPlayer.playing;
		if (_media_playing)
			play(true);
		if (windowState === TPFileViewer.WS_NORMAL) {
			lblTime.font = AppGlobals.regularFont;
			lblTime.height = AppSettings.itemDefaultHeight;
			fillMode = VideoOutput.Stretch
			mediaControls.availableControls = previewControls;
		}
		else {
			lblTime.font = AppGlobals.largeFont;
			lblTime.height = AppSettings.itemLargeHeight;
			fillMode = VideoOutput.PreserveAspectFit;
			mediaControls.availableControls = fullScreenControls;
		}
		if (_media_playing)
			play(true);
	}
}
