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
Q_PROPERTY(int childCount READ childCount NOTIFY childCountChanged FINAL)
Q_PROPERTY(int row READ row NOTIFY rowChanged FINAL)
Q_PROPERTY(uint depth READ depth CONSTANT FINAL)
Q_PROPERTY(uint actionCount READ actionCount NOTIFY actionCountChanged FINAL)
Q_PROPERTY(qreal delegateHeight READ delegateHeight NOTIFY delegateHeightChanged FINAL)
Q_PROPERTY(bool sticky READ sticky WRITE setSticky NOTIFY stickyChanged BINDABLE bindableSticky FINAL)
Q_PROPERTY(bool hasIcon READ hasIcon NOTIFY hasIconChanged BINDABLE bindableHasIcon FINAL)
Q_PROPERTY(bool hasExtraImage READ hasExtraImage NOTIFY hasExtraImageChanged BINDABLE bindableHasExtraImage FINAL)
Q_PROPERTY(bool collapsed READ collapsed WRITE setCollapsed NOTIFY collapsedChanged BINDABLE bindableCollapsed FINAL)
Q_PROPERTY(TPMessage::MessageType type READ type CONSTANT FINAL)
Q_PROPERTY(QString title READ title NOTIFY titleChanged BINDABLE bindableTitle FINAL)
Q_PROPERTY(QString text READ text NOTIFY textChanged BINDABLE bindableText FINAL)
Q_PROPERTY(QString icon READ icon NOTIFY iconChanged BINDABLE bindableIcon FINAL)
Q_PROPERTY(QString dateTime READ dateTime NOTIFY dateTimeChanged BINDABLE bindableDateTime FINAL)
Q_PROPERTY(QString extraInfo READ extraInfo NOTIFY extraInfoChanged BINDABLE bindableExtraInfo FINAL)
Q_PROPERTY(QString extraImage READ extraImage NOTIFY extraImageChanged BINDABLE bindableExtraImage FINAL)
Q_PROPERTY(TPFileOps* fileOps READ fileOps CONSTANT FINAL)
Q_PROPERTY(TPMessage* parentMessage READ parentMessage CONSTANT FINAL)
Q_PROPERTY(QQuickItem* actionsLayout READ actionsLayout NOTIFY actionsLayoutChanged FINAL)
Q_PROPERTY(QQuickItem* fileViewer READ fileViewer NOTIFY fileViewerChanged FINAL)

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
	Q_ENUM(ActionType)

	enum MessageType {
		MT_PHANTON		= 1U << 0,
		MT_TOPLEVEL		= 1U << 1,
		MT_CHAT			= 1U << 2,
		MT_TPMESSAGE	= 1U << 3,
	};
	Q_ENUM(MessageType)

	enum MessageComponent {
		MC_TEXT,
		MC_ACTIONS,
		MC_FILEOPS,
		MC_COUNT
	};
	Q_ENUM(MessageComponent)

	inline explicit TPMessage(TPMessage *parent_message = nullptr)
		: QObject{nullptr}
		, m_parentMessage{parent_message}, m_depth{parent_message ? parent_message->depth() + 1 : 0}
		, m_collapsed{false} {}
	~TPMessage();

	inline const uint childCount() const { return m_children.size(); }
	inline TPMessage *child(const int row) { return row >= 0 && row < m_children.size() ? m_children.at(row) : this; }
	inline const QList<TPMessage*> &children() const { return m_children; }
	inline const TPMessage *parentMessage() const { return m_parentMessage; }
	inline TPMessage *parentMessage() { return m_parentMessage; }
	TPMessage *findChild(const QVariant &value, const TPMessageFields field) const;
	void insertChild(TPMessage *child, const uint row);
	void removeChild(TPMessage *child);
	void removeAllChildren();

	inline const uint id() const { return m_id; }
	inline void setId(const uint id) { m_id = id; setObjectName(QString::number(id));}

	int row() const;
	inline uint depth() const { return m_depth; }

	inline const QString &userid() const { return m_userid; }
	inline void setUserId(QString &&userid) { m_userid = std::forward<QString>(userid);  }
	inline void setUserId(const QString &userid) { m_userid = userid; }

	inline const MessageType type() const { return m_type; }
	inline void setType(const MessageType type) { m_type = type; }

	inline const QString &title() const { return m_title.value(); }
	inline QBindable<QString> bindableTitle() { return &m_title; }
	inline void setTitle(QString &&new_title) { m_title = std::forward<QString>(new_title); }

	inline const QString &text() const { return m_text.value(); }
	inline QBindable<QString> bindableText() const { return &m_text; }
	inline void setText(QString &&new_text)
	{
		m_text = std::forward<QString>(new_text);
		m_componentHeight[MC_TEXT] = 0.0;
		emit delegateHeightChanged();
	}

	inline QString icon() const { return m_icon.value(); }
	inline QBindable<QString> bindableIcon() { return &m_icon; }
	inline void setIcon(QString &&new_icon)
	{
		m_hasIcon = new_icon.length() > 0;
		m_icon = std::forward<QString>(new_icon);
	}
	inline bool hasIcon() const { return m_hasIcon.value(); }
	inline QBindable<bool> bindableHasIcon() { return &m_hasIcon; }

	inline TPFileOps *fileOps() const { return m_fileOps; }
	void setFileName(const QString &filename);

	inline const QString &extraInfo() const { return m_extraInfo.value(); }
	inline QBindable<QString> bindableExtraInfo() { return &m_extraInfo; }
	inline void setExtraInfo(QString &&extra_info) { m_extraInfo = std::forward<QString>(extra_info); }

	inline QString extraImage() const { return m_extraImage.value(); }
	inline QBindable<QString> bindableExtraImage() { return &m_extraImage; }
	inline void setExtraImage(QString &&new_image)
	{
		m_hasExtraImage = new_image.length() > 0;
		m_extraImage = std::forward<QString>(new_image);
	}
	inline bool hasExtraImage() const { return m_hasExtraImage.value(); }
	inline QBindable<bool> bindableHasExtraImage() { return &m_hasExtraImage; }

	inline const QString &dateTime() const { return m_dateTime.value(); }
	inline QBindable<QString> bindableDateTime() { return &m_dateTime; }
	void setDateTime(const QDateTime &ctime);

	inline const QString &encodedMessage() const { return m_encodedMessage; }
	inline void setEncodedMessage(QString &&encoded_message) { m_encodedMessage = std::forward<QString>(encoded_message); }

	const QDateTime &expiration() const { return m_expirationTime; }
	void setExpiration(QDateTime &&date_time = QDateTime{});
	inline const bool isExpirable() const { return m_expirationTime.isValid(); }

	inline const qreal delegateHeight() const
	{
		qreal d_height{0};
		for (uint i{0}; i < MC_COUNT; ++i)
			d_height += m_componentHeight[i];
		return d_height;
	}

	inline const bool sticky() const { return m_sticky.value(); }
	inline QBindable<bool> bindableSticky() { return &m_sticky; }
	inline void setSticky(const bool sticky) { m_sticky = sticky; }

	inline const bool collapsed() const { return m_collapsed.value(); }
	inline QBindable<bool> bindableCollapsed() { return &m_collapsed; }
	inline void setCollapsed(const bool collapsed)
	{
		m_collapsed = collapsed;
		if (m_actionsLayout)
			m_actionsLayout->setProperty("visible", collapsed);
		if (m_fileViewer)
			m_fileViewer->setProperty("visible", collapsed);
		emit delegateHeightChanged();
	}

	inline decltype(auto) actionCount() const { return m_actions.count(); }
	int insertAction(QString &&label, const ActionType action_type, int index = -1,
					const std::function<QVariant(const QVariant &)> &func = nullptr, const bool setup_actions = false);
	Q_INVOKABLE inline QString actionLabel(const uint action_id) const
	{
		return action_id < m_actions.count() ? m_actions.at(action_id).label : QString{};
	}
	inline void changeActionLabel(const uint action_id, QString &&new_label)
	{
		if (action_id < m_actions.count()) {
			if (new_label != m_actions.at(action_id).label) {
				m_actions[action_id].label = std::forward<QString>(new_label);
				emit actionChanged(action_id);
			}
		}
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
		if (action_id >= 0 && action_id < m_actions.count()) {
			setupActionsLayout(false, action_id == m_actions.count(), action_id < m_actions.count());
			m_actions.remove(action_id);
			emit actionCountChanged();
		}
	}

	inline QVariant generalPurposeData(const int key) const { return m_generalPurposeData.value(key); }
	inline void setGeneralPurposeData(const int key, QVariant &&gpd)
	{
		m_generalPurposeData.insert(key, std::forward<QVariant>(gpd));
	}

	Q_INVOKABLE inline void setMessageComponentHeight(const MessageComponent component, const qreal new_height)
	{
		if (m_componentHeight[component] == 0 && new_height != 0) {
			m_componentHeight[component] = new_height;
			if ((m_text.value().length() > 0) == (m_componentHeight[MC_TEXT] != 0)) {
				if ((actionCount() > 0) == (m_componentHeight[MC_ACTIONS] != 0)) {
					if ((m_fileOps != nullptr) == (m_componentHeight[MC_FILEOPS] != 0))
						emit delegateHeightChanged();
				}
			}
		}
	}

	Q_INVOKABLE inline void setActionsLayoutParent(QQuickItem *parent)
	{
		if (parent != m_actionsLayoutParent) {
			m_actionsLayoutParent = parent;
			if (m_actionsLayout) {
				m_actionsLayout->setParentItem(parent);
				m_actionsLayoutParent->setHeight(m_actionsLayout->height());
			}
		}
	}
	Q_INVOKABLE inline void setFileViewerParent(QQuickItem *parent)
	{
		if (parent != m_fileViewerParent) {
			m_fileViewerParent = parent;
			if (m_fileViewer) {
				m_fileViewer->setParentItem(parent);
				m_fileViewerParent->setHeight(m_fileViewer->property("minimumHeight").toReal());
				QMetaObject::invokeMethod(m_fileViewer, "anchorToParent");
			}
		}
	}

	inline QQuickItem *actionsLayout() const { return m_actionsLayout; }
	inline QQuickItem *fileViewer() const { return m_fileViewer; }

