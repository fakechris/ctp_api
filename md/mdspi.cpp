#include <memory.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
//#include "windows.h"

#include "md/mdspi.h"
#include "json.h"

using namespace std;
#pragma warning(disable : 4996)

extern int requestId;
//extern HANDLE g_hEvent;

map<string, ofstream*> g_files;

void dump(char* instrumentId, char* tradingDay, string content)
{
	// check file handle exists
	string basename("/vagrant/Demo/src/data/");
	string filename = basename + instrumentId + "_" + tradingDay + ".txt";
	map<string, ofstream*>::iterator it = g_files.find(filename);
	if (it == g_files.end()) {
		ofstream* s1 = new ofstream(filename.c_str(), ios::out);
		g_files.insert(std::make_pair(filename, s1));
		(*s1) << content;
		s1->flush();
	} else {
		(*(it->second)) << content;
		it->second->flush();
	}
}

void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
  IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
  cerr<<" 响应 | 连接中断..."
    << " reason=" << nReason << endl;
}

void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
  cerr<<" 响应 | 心跳超时警告..."
    << " TimerLapse = " << nTimeLapse << endl;
}

void CtpMdSpi::OnFrontConnected()
{
	cerr<<" 连接交易前置...成功"<<endl;
  //SetEvent(g_hEvent);
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId,
	        TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.UserID, userId);
	strcpy(req.Password, passwd);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
  cerr<<" 请求 | 发送登录..."<<((ret == 0) ? "成功" :"失败") << endl;
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
    cerr<<" 响应 | 登录成功...当前交易日:"
      <<pRspUserLogin->TradingDay<<endl;
	}
  //if(bIsLast) SetEvent(g_hEvent);
}

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
  vector<char*> list;
  char *token = strtok(instIdList, ",");
  while( token != NULL ){
    list.push_back(token);
    token = strtok(NULL, ",");
  }
  unsigned int len = list.size();
  char** pInstId = new char* [len];
  for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i];
	int ret=pUserApi->SubscribeMarketData(pInstId, len);
  cerr<<" 请求 | 发送行情订阅... "<<((ret == 0) ? "成功" : "失败")<< endl;
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspSubMarketData(
         CThostFtdcSpecificInstrumentField *pSpecificInstrument,
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<" 响应 |  行情订阅...成功"<<endl;
  //if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr<<" 响应 |  行情取消订阅...成功"<<endl;
  //if(bIsLast)  SetEvent(g_hEvent);
}

