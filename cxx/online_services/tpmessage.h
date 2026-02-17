#pragma once

#include <QObject>
#include <QQmlEngine>

enum TPMessageFields {
	TPMESSAGE_FIELD_ID,
	TPMESSAGE_FIELD_TEXT,
	TPMESSAGE_FIELD_ICON,
	TPMESSAGE_FIELD_DATE,
	TPMESSAGE_FIELD_TIME,
	TPMESSAGE_FIELD_EXTRA_INFO,
	TPMESSAGE_FIELD_EXTRA_ICON,
	TPMESSAGE_FIELD_ACTIONS,
	TPMESSAGE_FIELD_STICKY,
};

class TPMessage : public QObject
{

Q_OBJECT
QML_ELEMENT

public:
	inline explicit TPMessage() : QObject{nullptr}, m_id{-1}, m_plugged{false}, m_sticky{false} {}
	inline TPMessage(QString &&displayText, QString &&iconSource) : QObject{nullptr}
	{
		setDisplayText(std::move(displayText));
		setIconSource(std::move(iconSource));
	}

	inline qsizetype id() const { return m_id; }
	inline void setId(const qsizetype id) { m_id = id; emit dataChanged(TPMESSAGE_FIELD_ID); }

	inline const QString &_displayText() const { return m_text; }
	//32 is the space character. All the separators are 31 or less. Good output is 33 or greater
	inline QString displayText() const { return static_cast<int>(m_text.last(1).at(0).toLatin1()) > 32 ? m_text : m_text.chopped(1); }
	inline void setDisplayText(QString &&new_text) { m_text = std::move(new_text); emit dataChanged(TPMESSAGE_FIELD_TEXT); }

	inline const QString &_iconSource() const { return m_icon; }
	inline QString iconSource() const { return m_icon; }
	inline void setIconSource(QString &&new_icon) { m_icon = std::move(new_icon); emit dataChanged(TPMESSAGE_FIELD_ICON); }

	QString date() const;
	QString time() const;

	inline const QString &extraInfoLabel() const { return m_extraInfoLabel; }
	inline void setExtraInfoLabel(const QString &new_label) { m_extraInfoLabel = new_label; emit dataChanged(TPMESSAGE_FIELD_EXTRA_INFO); }
	inline const QString &extraInfoImage() const { return m_extraInfoImage; }
	inline void setExtraInfoImage(const QString &new_image) { m_extraInfoImage = new_image; emit dataChanged(TPMESSAGE_FIELD_EXTRA_ICON); }

	inline const bool plugged() const { return m_plugged; }
	void plug();

	inline const bool sticky() const { return m_sticky; }
	inline void setSticky(const bool sticky) { m_sticky = sticky; emit dataChanged(TPMESSAGE_FIELD_STICKY); }

	inline const bool hasActions() const { return !m_actions.isEmpty(); }

	/**
	  *@param message_id
	  *@param action_name
	  *@param remove if Specified, overrides sticky
	  *@return action_id
	 */
	int insertAction(const QString& actionLabel, const std::function<void(const QVariant &var)> &actionFunc = nullptr,
					 std::optional<bool> remove = std::nullopt);
	inline const QString &_action(const uint action_id) const
	{
		Q_ASSERT_X(action_id < m_actions.count(), "TPMessage::_action", "action_id out of range");
		return m_actions.at(action_id);
	}
	inline QString action(const int action_id) const
	{
		if (action_id >= 0 && action_id < m_actions.count())
		{
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
	void actionTriggered(const int action_id, const std::optional<bool> remove_message);
	void dataChanged(const uint field);

private:
	qsizetype m_id;
	bool m_plugged, m_sticky;
	QString m_text;
	QString m_icon;
	QString m_extraInfoImage, m_extraInfoLabel;
	QStringList m_actions;
	QVariantList m_data;
	QDateTime m_ctime;
	QList<std::function<void(const QVariant &var)>> m_actionFuncs;
};

