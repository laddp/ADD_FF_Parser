#ifndef RECORD_H
#define RECORD_H
#include <fstream>
#include <string>
#include <map>
#include <vector>

#include "RecordFactory.h"
using namespace std;

class Record
{
public:
	typedef enum {
		InfoHeader      = 1,
		DocHeader       = 2,
		Division        = 3,
		PostCode        = 4,
		PromptPayMsg    = 5,
		ContractInfo    = 5,
		Statement       = 11,
		LineItem        = 12,
		CreditInfo      = 13,
		DiscntInfo      = 14,
		FinChgInfo      = 15,
		ContractFlags   = 16,
		MeterInfo       = 17,
		ContractLine    = 18,
		ContractTaxLine = 19,
		LineItemAddr    = 26,
		StatementAddr   = 28,
		Trailer         = 99
	} RecordType;
	static const string RT_to_String(RecordType rt);

    const RecordType   Type;
    const char * const StatementFile;
    const int          RecordNum;

	virtual void PrintDetail(void) { cout << "Record #" << RecordNum << "\n"; }
	virtual void dump(ofstream &of) const = 0;

	static void InitStatics(void);
	static int Len(RecordType t) { return Length[t]; }
	static const int MaxLen = 1798;

protected:
	static int Length[Trailer+1];
	
	Record(RecordType rt, const char * const sf, int line);
	virtual ~Record(void);
};


class InfoHeaderRec: public Record
{
public:
	InfoHeaderRec(ifstream &ifs, const char * const sf, int line);
	virtual void dump(ofstream &of) const;
};


class DocumentHeaderRec: public Record
{
public:
	typedef enum {
		Statement,
		SvcContr
	} DocType;

	DocumentHeaderRec(ifstream &ifs, const char * const sf, int line,
	                  DocType type);

	virtual void dump(ofstream &of) const;
		
	const string getDate(void) const { return docDate; }
private:
	char docType[25+1];
	char docDate[6+1];
	char db[2+1];
	char budgetDue[6+1];
	char budgetMonDue[2+1];
	char docMessage[11][64+1];
	char logo[1+1];
	char expDB[4+1];	
};

class DivisionRec: public Record
{
public:
	DivisionRec(ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

	int getDivisionNum(void) const { return DivNum; }

private:
	int DivNum;
	char DivisionName[64+1];
	char DivisionAddr1[64+1];
	char DivisionAddr2[64+1];
	char DivisionAddr3[64+1];
};


class PostingCode: public Record
{
public:
	PostingCode(ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;
	
	const string getCode() const { return Code; }
	const string getShortDesc() const { return ShortDesc; }
	const string getDescription() const { return Description; }

	bool isInval(void) const
	{
		if (strcmp(ShortDesc, "INVAL")==0)
			return true;
		else
			return false;
	}

private:
	char Code[3+1];
	char ShortDesc[5+1];
	char Description[30+1];
};


class PromptPayMsgRec: public Record
{
public:
	PromptPayMsgRec(ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;
	
private:
	char Message[256+1];
};


class ContractInfoRec : public Record
{
public:
	ContractInfoRec(ifstream &ifs, const char * const sf, int line);
	
	const char   getLetter() const { return ContractLetter; }
	const int    getBase()   const { return BasePrice; }
	const int    getSub()    const { return SubLevel; }
	const string getContr()  const { return Contract; }
	const string getText()   const { return Text; }

	virtual void dump(ofstream &of) const;
private:
	char Contract[5+1];
	char ContractLetter;
	int  BasePrice;
	int  SubLevel;
	char Text[(5*40)+1];
};


class StatementRec: public Record
{
public:
	StatementRec(ifstream &ifs, const char * const sf, int line, bool isSvcContr = false);

	virtual void dump(ofstream &of) const;

	int getAcctNum(void) const { return AcctNum; }
	double getCurBal(void) const { return CurBal; }
	double getTotalDue(void) const { return TotalDue; }
	int getDivisionNum(void) const { return DivisionNum; }

	void setPrevBal(double pv) { PrevBal = pv; }

	bool isBudget(void) const;
	bool specialHandling(void) const { return SpecialHandling; }

	void AddItem(Record *r);
	void DumpItems(ofstream &of) const;

	virtual void PrintDetail(void);
	void PrintBalances(void);

private:
	vector<Record *> Items;

	const bool IsSvcContr;

	int AcctNum;
	int DivisionNum;
	int Type;

	char Name[30+1];
	char Addr1[30+1];
	char Addr2[30+1];
	char Town[20+1];
	char State[10+1];
	char Zip[9+1];
	
	double PrevBal;
	double CurBal;
	
	char BudgetStartMon[2+1];
	double BudgetAmt;
	int BudgetPmtCnt;
	double BudgetPastDue;
	double TotalDue;
	double PrepayBudgetCred;
	double NonBudgetCharges;
	
	char OCR[47+1];

	char SType[2+1];
	bool OpenItem;
	
	char LDCNum[4+1];
	char DocRefNum[9+1];
	char CompanyCode[2+1];
	
	bool California;
	bool MiscAcctFlag;
	bool DutyToWarn;
	
	double AltDue;
	
