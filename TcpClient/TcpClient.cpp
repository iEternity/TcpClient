#include <iostream>
#include "TcpClient.h"

using namespace net;
using namespace std;

void defaultOnMessage(const char* data, size_t len)
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
void defaultOnError(const TcpClientPtr& client)
{
}

TcpClient::TcpClient(const std::string& ip, unsigned port):
	ip_(ip),
	port_(port),
	messageCallback_(defaultOnMessage),
	connectCallback_(defaultOnConnect),
	errorCallback_(defaultOnError),
	connected_(false)
{
	::memset(sendBuffer_, 0, sizeof sendBuffer_);
	::memset(recvBuffer_, 0, sizeof recvBuffer_);

	initWinSock();

	socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
	if (socket_ == INVALID_SOCKET) handleError("socket() error!");

	memset(&servAddr_, 0, sizeof(servAddr_));
	servAddr_.sin_family		= AF_INET;
	servAddr_.sin_addr.s_addr	= inet_addr(ip.data());
	servAddr_.sin_port			= htons(port);
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

void TcpClient::handleError(const char* message)
{
	std::cerr << "net::TcpClient:";
	std::cerr << message << endl;
	exit(1);
}

void TcpClient::connect()
{
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
	}
}

void TcpClient::reconnect()
{

}

int TcpClient::send(const char* data, size_t len)
{
	if (!connected_)
	{
		start();
	}
	
	return ::send(socket_, data, len, 0);
}

void TcpClient::start()
{
	connect();
	recvThread_.reset(new std::thread(std::bind(&TcpClient::recvThreadFunc, this)));
}

std::string TcpClient::toIpPort()
{
	return ip_ + ":" + std::to_string(port_);
}

void TcpClient::recvThreadFunc()
{
	int recvLen = 0;
	while (true)
	{
		recvLen = ::recv(socket_, recvBuffer_, sizeof recvBuffer_, 0);
		if (recvLen <= 0)
		{
			connected_ = false;
			connectCallback_(toIpPort(), false);
			break;
		}
		messageCallback_(recvBuffer_, recvLen);
	}
}