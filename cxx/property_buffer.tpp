#include <QList>
#include <QTimer>

using bufferPropertyId = char[9];

constexpr uint16_t fnv1a_fasthash(const bufferPropertyId &s) {
	uint16_t hash{0x9dc5};
	for (const auto c : s) {
		hash ^= static_cast<uint16_t>(c);
		hash *= 0x0193;
	}
	return hash;
}

template<typename T>
struct st_QmlPropertyChangesBuffer {
	uint16_t id{0};
	QTimer *timer{nullptr};
	T property_value;
	std::function<void(const T&)> finalPropertyFn{nullptr};
};

std::vector<uint16_t> __bufferProperties;

template<typename T>
void bufferProperty(const bufferPropertyId &id_str, const T &property_value,
													const std::function<void(const T&)>fn, int msecs = 500)
{
	uint16_t id{fnv1a_fasthash(id_str)};
	if (std::find(__bufferProperties.cbegin(), __bufferProperties.cend(), id) != __bufferProperties.cend())
		return;
	__bufferProperties.push_back(id);

	auto p_buffer{std::make_shared<st_QmlPropertyChangesBuffer<T>>()};
	p_buffer->id = id;
	p_buffer->property_value = property_value;
	p_buffer->finalPropertyFn = fn;
	p_buffer->timer = new QTimer{};
	p_buffer->timer->setInterval(msecs);
	p_buffer->timer->callOnTimeout([p_buffer] () {
		p_buffer->finalPropertyFn(p_buffer->property_value);
		__bufferProperties.erase(std::find(__bufferProperties.cbegin(), __bufferProperties.cend(), p_buffer->id));
		delete p_buffer->timer;
	});
	p_buffer->timer->start();
}