	bool SpecialHandling;
	
	int MessageCode;
	
	char DirectCardInd[1+1];
	int ExpAcctNum;
	char Language[1+1];
	
	double CreditLimit;
	
	char PrevStmtDate[6+1];
	
	int CollectorNum;
};


class LineItemRec: public Record
{
public:
	LineItemRec(StatementRec *s, RecordFactory::PcodeMap &pcodes,
				ifstream &ifs, const char * const sf, int line);
	
	virtual void dump(ofstream &of) const;
	const char* const getDate(void) const { return TransDate; }
	double getAmt(void) const { return TransDollars; }

private:
	StatementRec *Statement;

	char TransDate[6+1];
	char TransCode[3+1];
	double TransDollars;
	double TransGallons;
	double TransUnitPrice;
	char RefNum[9+1];
	char Addr[60+1];
	char DetailMsg[2][64+1];
	char CylQuantDeliv[3+1];
	char CylQuantReturn[3+1];
	char PONum[25+1];
	bool TransWasMinCharge;
	double OrigTransDollars;
	/* Not implemented until 2.6.3
	char DueDate[6+1];
	char NetDays[4+1];
	bool KeyOffFlag;
	*/
	char Unknown[42+1];
	int TankNum;
};


class ContractLineRec : public Record
{
public:
	ContractLineRec(StatementRec *s, RecordFactory::PcodeMap &pcodes,
                    RecordFactory::ContractInfoMap &contracts,
                    ifstream &ifs, const char * const sf, int line);

	const string getContract() const { return Contract; }
	const string getPostCode() const { return PostingCode; }
	double       getAmount()   const { return Amount; }

	virtual void dump(ofstream &of) const;
private:
	StatementRec *Statement;

	char Date[6+1];
	char PostingCode[3+1];
	char Contract[5+1];
	double Amount;
	double Discount;
	char RefNum[9+1];
	char SvcAddr[60+1];
	char CoveragePeriod[25+1];
	char RenewalPeriod[1+1];
	char ConsecutiveMonthsLeft[2+1];
	char ConsecutiveMonthsTotal[2+1];
	double Deviation;
	int Location;
};


class ContractTaxLineRec: public Record
{
public:
	ContractTaxLineRec(StatementRec *s, ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

	double getTax()   const { return SalesTax; }
	double getTotal() const { return TotalAmount; }

private:
	StatementRec *Statement;

	double SalesTax;
	double TotalAmount;
};


class CreditInfoRec: public Record
{
public:
	CreditInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;
	
	double getFinanceCharge(void) const { return FinanceCharge; }

private:
	StatementRec *Statement;

	double FinanceCharge;
	double BudgetInterest;
	double Current;
	double PastDue1;
	double PastDue2;
	double PastDue3;
	double PastDue4;
	double PriceProgGallonsRemain;
	char Addr[60+1];
	char DunningMsg[2][64+1];
	char SummaryMsg[24][64+1];
	double FinanceChargeAnnualRate;
	double FinanceChargeMonthlyRate;
	double AvgDailyBal;
	char FinanceChargeDate[6+1];
	char FinanceChargeDueDate[6+1];
	double LateFee;
};

class DiscntInfoRec: public Record
{
public:
	DiscntInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

private:
	StatementRec *Statement;

	char DiscDate[6+1];
	double DiscAmt;
};


class FinChgInfoRec: public Record
{
public:
	FinChgInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

private:
	StatementRec *Statement;

	char ChargeDate[6+1];
	char RefNum[9+1];
};


class ContractFlagsRec : public Record
{
public:
	ContractFlagsRec(StatementRec *s, ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

private:
	StatementRec *Statement;
	
	char TotalMessage[1+1];
	char BudgetIncludeFlag[1+1];
};


class MeterInfoRec: public Record
{
public:
	MeterInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

private:
	StatementRec *Statement;

	char PrevReadDate[6+1];
	char PrevReading[9+1];
	char CurrReading[9+1];
	char Diff[9+1];
	char Units[11+1];
	char UnitsDesc[12+1];
	char Type[3+1];
	char SerialNum[15+1];
	double PressureAltConvFactor;
	bool LastReadingEstimated;
	char ContractRefNum[15+1];
	char StepRates[10][55+1];
	double ConversionFactor;
	double UnitPrice;	
};


class TrailerRec: public Record
{
public:
	TrailerRec(ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

	int getTotalItems(void) const { return TotalItems; }
	double getTotalBilled(void) const { return TotalBilled; }
	const string getDate(void) const { return Date; }
	const string getTime(void) const { return Time; }

	void dumpNew(ofstream &of, int dumpCount, double dump_stmt_bal) const;

private:
	char Date[6+1];
	char Time[8+1];
	int TotalItems;
	double TotalBilled;
};


class LineItemAddrRec: public Record
{
public:
	LineItemAddrRec(ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;
private:
	char Address[252+1];
};


class StatementAddrRec: public Record
{
public:
	StatementAddrRec(ifstream &ifs, const char * const sf, int line);

	virtual void dump(ofstream &of) const;

private:
	char Address[252+1];
};

#endif /* RECORD_H */
