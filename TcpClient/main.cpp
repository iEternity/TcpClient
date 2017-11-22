#include <iostream>
#include "TcpClient.h"
#include "Utility.h"

using namespace std;
using namespace net;

void onMessage(const char* data, size_t len)
{
	cout << utility::UTF8_To_GBK(string(data, len)) << endl;
}

int main()
{
	//TcpClient client("192.168.8.63", 12345);
	TcpClient client("127.0.0.1", 31629);
	client.setMessageCallback(onMessage);
	client.start();

	std::string line;
	while (getline(cin,line))
	{
		client.send(line.c_str(), line.size());
	}
}