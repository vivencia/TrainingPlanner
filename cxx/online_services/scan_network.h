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

class tpScanNetwork : public QObject
{

Q_OBJECT

static constexpr size_t PACKET_SIZE{64};
static constexpr std::chrono::seconds MAX_WAIT_TIME{1};

public:
	explicit inline tpScanNetwork(const QString &base_ip) : QObject{nullptr} {}

	void scan(const QString &base_ip)
	{
		for (uint i{1}; i <= 254; i++) {
			const QString &ip{base_ip + QString::number(i)};
			if ( QThread::currentThread()->isInterruptionRequested() )
				return;
			if (ping(ip))
				emit addressReachable(ip);
			// Add a small delay between pings to avoid overwhelming the network
			QThread::currentThread()->sleep(std::chrono::milliseconds(10));
		}
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

			if (::sendto(sockfd, packetdata, PACKET_SIZE, 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) > 0)
			{
				if (::recvfrom(sockfd, packetdata, sizeof(packetdata), 0, nullptr, nullptr) > 0)
					ping_success = true;
			}
			::close(sockfd);
		}
		return ping_success;
	}
};
