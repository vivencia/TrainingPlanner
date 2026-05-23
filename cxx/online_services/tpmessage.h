#pragma once

#include <QDateTime>
#include <QObject>
#include <QVariant>

enum TPMessageFields {
	TPMESSAGE_FIELD_ID,
	TPMESSAGE_FIELD_TITLE,
	TPMESSAGE_FIELD_TEXT,
	TPMESSAGE_FIELD_ICON,
	TPMESSAGE_FIELD_DATE,
	TPMESSAGE_FIELD_TIME,
	TPMESSAGE_FIELD_FILE,
	TPMESSAGE_FIELD_EXTRA_INFO,
	TPMESSAGE_FIELD_EXTRA_ICON,
	TPMESSAGE_FIELD_ACTIONS,
	TPMESSAGE_FIELD_EXPIRATION,
	TPMESSAGE_FIELD_STICKY,
};

QT_FORWARD_DECLARE_CLASS(TPFilePath)
QT_FORWARD_DECLARE_CLASS(QTimer)

class TPMessage : public QObject
{

Q_OBJECT

public:
	inline explicit TPMessage() : QObject{nullptr} {}
	~TPMessage();
	inline qsizetype id() const { return m_id; }
	inline void setId(const qsizetype id) { m_id = id; emit dataChanged(TPMESSAGE_FIELD_ID); }

	inline const QString &title() const { return m_title; }
	inline void setTitle(QString &&new_title) { m_title = std::forward<QString>(new_title); emit dataChanged(TPMESSAGE_FIELD_TITLE); }
	inline const QString &text() const { return m_text; }
	inline void setText(QString &&new_text) { m_text = std::forward<QString>(new_text); emit dataChanged(TPMESSAGE_FIELD_TEXT); }
	inline const QString &iconSource() const { return m_icon; }
	inline void setIconSource(QString &&new_icon) { m_icon = std::forward<QString>(new_icon); emit dataChanged(TPMESSAGE_FIELD_ICON); }
	inline TPFilePath *fileName() const { return m_tpFilePath; }
	void setFileName(const TPFilePath &tpfilepath);
	void setFileName(const QString &filename);
	inline const QString &extraInfoText() const { return m_extraInfoText; }
	inline void setExtraInfoText(const QString &new_label) { m_extraInfoText = new_label; emit dataChanged(TPMESSAGE_FIELD_EXTRA_INFO); }
	inline const QString &extraInfoImage() const { return m_extraInfoImage; }
	inline void setExtraInfoImage(const QString &new_image) { m_extraInfoImage = new_image; emit dataChanged(TPMESSAGE_FIELD_EXTRA_ICON); }
	inline const bool &isExpirable() const { return m_timedExpiration; }
	void setExpiration(const uint secs = 0);

	QString date() const;
	QString time() const;

	inline const bool plugged() const { return m_plugged; }
	void plug();
	void unplug() { emit killMessage(this);}

	inline const bool sticky() const { return m_sticky; }
	inline void setSticky(const bool sticky) { m_sticky = sticky; emit dataChanged(TPMESSAGE_FIELD_STICKY); }

	//When m_tpfilepath is set, OnlineMessages sets up a TPFileViewer which counts as a action
	inline const bool hasActions() const { return m_tpFilePath || !m_actions.isEmpty(); }

	/**
	  *@param message_id
	  *@param action_name
	  *@param remove if Specified, overrides sticky
	  *@return action_id
	 */
	int insertAction(const QString& actionLabel, const std::function<void(const QVariant &var)> &actionFunc = nullptr);
	inline const QString &_action(const uint action_id) const
	{
		Q_ASSERT_X(action_id < m_actions.count(), "TPMessage::_action", "action_id out of range");
		return m_actions.at(action_id);
	}
	inline QString action(const int action_id) const
	{
		if (action_id >= 0 && action_id < m_actions.count()) {
			const QString &actionText{m_actions.at(action_id)};
			return static_cast<int>(actionText.at(actionText.length()-1).toLatin1()) > 31 ? actionText : actionText.chopped(1);
		}
		return QString{};
	}
	inline QStringList actions() const { return m_actions; }
	void removeAction(const int action_id);
	Q_INVOKABLE void execAction(const int action_id);

	/**
	  *@brief Used to store information across classes
	  *@param data Anything that QVariant can handle
	  *@param action_id Associate data with an action or not
	  *@return The index of the inserted data(which will be equal to action_id if it's not -1)
	 */
	uint insertData(const QVariant &data, const int action_id = -1);
	inline const QVariant &_data(const uint data_id) const
	{
		Q_ASSERT_X(data_id < m_data.count(), "TPMessage::_data", "data_id out of range");
		return m_data.at(data_id);
	}
	inline QVariant data(const int data_id) const
	{
		if (data_id >= 0 && data_id < m_data.count())
			return m_data.at(data_id);
		return QVariant{};
	}
	void removeData(const int data_id);

signals:
	void actionTriggered(const int action_id);
	void dataChanged(const uint field);
	void killMessage(TPMessage *message);

private:
	qsizetype m_id{-1};
	bool m_plugged{false}, m_sticky{false}, m_timedExpiration{false};
	QString m_title, m_text, m_icon, m_extraInfoText, m_extraInfoImage;
	TPFilePath *m_tpFilePath{nullptr};
	QStringList m_actions;
	QVariantList m_data;
	QDateTime m_ctime;
	QTimer *m_timer{nullptr};
	QList<std::function<void(const QVariant &var)>> m_actionFuncs;
};