std::string serializeMarketDataJson(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	std::string result;
	JsonObject obj;
	// strings
	obj.insert(JsonString("instrumentId"), JsonString(pDepthMarketData->InstrumentID));
	obj.insert(JsonString("tradingDay"), JsonString(pDepthMarketData->TradingDay));
	obj.insert(JsonString("updateTime"), JsonString(pDepthMarketData->UpdateTime));
	obj.insert(JsonString("updateMillisec"), JsonNumber<int>(pDepthMarketData->UpdateMillisec));
	// exchange info
	obj.insert(JsonString("exchangeId"), JsonString(pDepthMarketData->ExchangeID));
	obj.insert(JsonString("exchangeInstId"), JsonString(pDepthMarketData->ExchangeInstID));
	// infos
	obj.insert(JsonString("lastPrice"), JsonNumber<double>(pDepthMarketData->LastPrice));
	obj.insert(JsonString("preSettlementPrice"), JsonNumber<double>(pDepthMarketData->PreSettlementPrice));
	obj.insert(JsonString("preClosePrice"), JsonNumber<double>(pDepthMarketData->PreClosePrice));
	obj.insert(JsonString("preOpenInterest"), JsonNumber<double>(pDepthMarketData->PreOpenInterest));
	obj.insert(JsonString("openPrice"), JsonNumber<double>(pDepthMarketData->OpenPrice));
	obj.insert(JsonString("highestPrice"), JsonNumber<double>(pDepthMarketData->HighestPrice));
	obj.insert(JsonString("lowestPrice"), JsonNumber<double>(pDepthMarketData->LowestPrice));
	obj.insert(JsonString("volume"), JsonNumber<int>(pDepthMarketData->Volume));
	obj.insert(JsonString("turnover"), JsonNumber<double>(pDepthMarketData->Turnover));
	obj.insert(JsonString("openInterest"), JsonNumber<double>(pDepthMarketData->OpenInterest));
	obj.insert(JsonString("closePrice"), JsonNumber<double>(pDepthMarketData->ClosePrice));
	obj.insert(JsonString("settlementPrice"), JsonNumber<double>(pDepthMarketData->SettlementPrice));
	obj.insert(JsonString("upperLimitPrice"), JsonNumber<double>(pDepthMarketData->UpperLimitPrice));
	obj.insert(JsonString("lowerLimitPrice"), JsonNumber<double>(pDepthMarketData->LowerLimitPrice));
	obj.insert(JsonString("preDelta"), JsonNumber<double>(pDepthMarketData->PreDelta));
	obj.insert(JsonString("currDelta"), JsonNumber<double>(pDepthMarketData->CurrDelta));
  // prices
	obj.insert(JsonString("bidPrice1"), JsonNumber<double>(pDepthMarketData->BidPrice1));
	obj.insert(JsonString("bidVolume1"), JsonNumber<int>(pDepthMarketData->BidVolume1));
	obj.insert(JsonString("askPrice1"), JsonNumber<double>(pDepthMarketData->AskPrice1));
	obj.insert(JsonString("askVolume1"), JsonNumber<int>(pDepthMarketData->AskVolume1));
	obj.insert(JsonString("bidPrice2"), JsonNumber<double>(pDepthMarketData->BidPrice2));
	obj.insert(JsonString("bidVolume2"), JsonNumber<int>(pDepthMarketData->BidVolume2));
	obj.insert(JsonString("askPrice2"), JsonNumber<double>(pDepthMarketData->AskPrice2));
	obj.insert(JsonString("askVolume2"), JsonNumber<int>(pDepthMarketData->AskVolume2));
	obj.insert(JsonString("bidPrice3"), JsonNumber<double>(pDepthMarketData->BidPrice3));
	obj.insert(JsonString("bidVolume3"), JsonNumber<int>(pDepthMarketData->BidVolume3));
	obj.insert(JsonString("askPrice3"), JsonNumber<double>(pDepthMarketData->AskPrice3));
	obj.insert(JsonString("askVolume3"), JsonNumber<int>(pDepthMarketData->AskVolume3));
	obj.insert(JsonString("bidPrice4"), JsonNumber<double>(pDepthMarketData->BidPrice4));
	obj.insert(JsonString("bidVolume4"), JsonNumber<int>(pDepthMarketData->BidVolume4));
	obj.insert(JsonString("askPrice4"), JsonNumber<double>(pDepthMarketData->AskPrice4));
	obj.insert(JsonString("askVolume4"), JsonNumber<int>(pDepthMarketData->AskVolume4));
	obj.insert(JsonString("bidPrice5"), JsonNumber<double>(pDepthMarketData->BidPrice5));
	obj.insert(JsonString("bidVolume5"), JsonNumber<int>(pDepthMarketData->BidVolume5));
	obj.insert(JsonString("askPrice5"), JsonNumber<double>(pDepthMarketData->AskPrice5));
	obj.insert(JsonString("askVolume5"), JsonNumber<int>(pDepthMarketData->AskVolume5));
	//
	obj.insert(JsonString("averagePrice"), JsonNumber<double>(pDepthMarketData->AveragePrice));
	return obj.serialize() + "\n";
}

