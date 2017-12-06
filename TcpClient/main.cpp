#include <iostream>
#include <string>
#include <stdint.h>
#include "TcpClient.h"
#include "Utility.h"

using namespace std;
using namespace net;

#define StringMsg 10000
#define FileMsg   10001

string packageName;

struct BuildConfig
{
	char appName[32];
	//打包类型，0：外网正式，125：内网测试，888：外网内测
	int16_t buildType;
	char recommendID[32];
	char channel[32];
	bool isUpdateSVN;
	bool isEncrypt;
	bool isYQW;
	char gameSrc[256];
	char versionName[32];
	char versionCode[32];
};

void onMessage(const TcpClientPtr& client, int32_t request, size_t len);
BuildConfig getConfig();
void onRecvFile(const TcpClientPtr& client, int len);
void printRecvProcess(int hasReceived, int sumBytes);

int main()
{
#ifdef NDEBUG
	TcpClientPtr client = make_shared<TcpClient>("192.168.8.238", 12345);
#else
	TcpClientPtr client = make_shared<TcpClient>("192.168.8.54", 12345);
#endif
	client->setMessageCallback(onMessage);

	BuildConfig config = getConfig();
	client->send((char*)&config, sizeof(config));

	getchar();
}

BuildConfig getConfig()
{
	char currentDir[256];
	::GetCurrentDirectory(sizeof currentDir, currentDir);
	string iniPath = string(currentDir) + "\\build.ini";

	BuildConfig config;
	char abbr[32];
	::GetPrivateProfileString("build", "abbr", "", abbr, sizeof abbr, iniPath.data());
	::GetPrivateProfileString("build", "recommendID", "", config.recommendID, sizeof(config.recommendID), iniPath.data());
	::GetPrivateProfileString("build", "channel", "", config.channel, sizeof(config.channel), iniPath.data());
	::GetPrivateProfileString("build", "gameSrc", "", config.gameSrc, sizeof(config.gameSrc), iniPath.data());
	::GetPrivateProfileString("build", "versionName", "", config.versionName, sizeof(config.versionName), iniPath.data());
	::GetPrivateProfileString("build", "versionCode", "", config.versionCode, sizeof(config.versionCode), iniPath.data());

	_snprintf(config.appName, sizeof(config.appName), "%s_an", abbr);
	config.buildType	= ::GetPrivateProfileInt("build", "buildType", 125, iniPath.data());
	config.isUpdateSVN	= ::GetPrivateProfileInt("build", "isUpdateSVN", 1, iniPath.data());
	config.isEncrypt	= ::GetPrivateProfileInt("build", "isEncrypt", 0, iniPath.data());
	config.isYQW		= ::GetPrivateProfileInt("build", "isYQW", 1, iniPath.data());

	packageName = string(config.appName) + "_" + config.channel + "_" + to_string(config.buildType) + ".zip";

	return config;
}

void onMessage(const TcpClientPtr& client, int32_t request, size_t len)
{
	if (request == StringMsg)
	{
		string content;
		char buf[128*1024];

		int nRead = client->read(buf, len);
		int hasReadBytes = 0;

		while (nRead > 0)
		{
			content += string(buf, nRead);

			hasReadBytes += nRead;
			if (hasReadBytes >= len) break;

			nRead = client->read(buf, sizeof buf);
		}

		cout << content << endl;
	}
	else if (request == FileMsg)
	{
		cout << "打包完成，开始接收文件..." << endl;
		cout << "文件大小：" << (double)len / (1024 * 1024) << "Mib" << endl;
		onRecvFile(client, len);
		cout << "文件接收完成！" << endl;
	}
	else
	{
		cout << "received: " << len << "bytes" << endl;
	}
}

void onRecvFile(const TcpClientPtr& client, int needRecvLen)
{
	int hasReadBytes = 0;
	FILE* fp = fopen(packageName.c_str(), "wb");
	if (fp)
	{
		char buf[128 * 1024];
		int nRead = client->read(buf, sizeof buf);
		while (nRead > 0)
		{
			fwrite(buf, 1, nRead, fp);
			hasReadBytes += nRead;
			printRecvProcess(hasReadBytes, needRecvLen);

			if (hasReadBytes >= needRecvLen) break;

			nRead = client->read(buf, sizeof buf);
		}

		fclose(fp);
		cout << endl;
	}
	else
	{
		cerr << "failed to open: " << packageName.c_str() << endl;
	}
}

void printRecvProcess(int hasReceived, int sumBytes)
{
	double percent = (double)hasReceived / (double)sumBytes;

	printf("正在接收文件： %d%%  %d | %d\r", (int)(percent * 100), hasReceived, sumBytes);
}