#include <iostream>
#include "TcpClient.h"
#include "Utility.h"

using namespace std;
using namespace net;

void onMessage(const char* data, size_t len)
{
	if (string(data, len) == "exit")
	{
		return;
	}

	auto isNeedResp = (string(data, 9) == "NEED_RESP");
	if (isNeedResp)
	{
		cout << utility::UTF8_To_GBK(string(data + 9, len-9)) << endl;
	}
	else
	{
		cout << utility::UTF8_To_GBK(string(data, len)) << endl;
	}
}

void test(int count)
{
	TcpClient client("192.168.8.63", 12345);
	client.setMessageCallback(onMessage);
	string line(3000, 'X');
	for (int i = 0; i < count; i++)
	{
		client.send(line);
		::Sleep(100);
	}
}

int main()
{
	test(1);
	/*TcpClient client("192.168.8.63", 12345);
	client.setMessageCallback(onMessage);

	std::string line;
	while (getline(cin,line))
	{
		client.send(line);
	}*/

	system("pause");
}