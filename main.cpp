#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <utility>
#include <string>

#include <boost/format.hpp>
#include <boost/regex.hpp>

// google cityhash
#include <city.h>
#include "hashtable.hpp"

struct URL
{
	std::string strProtocol;
	std::string strDomain;
	std::string strPath;
};

std::string strContent;
boost::regex stUrlFormatRegex("^([a-zA-Z0-9]+)://([^/]+)(/?[\\w\\W]*)$", boost::regex::normal|boost::regex::icase);
URL stBaseUrl;

int GetUrl(const char* szUrl, URL* pstUrl)
{
	if(pstUrl == NULL) return -1;

	boost::smatch m;
	if(boost::regex_search(std::string(szUrl), m, stUrlFormatRegex))
	{
		pstUrl->strProtocol = m[1].str();
		pstUrl->strDomain = m[2].str();
		pstUrl->strPath = m[3].str();
		if(pstUrl->strPath.empty())
			pstUrl->strPath = "/";
		return 0;
	}
	else
		return -1;
}

std::string FixUrl(std::string strUrl)
{
	boost::regex stScriptRegex("^([a-zA-Z0-9]+):[\\w\\W]*$", boost::regex::normal|boost::regex::icase);
	boost::smatch what;
	if(!boost::regex_search(strUrl, what, stUrlFormatRegex))
	{
		if(strUrl.at(0) == '/')
			return (boost::format("%s://%s%s") % stBaseUrl.strProtocol % stBaseUrl.strDomain % strUrl).str();
		else
		{
			if(boost::regex_search(strUrl, stScriptRegex))
				return std::string("");

			std::string::size_type pos = stBaseUrl.strPath.find_last_of("/");
			return (boost::format("%s://%s%s%s") % stBaseUrl.strProtocol % stBaseUrl.strDomain % stBaseUrl.strPath.substr(0, pos + 1) % strUrl).str();
		}
	}
	else
		return strUrl;
}

void print_usage()
{
	printf("usage: urls [-help] [-base \"url\"] [content]\n");
}

int main(int argc, char* argv[])
{
	const char* szArgContent = NULL;;
	for(int i=1; i<argc; ++i)
	{
		if(strcmp(argv[i], "-help") == 0)
		{
			print_usage();
			return -1;
		}
		else if(strcmp(argv[i], "-base") == 0)
		{
			++i;
			if(i < argc)
			{
				GetUrl(argv[i], &stBaseUrl);
			}
			else
			{
				print_usage();
				return -1;
			}
		}
		else
		{
			szArgContent = argv[i];
		}
	}

	if(szArgContent == NULL)
	{
		char buffer[4097];
		memset(buffer, 0, 4097);
		int size = 0;
		while((size = read(STDIN_FILENO, buffer, 4096)) > 0)
		{
			strContent.append(buffer);
			memset(buffer, 0, size);
		}
	}
	else
		strContent = std::string(szArgContent);

	HashTable<u_int64_t, u_int32_t> stDict = HashTable<u_int64_t, u_int32_t>::CreateHashTable(1000, 10);

	boost::regex stUrlRegex("<a [^<>]*href=\"?'?([^ \"'>#]+)\"?'?[^<>]*>|<i?frame [^<>]*src=\"?'?([^ \"'>#]+)\"?'?[^<>]*>", boost::regex::normal|boost::regex::icase);
	boost::sregex_iterator end;
	for(boost::sregex_iterator iter = boost::make_regex_iterator(strContent, stUrlRegex);
		iter != end;
		++iter)
	{
		std::string strUrl = iter->str(1);
		if(strUrl.empty())
			strUrl = FixUrl(iter->str(2));
		else
			strUrl = FixUrl(strUrl);

		if(strUrl.empty())
			continue;
		
		u_int64_t ddwUrlId = CityHash64(strUrl.c_str(), strUrl.length());
		if(stDict.Hash(ddwUrlId) == NULL)
		{
			printf("error(%s:%d): HashTable is overflow.\n", __FILE__, __LINE__);
			return -1;
		}

		if(*stDict.Hash(ddwUrlId) == 1)
			continue;

		*stDict.Hash(ddwUrlId) = 1;
		printf("%s\n", strUrl.c_str());
	}

	stDict.Delete();
	return 0;
}


