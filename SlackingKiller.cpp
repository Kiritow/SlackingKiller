/*
	摸鱼终结者: 统计摸鱼程序和运行时间 (滑稽)
	Created by Kiritow.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <ctime>
using namespace std;

#include <Windows.h>
#include <TlHelp32.h>

class OutputFormater
{
public:
	vector<vector<string>> data;
	void add(const vector<string>& vec)
	{
		data.push_back(vec);
	}
	void update()
	{
		int n = 0;
		for (const auto& rows : data)
		{
			n = n > rows.size() ? n : rows.size();
		}
		vector<int>(n, 0).swap(colMaxLen);
		for (const auto& rows : data)
		{
			int i = 0;
			for (const auto& col : rows)
			{
				colMaxLen[i] = colMaxLen[i] > col.size() ? colMaxLen[i] : col.size();
				++i;
			}
		}
	}
	
private:
	vector<int> colMaxLen;
	friend ostream& operator << (ostream& ofs, OutputFormater& formater);
};

ostream& operator << (ostream& ofs,OutputFormater& formater)
{
	formater.update();
	for (const auto& rows : formater.data)
	{
		int i = 0;
		for (const auto& col : rows)
		{
			ofs << col << string(formater.colMaxLen[i] - col.size() + 1, ' ');
			++i;
		}
		ofs << endl;
	}
	return ofs;
}

class winapi_exception : public exception
{
public:
	winapi_exception(const string& callfn)
	{
		str = "WINAPI " + callfn + " Failed. GetLastError: " + to_string(GetLastError());
	}
	const char* what() const override
	{
		return str.c_str();
	}
private:
	string str;
};

class ProcInfo
{
public:
	string name;
	long long startTime;
};

// exception: winapi_exception
vector<ProcInfo> GetAllInfo()
{
	HANDLE hPS = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hPS == INVALID_HANDLE_VALUE)
		throw winapi_exception("CreateToolhelp32Snapshot");
	vector<ProcInfo> vec;
	PROCESSENTRY32 pe32 = { sizeof(pe32) };
	if (Process32First(hPS, &pe32))
	{
		do 
		{
			ProcInfo p;
			p.name = pe32.szExeFile;

			HANDLE hand = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
			if (hand == NULL)
			{
				p.startTime = -1;
			}
			else
			{
				FILETIME ftime, a, b, c;
				GetProcessTimes(hand, &ftime, &a, &b, &c);
				CloseHandle(hand);

				p.startTime = (*(long long*)&ftime - 116444736000000000) / 10000000;
			}

			vec.push_back(p);
		} while (Process32Next(hPS, &pe32));
	}
	CloseHandle(hPS);
	return vec;
}

vector<string> ReadConfig()
{
	ifstream ifs("WatchList.txt");
	if (!ifs)
	{
		ofstream ofs("WatchList.txt");
		if (!ofs)
		{
			cout << "Failed to create WatchList.txt" << endl;
		}

		return vector<string>();
	}
	vector<string> vec;
	string str;
	while (getline(ifs, str))
	{
		vec.push_back(str);
	}
	return vec;
}

string GetReadableTime(long long sec)
{
	string ans;
	if (sec >= 3600)
	{
		ans += to_string(sec / 3600) + "h ";
		sec %= 3600;
	}
	if (sec >= 60)
	{
		ans += to_string(sec / 60) + "m ";
		sec %= 60;
	}
	if (sec > 0)
	{
		ans += to_string(sec) + "s ";
	}

	return ans;
}

void CompareSimple()
{
	auto conf = ReadConfig();
	auto vec = GetAllInfo();
	int cnt = 0;
	OutputFormater output;

	for (const auto& x : vec)
	{
		for (const auto& y : conf)
		{
			if (x.name.find(y) != string::npos)
			{
				if (x.startTime > 0)
				{
					output.add({ x.name,GetReadableTime(time(NULL) - x.startTime) });
					//cout << x.name << " " << GetReadableTime(time(NULL) - x.startTime) << endl;
				}
				else
				{
					output.add({ x.name,"Unknown Running Time" });
					//cout << x.name << " Unknown Running Time" << endl;
				}
				++cnt;
				break;
			}
		}
	}

	cout << output;

	cout << cnt << " programs found" << endl;
}

int main()
{
	try
	{
		CompareSimple();
	} 
	catch (exception& e)
	{
		cout << e.what() << endl;
	}
	
	return 0;
}
