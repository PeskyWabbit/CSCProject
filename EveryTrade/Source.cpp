#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include "NxCoreAPI.h"
#include "NxCoreAPI_class.h"
#include <string>
#include <chrono>
#include <thread>
#include <iostream>
#include <omp.h>
#include <thread>
#include <vector>


NxCoreClass nxCoreClass;
int startHour = 0;
int endHour = 0;
int startMinutes = 0;
int endMinutes = 0;
std::string file = "";
std::string ticker = "";
FILE* outfile = std::fopen("C:\\Users\\Nanex1\\Documents\\output1.txt", "a");
void getSymbol(const NxCoreMessage* pNxCoreMsg, char* Symbol)
{
	// Is this a valid option?
	if ((pNxCoreMsg->coreHeader.pnxStringSymbol->String[0] == 'o') && (pNxCoreMsg->coreHeader.pnxOptionHdr))
	{
		// If pnxsDateAndStrike->String[1] == ' ', then this symbol is in new OSI format.
		if (pNxCoreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[1] == ' ')
		{
			sprintf(Symbol, "%s%02d%02d%02d%c%08d",
				pNxCoreMsg->coreHeader.pnxStringSymbol->String,
				pNxCoreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Year - 2000,
				pNxCoreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Month,
				pNxCoreMsg->coreHeader.pnxOptionHdr->nxExpirationDate.Day,
				(pNxCoreMsg->coreHeader.pnxOptionHdr->PutCall == 0) ? 'C' : 'P',
				pNxCoreMsg->coreHeader.pnxOptionHdr->strikePrice);
		}
		// Otherwise the symbol is in old OPRA format.
		else
		{
			sprintf(Symbol, "%s%c%c",
				pNxCoreMsg->coreHeader.pnxStringSymbol->String,
				pNxCoreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[0],
				pNxCoreMsg->coreHeader.pnxOptionHdr->pnxsDateAndStrike->String[1]);
		}
	}
	// Not an option, just copy the symbol
	else
	{
		strcpy(Symbol, pNxCoreMsg->coreHeader.pnxStringSymbol->String);
	}
}

