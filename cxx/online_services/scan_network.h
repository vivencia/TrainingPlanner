#pragma once

#include <QDebug>
#include <QObject>
#include <QThread>

extern "C" {
	#include <arpa/inet.h>
	#include <chrono>
	#include <netinet/in.h>
	#include <netinet/ip_icmp.h>
	#include <sys/socket.h>
	#include <unistd.h>
}

using namespace Qt::Literals::StringLiterals;
constexpr QLatin1StringView base_ip{"192.168.10."_L1};

class tpScanNetwork : public QObject
{

Q_OBJECT

static constexpr std::chrono::seconds MAX_WAIT_TIME{1};

public:
	explicit inline tpScanNetwork() : QObject{nullptr} {}

	void scan(const QString &last_working_address)
	{
		for (int n{0}, server_id{0}; n <= 255; ++n)
		{
			if (!last_working_address.isEmpty())
			{
				server_id = last_working_address.section('.', 4, 4).toInt();
				if (server_id == 256)
					server_id = 0;
			}
			const QString &ip{base_ip + QString::number(server_id++)};

			if (QThread::currentThread()->isInterruptionRequested())
			{
				QThread::currentThread()->quit();
				return;
			}
			#ifndef QT_NO_DEBUG
			qDebug() << "Pinging " << ip;
			#endif
			if (ping(ip))
				emit addressReachable(ip);
			// Add a small delay between pings to avoid overwhelming the network
			QThread::currentThread()->sleep(std::chrono::milliseconds(10));
		}
		emit addressReachable("None"_L1);
		QThread::currentThread()->quit();
	}

signals:
	void addressReachable(const QString &ip);

private:
	bool ping(const QString &ip_addr) const
	{
		bool ping_success{false};
		const int sockfd{::socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP)};
		if (sockfd >= 0)
		{
			::sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = ::inet_addr(ip_addr.toLatin1().constData());

			// Initialize the ICMP header
			struct ::icmphdr icmp_hdr;
			::memset(&icmp_hdr, 0, sizeof(icmp_hdr));
			icmp_hdr.type = ICMP_ECHO;
			icmp_hdr.un.echo.id = 1234;
			icmp_hdr.un.echo.sequence = 1;

			::timeval tv{};
			tv.tv_sec = MAX_WAIT_TIME.count();
			tv.tv_usec = 0;
			::setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

			// Initialize the packet data (header and payload)
			char packetdata[sizeof(icmp_hdr) + 5];
			::memcpy(packetdata, &icmp_hdr, sizeof(icmp_hdr));
			::memcpy(packetdata + sizeof(icmp_hdr), "12345", 5);

			if (::sendto(sockfd, packetdata, sizeof(packetdata), 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) > 0)
			{
				if (::recvfrom(sockfd, packetdata, sizeof(packetdata), 0, nullptr, nullptr) > 0)
					ping_success = true;
			}
			::close(sockfd);
		}
		return ping_success;
	}
};