signals:
	void actionEnabledChanged(const int action_id, const bool enabled);
	void killMessage();
	void actionChanged(const uint action_id);
	void childCountChanged();
	void rowChanged();
	void actionCountChanged();
	void delegateHeightChanged();
	void titleChanged();
	void iconChanged();
	void hasIconChanged();
	void textChanged();
	void extraInfoChanged();
	void extraImageChanged();
	void hasExtraImageChanged();
	void dateTimeChanged();
	void stickyChanged();
	void collapsedChanged();
	void actionsLayoutChanged();
	void fileViewerChanged();

public slots:
	void popupSizeChanged(const qreal w_ratio, const qreal h_ratio);
	inline void execAction(const int action_id, const QVariant &data)
	{
		if (action_id >= 0 && action_id < m_actions.count())
			m_actions.at(action_id).func(data);
	}

private:
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_title, &TPMessage::titleChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_icon, &TPMessage::iconChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_text, &TPMessage::textChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_extraInfo, &TPMessage::extraInfoChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_extraImage, &TPMessage::extraImageChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, QString, m_dateTime, &TPMessage::dateTimeChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, bool, m_hasIcon, &TPMessage::iconChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, bool, m_hasExtraImage, &TPMessage::hasExtraImageChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, bool, m_sticky, &TPMessage::stickyChanged)
	Q_OBJECT_BINDABLE_PROPERTY(TPMessage, bool, m_collapsed, &TPMessage::collapsedChanged)

	QList<TPMessage*> m_children;
	TPMessage *m_parentMessage{nullptr};
	uint m_id{0}, m_depth{0};
	qreal m_componentHeight[MC_COUNT]{0.0};
	MessageType m_type;
	QString m_userid, m_encodedMessage;
	QDateTime m_expirationTime;
	TPFileOps *m_fileOps{nullptr};
	QTimer *m_timer{nullptr};

	struct st_Action {
		QString label;
		ActionType type{AT_BUTTON};
		std::function<QVariant(const QVariant &data)> func{nullptr};
		bool enabled{true};
		QQuickItem *qml_item{nullptr};
	};
	QList<st_Action> m_actions;
	QHash<int,QVariant> m_generalPurposeData;
	QQuickItem *m_actionsLayout{nullptr}, *m_fileViewer{nullptr}, *m_actionsLayoutParent{nullptr}, *m_fileViewerParent{nullptr};
	QVariantMap m_fileViewerProperties;

	static QQmlComponent *_actionsLayoutComponent, *_fileViewerComponent;
	bool isChild(TPMessage *msg) const;
	void createActionsLayout();
	void setupActionsLayout(const bool append = true, const bool remove_last = false, const bool reset = false);
	void createActionItem(const uint action_index);
	void createFileViewer();
};