std::string serializeMarketDataCSV(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	std::stringstream ss;
	std::string result;
	ss << pDepthMarketData->InstrumentID << ","
		 << pDepthMarketData->TradingDay << ","
		 << pDepthMarketData->UpdateTime << ","
		 << pDepthMarketData->UpdateMillisec << ","
		 << pDepthMarketData->LastPrice << ","
		 << pDepthMarketData->PreSettlementPrice << ","
		 << pDepthMarketData->PreClosePrice << ","
		 << pDepthMarketData->PreOpenInterest << ","
		 << pDepthMarketData->OpenPrice << ","
		 << pDepthMarketData->HighestPrice << ","
		 << pDepthMarketData->LowestPrice << ","
		 << pDepthMarketData->Volume << ","
		 << pDepthMarketData->Turnover << ","
		 << pDepthMarketData->OpenInterest << ","
		 << pDepthMarketData->ClosePrice << ","
		 << pDepthMarketData->SettlementPrice << ","
		 << pDepthMarketData->UpperLimitPrice << ","
		 << pDepthMarketData->LowerLimitPrice << ","
		 << pDepthMarketData->PreDelta << ","
		 << pDepthMarketData->CurrDelta << ","
		 << pDepthMarketData->BidPrice1 << ","
		 << pDepthMarketData->BidVolume1 << ","
		 << pDepthMarketData->AskPrice1 << ","
		 << pDepthMarketData->AskVolume1 << ","
		 << pDepthMarketData->BidPrice2 << ","
		 << pDepthMarketData->BidVolume2 << ","
		 << pDepthMarketData->AskPrice2 << ","
		 << pDepthMarketData->AskVolume2 << ","
		 << pDepthMarketData->BidPrice3 << ","
		 << pDepthMarketData->BidVolume3 << ","
		 << pDepthMarketData->AskPrice3 << ","
		 << pDepthMarketData->AskVolume3 << ","
		 << pDepthMarketData->BidPrice4 << ","
		 << pDepthMarketData->BidVolume4 << ","
		 << pDepthMarketData->AskPrice4 << ","
		 << pDepthMarketData->AskVolume4 << ","
		 << pDepthMarketData->BidPrice5 << ","
		 << pDepthMarketData->BidVolume5 << ","
		 << pDepthMarketData->AskPrice5 << ","
		 << pDepthMarketData->AskVolume5 << ","
		 << pDepthMarketData->AveragePrice << ","
		 << pDepthMarketData->ExchangeID << ","
		 << pDepthMarketData->ExchangeInstID << ",," << std::endl;
	return ss.str();
}


void CtpMdSpi::OnRtnDepthMarketData(
             CThostFtdcDepthMarketDataField *pDepthMarketData)
{
  cerr<<" 行情 | 合约:"<<pDepthMarketData->InstrumentID
    <<" 日期:"<<pDepthMarketData->TradingDay
    <<" 时间:"<<pDepthMarketData->UpdateTime
    <<" 毫秒:"<<pDepthMarketData->UpdateMillisec
    <<" 现价:"<<pDepthMarketData->LastPrice
    <<" 上次结算价:" << pDepthMarketData->PreSettlementPrice
    <<" 本次结算价:" << pDepthMarketData->SettlementPrice
    <<" 最高价:" << pDepthMarketData->HighestPrice
    <<" 最低价:" << pDepthMarketData->LowestPrice
    <<" 数量:" << pDepthMarketData->Volume
    <<" 成交金额:" << pDepthMarketData->Turnover
    <<" 持仓量:" << pDepthMarketData->OpenInterest
    <<" 卖一价:" << pDepthMarketData->AskPrice1
    <<" 卖一量:" << pDepthMarketData->AskVolume1
    <<" 买一价:" << pDepthMarketData->BidPrice1
    <<" 买一量:" << pDepthMarketData->BidVolume1
    <<" 持仓量:"<< pDepthMarketData->OpenInterest <<endl;
	//cerr << "csv: " << serializeMarketDataCSV(pDepthMarketData) << endl;
	//cerr << "json: " << serializeMarketDataJson(pDepthMarketData) << endl;
	dump(pDepthMarketData->InstrumentID, pDepthMarketData->TradingDay, serializeMarketDataJson(pDepthMarketData));
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
    cerr<<" 响应 | "<<pRspInfo->ErrorMsg<<endl;
  }
  return ret;
}
