#pragma once

#include <QObject>
#include <QQmlEngine>

QT_FORWARD_DECLARE_CLASS(TPMessagesManager)

class TPMessage : public QObject
{

Q_OBJECT
QML_ELEMENT

/*Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged FINAL)
Q_PROPERTY(QString displayText READ displayText WRITE setDisplayText NOTIFY displayTextChanged FINAL)
Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged FINAL)
Q_PROPERTY(QString date READ date CONSTANT)
Q_PROPERTY(QString time READ time CONSTANT)
Q_PROPERTY(QString extraInfoLabel READ extraInfoLabel WRITE setExtraInfoLabel NOTIFY extraInfoChanged FINAL)
Q_PROPERTY(QString extraInfoImage READ extraInfoImage WRITE setExtraInfoImage NOTIFY extraInfoChanged FINAL)
Q_PROPERTY(QStringList actions READ actions NOTIFY actionsChanged FINAL)
Q_PROPERTY(bool sticky READ sticky WRITE setSticky NOTIFY stickyChanged FINAL)
Q_PROPERTY(bool hasActions READ hasActions NOTIFY hasActionsChanged FINAL)*/
//TODO: remove the signals

public:
	inline explicit TPMessage(TPMessagesManager *parent) : QObject{nullptr}, m_parent{parent}, m_id{-1},
									m_plugged{false}, m_autodelete{true}, m_sticky{false} {}
	inline TPMessage(QString &&displayText, QString &&iconSource, TPMessagesManager *parent)
			: QObject{nullptr}, m_parent{parent}, m_id{-1}, m_plugged{false}, m_autodelete{true}, m_sticky{false}
	{
		setDisplayText(std::move(displayText), false);
		setIconSource(std::move(iconSource), false);
	}

	inline int id() const { return m_id; }
	inline void setId(const int id) { m_id = id; emit idChanged(); }

	inline const QString &_displayText() const { return m_text; }
	//32 is the space character. All the separators are 31 or less. Good output is 33 or greater
	inline QString displayText() const { return static_cast<int>(m_text.last(1).at(0).toLatin1()) > 32 ? m_text : m_text.chopped(1); }
	void setDisplayText(QString &&new_text, const bool emitSignal = true)
	{
		m_text = std::move(new_text);
		if (emitSignal)
			emit displayTextChanged();
	}

	inline const QString &_iconSource() const { return m_icon; }
	inline QString iconSource() const { return m_icon; }
	void setIconSource(QString &&new_icon, const bool emitSignal = true)
	{
		m_icon = std::move(new_icon);
		if (emitSignal)
			emit iconSourceChanged();
	}

	QString date() const;
	QString time() const;

	inline const QString &extraInfoLabel() const { return m_extraInfoLabel; }
	inline void setExtraInfoLabel(const QString &new_label) { m_extraInfoLabel = new_label; emit extraInfoChanged(); }
	inline const QString &extraInfoImage() const { return m_extraInfoImage; }
	inline void setExtraInfoImage(const QString &new_image) { m_extraInfoImage = new_image; emit extraInfoChanged(); }

	inline const bool plugged() const { return m_plugged; }
	void plug();

	inline const bool autoDelete() const { return m_autodelete; }
	inline void setAutoDelete(const bool autodelete) { m_autodelete = autodelete; }

	inline const bool sticky() const { return m_sticky; }
	inline void setSticky(const bool sticky) { m_sticky = sticky;  emit stickyChanged(); }

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
	void idChanged();
	void displayTextChanged();
	void iconSourceChanged();
	void extraInfoChanged();
	void actionsChanged();
	void stickyChanged();
	void hasActionsChanged();

private:
	TPMessagesManager *m_parent;
	int m_id;
	bool m_plugged, m_autodelete, m_sticky;
	QString m_text;
	QString m_icon;
	QString m_extraInfoImage, m_extraInfoLabel;
	QStringList m_actions;
	QVariantList m_data;
	QDateTime m_ctime;
	QList<std::function<void(const QVariant &var)>> m_actionFuncs;

	void setPlugged(const bool plugged);
	friend class TPMessagesManager;
};

