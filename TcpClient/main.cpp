#include <iostream>
#include <string>
#include <stdint.h>
#include "TcpClient.h"
#include "Utility.h"

using namespace std;
using namespace net;

void onMessage(const char* data, size_t size)
{
	cout << string(data, size) << endl;
}

int main(int argc, char*argv[])
{
	TcpClient echoClient("127.0.0.1", 12345);
	echoClient.setMessageCallback(onMessage);

	string line;
	while (std::getline(std::cin, line))
	{
		echoClient.send(line);
	}
}