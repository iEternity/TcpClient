#pragma once
#include <string>
#include <WinSock2.h>
#include <thread>
#include <memory>
#include <functional>
#include <atomic>
#include "CountDownLatch.h"

namespace net
{
	class TcpClient;
	using ConnectCallback = std::function<void(const std::string& addr, bool isUp)>;
	using MessageCallback = std::function<void(const char*, size_t)>;
	using ErrorCallback = std::function<void(const TcpClient&)>;

class TcpClient
{
public:
	TcpClient(const TcpClient&) = delete;
	TcpClient& operator=(const TcpClient&) = delete;

	TcpClient(const std::string& ip, unsigned port);

	~TcpClient();

	void setConnectCallback(const ConnectCallback& cb)	{ connectCallback_ = cb; }
	void setConnectCallback(ConnectCallback&& cb)		{ connectCallback_ = std::move(cb); }
	void setMessageCallback(const MessageCallback& cb)	{ messageCallback_ = cb; }
	void setMessageCallback(MessageCallback&& cb)		{ messageCallback_ = std::move(cb); }
	void setErrorCallback(const ErrorCallback& cb)		{ errorCallback_   = cb; }
	void setErrorCallback(ErrorCallback&& cb)			{ errorCallback_   = std::move(cb); }

	int send(const char* data, size_t len);
	int send(const std::string& msg);
	void disconnect();

	std::string toIpPort();

private:
	void initWinSock();
	void initNewSocket();
	static void handleError(const char* message);
	void start();
	void connect();
	void recvThreadFunc();

private:
	static const int kBufferSize = 1024*64;

	std::string		ip_;
	unsigned		port_;
	sockaddr_in		servAddr_;
	SOCKET			socket_;
	char			recvBuffer_[kBufferSize];
	char			sendBuffer_[kBufferSize];

	std::unique_ptr<std::thread> recvThread_;

	ConnectCallback connectCallback_;
	MessageCallback messageCallback_;
	ErrorCallback errorCallback_;
	std::atomic<bool> connected_;
	CountDownLatch latch_;
};

}