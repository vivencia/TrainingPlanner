#pragma once

#include "tpfileops.h"

#include <QDateTime>
#include <QObject>
#include <QVariant>
#include <QtQml/qqml.h>

QT_FORWARD_DECLARE_CLASS(QTimer)

class TPMessage : public QObject
{

Q_OBJECT
QML_ELEMENT
QML_VALUE_TYPE(TPMessage)

Q_PROPERTY(int id READ id CONSTANT FINAL)
Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged BINDABLE bindableTitle FINAL)
Q_PROPERTY(QString text READ text CONSTANT FINAL)
Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged BINDABLE bindableIcon FINAL)
Q_PROPERTY(QString dateTime READ dateTime CONSTANT FINAL)
Q_PROPERTY(QString extraInfo READ extraInfo WRITE setExtraInfo NOTIFY extraInfoChanged BINDABLE bindableExtraInfo FINAL)
Q_PROPERTY(QString extraImage READ extraImage CONSTANT FINAL)
Q_PROPERTY(TPFileOps* fileOps READ fileOps CONSTANT FINAL)
Q_PROPERTY(bool sticky READ sticky CONSTANT FINAL)
Q_PROPERTY(int actionCount READ actionCount CONSTANT FINAL)

public:
	enum TPMessageFields {
		FIELD_ID,
		FIELD_ROW,
		FIELD_USERID,
		FIELD_TYPE,
		FIELD_TITLE,
		FIELD_TEXT,
		FIELD_ICON,
		FIELD_DATETIME,
		FIELD_FILE,
		FIELD_EXTRA_INFO,
		FIELD_EXTRA_ICON,
		FIELD_ACTIONS,
		FIELD_EXPIRATION,
		FIELD_STICKY,
	};

	enum ActionType {
		AT_NONE,
		AT_BUTTON,
		AT_RADIO,
		AT_CHECKBOX,
	};
	Q_ENUM(ActionType);

	inline explicit TPMessage(TPMessage *parent_message = nullptr)
		: QObject{nullptr}
		, m_parentMessage{parent_message}
		{}
	~TPMessage();

	inline const int8_t depth() const { return m_depth; }
	inline const uint childCount() const { return m_children.count(); }
	inline TPMessage *child(const int row) { return row >= 0 && row < m_children.count() ? m_children.at(row) : this; }
	inline const QList<TPMessage*> &children() const { return m_children; }
	inline const TPMessage *parent() const { return m_parentMessage; }
	inline TPMessage *parent() { return m_parentMessage; }
	TPMessage *findChild(const QVariant &value, const TPMessageFields field) const;
	void insertChild(TPMessage *child, int row = - 1);
	void removeChild(TPMessage *child);
	void remove(const QLatin1StringView &exclude_type = QLatin1StringView{}, const bool remove_self_if_no_children = true);

	inline const uint id() const { return m_id; }
	inline void setId(const uint id) { m_id = id; }

	inline const uint16_t row() const { return m_row; }
	inline void setRow(uint16_t row) { m_row = row; }

	inline const QString &userid() const { return m_userid; }
	inline void setUserId(QString &&userid) { m_userid = std::forward<QString>(userid);  }

	inline const QLatin1StringView type() const { return m_type; }
	inline void setType(const QLatin1StringView &type) { m_type = type; }

	inline void setDateTime(QDateTime &&ctime) { m_dateTime = std::forward<QDateTime>(ctime); }
	QString dateTime() const;

	inline const QString &title() const { return m_title; }
	QBindable<int> bindableTitle() { return &m_title; }
	inline void setTitle(QString &&new_title) { m_title = std::forward<QString>(new_title); }

	inline const QString &text() const { return m_text; }
	inline void setText(QString &&new_text) { m_text = std::forward<QString>(new_text); }

	inline QString icon() const { return m_icon; }
	QBindable<int> bindableIcon() { return &m_icon; }
	inline void setIcon(QString &&new_icon) { m_icon = std::forward<QString>(new_icon); }

	inline TPFileOps *fileOps() const { return m_fileOps; }
	void setFileName(const QString &filename);

	inline const QString &extraInfo() const { return m_extraInfo; }
	QBindable<int> bindableExtraInfo() { return &m_extraInfo; }
	inline void setExtraInfo(QString &&extra_info) { m_extraInfo = std::forward<QString>(extra_info); }

	inline QString extraImage() const { return m_extraImage; }
	inline void setExtraImage(QString &&new_image) { m_extraImage = std::forward<QString>(new_image); }

	inline const QString &encodedMessage() const { return m_encodedMessage; }
	inline void setEncodedMessage(QString &&encoded_message) { m_encodedMessage = std::forward<QString>(encoded_message); }

	const QDateTime &expiration() const { return m_expirationTime; }
	void setExpiration(QDateTime &&date_time = QDateTime{});
	inline const bool isExpirable() const { return m_expirationTime.isValid(); }

	inline const bool sticky() const { return m_sticky; }
	inline void setSticky(const bool sticky) { m_sticky = sticky; }

	inline decltype(auto) actionCount() const { return m_actions.count(); }
	int insertAction(QString &&label, const ActionType action_type, const std::function<QVariant(const QVariant &)> &func = nullptr);
	Q_INVOKABLE inline QString actionLabel(const uint action_id) const
	{
		return action_id >= 0 && action_id < m_actions.count() ? m_actions.at(action_id).label : QString{};
	}
	Q_INVOKABLE inline const TPMessage::ActionType actionType(const int action_id) const
	{
		return action_id >= 0 && action_id < m_actions.count() ? m_actions.at(action_id).type : AT_NONE;
	}
	Q_INVOKABLE const bool actionEnabled(const int action_id) const
	{
		return action_id >= 0 && action_id < m_actions.count() ? m_actions.at(action_id).enabled : false;
	}
	Q_INVOKABLE void setActionEnabled(const int action_id, const bool enabled)
	{
		if (action_id >= 0 && action_id < m_actions.count()) {
			if (m_actions.at(action_id).enabled != enabled) {
				m_actions[action_id].enabled = enabled;
				emit actionEnabledChanged(action_id, enabled);
			}
		}
	}
	void removeAction(const int action_id)
	{
		if (action_id >= 0 && action_id < m_actions.count())
			m_actions.remove(action_id);
	}
	void execAction(const int action_id, const QVariant &data)
	{
		if (action_id >= 0 && action_id < m_actions.count())
			emit actionTriggered(action_id, m_actions.at(action_id).func(data));
	}

	QVariant generalPurposeData() && { return m_generalPurposeData; }
	const QVariant &generalPurposeData() const & { return m_generalPurposeData; }
	void setGeneralPurposeData(QVariant &&gpd) { m_generalPurposeData = std::forward<QVariant>(gpd); }

