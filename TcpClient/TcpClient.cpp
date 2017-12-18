#include <iostream>
#include "TcpClient.h"

using namespace net;
using namespace std;

void defaultOnMessage(const char* data, size_t size)
{

}
void defaultOnConnect(const std::string& addr, bool isUp)
{
	if (isUp)
	{
		cout << "Connect to server: " << addr << " success!" << endl;
	}
	else
	{
		cout << "Disconnect from server: " << addr << endl;
	}
}
void defaultOnError(const TcpClient& client)
{
	
}

TcpClient::TcpClient(const std::string& ip, unsigned port):
	ip_(ip),
	port_(port),
	socket_(NULL),
	messageCallback_(defaultOnMessage),
	connectCallback_(defaultOnConnect),
	errorCallback_(defaultOnError),
	connected_(false),
	latch_(1)
{
	::memset(sendBuffer_, 0, sizeof sendBuffer_);
	::memset(recvBuffer_, 0, sizeof recvBuffer_);

	initWinSock();

	memset(&servAddr_, 0, sizeof(servAddr_));
	servAddr_.sin_family		= AF_INET;
	servAddr_.sin_addr.s_addr	= inet_addr(ip.data());
	servAddr_.sin_port			= htons(port);

	start();
}

TcpClient::~TcpClient()
{
	if (recvThread_) recvThread_->join();
	
	::closesocket(socket_);
	::WSACleanup();
}

void TcpClient::initWinSock()
{
	WSADATA data;
	int ret = ::WSAStartup(MAKEWORD(2, 2), &data);

	if (ret != 0)
	{
		handleError("WSAStartup failed!");
	}
}

void TcpClient::initNewSocket()
{
	if (socket_) ::closesocket(socket_);

	socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_ == INVALID_SOCKET) handleError("socket() error!");

	bool optVal = 1;
	::setsockopt(socket_, IPPROTO_TCP, TCP_NODELAY, (const char*)(&optVal), sizeof(optVal));
}

void TcpClient::start()
{
	latch_.reset(1);

	connect();

	if (recvThread_)
	{
		recvThread_->join();
	}
	recvThread_.reset(new std::thread(std::bind(&TcpClient::recvThreadFunc, this)));
}



void TcpClient::connect()
{
	initNewSocket();

	int ret = ::connect(socket_, (sockaddr*)&servAddr_, sizeof(servAddr_));
	if (ret == SOCKET_ERROR)
	{
		char buf[64];
		sprintf_s(buf, sizeof(buf), "Can not connect to server: %s", toIpPort().c_str());
		handleError(buf);
	}
	else
	{
		connected_ = true;
		connectCallback_(toIpPort(), true);
		latch_.countDown();
	}
}

void TcpClient::recvThreadFunc()
{
	latch_.wait();
	int recvLen = 0;
	while (true)
	{
		recvLen = ::recv(socket_, recvBuffer_, sizeof(recvBuffer_), 0);
		if (recvLen == 0)
		{
			connected_ = false;
			connectCallback_(toIpPort(), false);
			break;
		}
		else if (recvLen < 0)
		{
			//handleError("recvThreadFunc:recv error!");
			errorCallback_(*this);
			break;
		}

		messageCallback_(recvBuffer_, recvLen);
	}
}

int TcpClient::send(const char* data, size_t len)
{
	if (!connected_)
	{
		start();
	}
	
	return ::send(socket_, data, len, 0);
}

int TcpClient::send(const std::string& msg)
{
	if (!connected_)
	{
		start();
	}

	return ::send(socket_, msg.data(), msg.size(), 0);
}

int TcpClient::read(char* buf, size_t size)
{
	int recvLen = ::recv(socket_, buf, size, 0);
	if (recvLen == 0)
	{
		connected_ = false;
		connectCallback_(toIpPort(), false);
	}
	else if (recvLen < 0)
	{
		handleError("recvThreadFunc:recv error!");
		errorCallback_(*this);
	}

	return recvLen;
}

void TcpClient::disconnect()
{
	if (connected_)
	{
		::closesocket(socket_);
	}
}

std::string TcpClient::toIpPort()
{
	return ip_ + ":" + std::to_string(port_);
}

void TcpClient::handleError(const char* message)
{
	std::cerr << "net::TcpClient: ";
	std::cerr << message << endl;
}