int __stdcall nxCoreCallback(const NxCoreSystem* pNxCoreSys, const NxCoreMessage* pNxCoreMessage)
{
	switch (pNxCoreMessage->MessageType)
	{
	
		case NxMSG_TRADE:
		{
			
			const NxCoreTrade& nt = pNxCoreMessage->coreData.Trade;
			const NxCoreHeader& ch = pNxCoreMessage->coreHeader;
			const NxTime& t = pNxCoreSys->nxTime;
			const char* lexchange = nxCoreClass.GetDefinedString(NxST_EXCHANGE, ch.ListedExg);
			const char* rexchange = nxCoreClass.GetDefinedString(NxST_EXCHANGE, ch.ReportingExg);

			char symbol[23];
			int lineHour = t.Hour;
			int lineMinutes = t.Minute;
			std::string lineTicker = (std::string) ch.pnxStringSymbol->String;
			if (lineTicker != ticker || (lineHour < startHour) || (lineHour > endHour)) {
				//std::cout << (std::string) ch.pnxStringSymbol->String << " " << ticker << std::endl;
				
				break;
			}
			if ((startHour == lineHour && lineMinutes >= startMinutes) ||
				(endHour == lineHour && lineMinutes <= endMinutes) ||
				(lineHour > startHour&& lineHour < endHour)) {
				//std::cout << "YEEEEEEEEEEEEEEEEEEEEEEET" << std::endl;
				getSymbol(pNxCoreMessage, symbol);
				fprintf(outfile, "%.2d:%.2d:%.2d.%.3d, Reporting:%s, Listed:%s, %s, Price(%ld@%.2lf), O(%.2lf), H(%.2lf), L(%.2lf), C(%.2lf), V(%I64d), Net(%.2lf)\n",
					(int)t.Hour, (int)t.Minute, (int)t.Second, (int)t.Millisecond,
					rexchange,
					lexchange,
					symbol,
					nt.Size,
					nxCoreClass.PriceToDouble(nt.Price, nt.PriceType),
					nxCoreClass.PriceToDouble(nt.Open, nt.PriceType),
					nxCoreClass.PriceToDouble(nt.High, nt.PriceType),
					nxCoreClass.PriceToDouble(nt.Low, nt.PriceType),
					nxCoreClass.PriceToDouble(nt.Last, nt.PriceType),
					nt.TotalVolume,
					nxCoreClass.PriceToDouble(nt.NetChange, nt.PriceType));
			}
		}
	
		case NxMSG_EXGQUOTE:
		{
			
			
			const NxCoreQuote& nq = pNxCoreMessage->coreData.ExgQuote.coreQuote;
			const NxCoreHeader& ch = pNxCoreMessage->coreHeader;
			const NxTime& t = pNxCoreSys->nxTime;
			const char* exchange = nxCoreClass.GetDefinedString(NxST_EXCHANGE, ch.ReportingExg);
			const char* listedexg = nxCoreClass.GetDefinedString(NxST_EXCHANGE, ch.ListedExg);
			char symbol[23];
			int lineHour = t.Hour;
			int lineMinutes = t.Minute;
			
			std::string lineTicker = (std::string) ch.pnxStringSymbol->String;
			if (lineTicker != ticker || (lineHour < startHour) || (lineHour > endHour)) {
				//std::cout << (std::string) ch.pnxStringSymbol->String << " " << ticker << std::endl;
				
				break;
			}
			if ((startHour == lineHour && lineMinutes >= startMinutes) ||
				(endHour == lineHour && lineMinutes <= endMinutes) ||
				(lineHour > startHour && lineHour < endHour)) {
				
				//std::cout << "YEEEEEEEEEEEEEEEEEEEEEEET" << std::endl;
				getSymbol(pNxCoreMessage, symbol);
				fprintf(outfile, "%.2d:%.2d:%.2d.%.3d, Reporting:%s, Listed:%s, %s, Ask(%ld@%.2lf), Bid(%ld@%.2lf), AskChange(%.2lf), BidChange(%.2lf), AskSizeChange(%.2lf), BidSizeChange(%.2lf)\n",
					(int)t.Hour, (int)t.Minute, (int)t.Second, (int)t.Millisecond,
					exchange,
					listedexg,
					symbol,
					nq.AskSize,
					nxCoreClass.PriceToDouble(nq.AskPrice, nq.PriceType),
					nq.BidSize,
					nxCoreClass.PriceToDouble(nq.BidPrice, nq.PriceType),
					nxCoreClass.PriceToDouble(nq.AskPriceChange, nq.PriceType),
					nxCoreClass.PriceToDouble(nq.BidPriceChange, nq.PriceType),
					nxCoreClass.PriceToDouble(nq.AskSizeChange, nq.PriceType),
					nxCoreClass.PriceToDouble(nq.BidSizeChange, nq.PriceType));
			}
		}

	break;
	}
	return NxCALLBACKRETURN_CONTINUE;
	
}


int main(int argc, char** argv)
{	
	if (!nxCoreClass.LoadNxCore("C:\\Users\\Nanex1\\source\\repos\\CSCProject\\Debug\\NxCoreAPI.dll"))
	{
		fprintf(stderr, "Can't find NxCoreAPI.dll\n");
		system("pause");
	}

	for (int i = 0; i < argc; i++) {
		std::cout << argv[i] << "\n";
	}
	// Place the python script in the directory containing the tape files.
	// then replace the line below with the formatted array declaration outputted by the python script i.e. std::string files[1] = {"20080101.GS.nx2"};
	
	file = "C:\\Users\\Nanex1\\source\\repos\\CSCProject\\Debug\\" + (std::string) argv[6];
	ticker = (std::string) argv[1];
	startHour = std::stoi((std::string) argv[2]);
	startMinutes = std::stoi((std::string) argv[3]);
	endHour = std::stoi((std::string) argv[4]);
	endMinutes = std::stoi((std::string) argv[5]);
	std::cout << file << " " << ticker << std::endl;

	nxCoreClass.ProcessTape(file.c_str(), 0, NxCF_EXCLUDE_CRC_CHECK, 0, nxCoreCallback);
	
	system("pause");
	std::fclose(outfile);
	return 0;
	
}