public slots:
	inline void setTitle(const QString &new_title) { m_title = new_title; }
	inline void setIcon(const QString &new_icon) { m_icon = new_icon; }
	inline void setExtraInfo(const QString &extra_info) { m_extraInfo = extra_info; }

signals:
	void actionTriggered(const int action_id, const QVariant &return_value);
	void actionEnabledChanged(const int action_id, const bool enabled);
	void killMessage(TPMessage *message);

	void titleChanged();
	void iconChanged();
	void extraInfoChanged();

private:
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_title, &TPMessage::titleChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_icon, &TPMessage::iconChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_extraInfo, &TPMessage::extraInfoChanged)

	QList<TPMessage*> m_children;
	TPMessage *m_parentMessage{nullptr};
	uint m_id{0};
	int16_t m_row{-1};
	int8_t m_depth{0};
	QLatin1StringView m_type;
	QString m_userid, m_encodedMessage, m_text, m_extraImage;
	QDateTime m_dateTime, m_expirationTime;
	TPFileOps *m_fileOps{nullptr};
	bool m_sticky{false};
	QTimer *m_timer{nullptr};
	struct st_Action {
		QString label;
		ActionType type{AT_BUTTON};
		std::function<QVariant(const QVariant &data)> func{nullptr};
		bool enabled{true};
	};
	QList<st_Action> m_actions;
	QVariant m_generalPurposeData;
};

