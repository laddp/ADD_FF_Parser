#include <iostream>
#include <fstream>
#include <map>
#include <iomanip>
using namespace std;

#include "RecordFactory.h"
#include "Util.h"
#include "Record.h"

int Record::Length[Trailer+1];

class RecordDumper : public unary_function<Record *, void>
{
public:
	RecordDumper(ofstream& out) : os(out) {}
    void operator() (Record *x) { x->dump(os); }
private:
    ofstream& os;
};



const string Record::RT_to_String(RecordType rt)
{
	switch (rt)
	{
		case Record::InfoHeader:      return "InfoHeader";      break;
		case Record::DocHeader:       return "StmtHeader";      break;
		case Record::Division:        return "Division";        break;
		case Record::PostCode:        return "PostCode";        break;
		case Record::PromptPayMsg:    return "PromptPay/ContractInfo";  break;
		case Record::Statement:       return "Statement";       break;
		case Record::LineItem:        return "LineItem";        break;
		case Record::CreditInfo:      return "CreditInfo";      break;
		case Record::DiscntInfo:      return "DiscntInfo";      break;
		case Record::ContractFlags:   return "ContractFlags";   break;
		case Record::FinChgInfo:      return "FinChgInfo";      break;
		case Record::MeterInfo:       return "MeterInfo";       break;
		case Record::ContractLine:    return "ContractLine";    break;
		case Record::ContractTaxLine: return "ContractTaxLine"; break;
		case Record::LineItemAddr:    return "LineItemAddr";    break;
		case Record::StatementAddr:   return "StatementAddr";   break;
		case Record::Trailer:         return "Trailer";         break;
		default: return "Invalid"; break;
		}
}


Record::Record(RecordType rt, const char * const sf, int line):
	Type(rt), StatementFile(sf), RecordNum(line)
{
}

Record::~Record(void)
{
}


void Record::InitStatics(void)
{
	for (int i = 0; i < Trailer; i++)
		Length[i] = 0;

	// NOTE: Make sure to update MaxLen if one of these changes!
	Length[InfoHeader]   =    0; // not 13 as called for in spec!
	Length[DocHeader]    =  752;
	Length[Division]     =  260;
	Length[PostCode]     =   40;
	Length[PromptPayMsg] =  258;
	Length[Statement]    =  315;
	Length[LineItem]     =  285;
	Length[CreditInfo]   = 1798;
	Length[DiscntInfo]   =   17;
	Length[FinChgInfo]   =   17;
	Length[MeterInfo]    =  666;
	Length[Trailer]      =   33;
}

InfoHeaderRec::InfoHeaderRec(ifstream &ifs, const char * const sf, int line):
	Record(InfoHeader, sf, line)
{
}


void InfoHeaderRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << InfoHeader << '\n';
	
	of.fill(fc);
}


DocumentHeaderRec::DocumentHeaderRec(ifstream &ifs, const char * const sf, int line,
                                     DocumentHeaderRec::DocType type):
	Record(DocHeader, sf, line)
{
	ifs.get(docType, sizeof(docType));
	if (type == Statement)
		if (strcmp(docType, "STATEMENT                ") != 0)
		{
			cerr << "Error: docType not \"STATEMENT\", received \"" << docType << "\"\n";
		}
	ifs.get(docDate, sizeof(docDate));
	ifs.get(db, sizeof(db));
	ifs.get(budgetDue, sizeof(budgetDue));
	ifs.get(budgetMonDue, sizeof(budgetMonDue));

	if (opt_r)
	{
		cout << "Date: "
			 << docDate[0] << docDate[1] << '/'
			 << docDate[2] << docDate[3] << '/'
			 << docDate[4] << docDate[5] << '\n'
			 << "Statement Message: \n";
	}

	for (int i = 0; i < 11; i++)
	{
		ifs.get(docMessage[i], 64+1);
		if (opt_r)
			cout << docMessage[i] << '\n';
	}

	ifs.get(logo, sizeof(logo));
	ifs.get(expDB, sizeof(expDB));
	}


void DocumentHeaderRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << DocHeader;

	of << docType
	   << docDate
	   << db
	   << budgetDue
	   << budgetMonDue;
	for (int i = 0; i < 11; i++)
	{
		of << docMessage[i];
	}
	of << logo
	   << expDB
	   << '\n';

	of.fill(fc);
}


DivisionRec::DivisionRec(ifstream &ifs, const char * const sf, int line):
	Record(Division, sf, line)
{
	char divNum[2+1];
	ifs.get(divNum, sizeof(divNum));
	DivNum = atoi(divNum);
	ifs.get(DivisionName, sizeof(DivisionName));
	ifs.get(DivisionAddr1, sizeof(DivisionAddr1));
	ifs.get(DivisionAddr2, sizeof(DivisionAddr2));
	ifs.get(DivisionAddr3, sizeof(DivisionAddr3));
}


void DivisionRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << Division;
	of.width(2);
	of << DivNum;
	
	of << DivisionName
	   << DivisionAddr1
	   << DivisionAddr2
	   << DivisionAddr3
	   << '\n';

	of.fill(fc);
}


PostingCode::PostingCode(ifstream &ifs, const char * const sf, int line):
	Record(PostCode, sf, line)
{
	ifs.get(Code, sizeof(Code));
	ifs.get(ShortDesc, sizeof(ShortDesc));
	ifs.get(Description, sizeof(Description));
	if (opt_r)
	{
		cout << "Posting Code: " << Code << " Desc: " << Description << '\n';
	}
}


void PostingCode::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << PostCode;
	of.width(3);
	of << Code;
	
	of << ShortDesc
	   << Description
	   << '\n';

	of.fill(fc);
}


PromptPayMsgRec::PromptPayMsgRec(ifstream &ifs, const char * const sf, int line):
	Record(PromptPayMsg, sf, line)
{
	ifs.get(Message, sizeof(Message));
}


void PromptPayMsgRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << PromptPayMsg;
	
	of << Message << '\n';

	of.fill(fc);
}


ContractInfoRec::ContractInfoRec(ifstream &ifs, const char * const sf, int line):
	Record(ContractInfo, sf, line)
{
	ifs.get(Contract, sizeof(Contract));
	ContractLetter = Contract[0];

	char buf2[2+1];
	buf2[0] = Contract[1];
	buf2[1] = Contract[2];
	buf2[2] = '\0';
	BasePrice = atoi(buf2);
	
	buf2[0] = Contract[3];
	buf2[1] = Contract[4];
	SubLevel = atoi(buf2);

	char c;
	unsigned int i = 0;
	do {
		ifs.get(c);
		if (i < sizeof(Text)-2)
		{
			Text[i] = c;
			i++;
		}
	}
	while (c != '\n');

	if (Text[i-1] == '\n')
		Text[i-1] = '\0';
	else
		Text[i] = '\0';
	ifs.putback('\n');
}


void ContractInfoRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << ContractInfo;
	
	of << Contract << '\n';

	for (int i = 0; i < 5; i++)
	{
		of << Text[i];
	}

	of.fill(fc);
}


bool StatementRec::isBudget(void) const
{
	if (strcmp(SType, "SS") == 0 || strcmp(SType, "BS") == 0 ||
		strcmp(SType, "BN") == 0 || strcmp(SType, "BD") == 0)
		return true;
	else
		return false;
}


void StatementRec::PrintDetail(void)
{
	cout << "Acct num: " << AcctNum << '/' << ExpAcctNum
	     << " Name:  " << Name << '\n'
		 << " Div: " << DivisionNum
		 << " Type: " << Type
		 << " Statement Type: " << SType
		 << " Spec Flag: " << SpecialHandling << '\n';
}

void StatementRec::PrintBalances(void)
{
	cout
//		 << "Addr1: " << Addr1 << '\n'
//		 << "Addr2: " << Addr2 << '\n'
//		 << "Town/State/Zip: " << Town << ", " << State << " " << Zip << '\n'
		 << "Current: ";
	RecordFactory::PrintCurrency(cout, CurBal);
	cout << " Prev: ";
	RecordFactory::PrintCurrency(cout, PrevBal);
	cout << "\nBudget: ";
	RecordFactory::PrintCurrency(cout, BudgetAmt);
	cout << " Budget past due: ";
	RecordFactory::PrintCurrency(cout, BudgetPastDue);
	cout << " Budget pre-pay: ";
	RecordFactory::PrintCurrency(cout, PrepayBudgetCred);
	cout << " Non-budget: ";
	RecordFactory::PrintCurrency(cout, NonBudgetCharges);
	cout << "\nTotal: ";
	RecordFactory::PrintCurrency(cout, TotalDue);
	cout << '\n';
}

StatementRec::StatementRec(ifstream &ifs, const char * const sf, int line, bool isSvcContr):
	Record(Statement, sf, line), IsSvcContr(isSvcContr)
{
	char buf6[6+1];
	ifs.get(buf6, sizeof(buf6));
	AcctNum = atoi(buf6);

	char buf2[2+1];
	ifs.get(buf2, sizeof(buf2));
	DivisionNum = atoi(buf2);

	char buf1[1+1];
	ifs.get(buf1, sizeof(buf1));
	Type = atoi(buf1);

	ifs.get(Name,  sizeof(Name));
	ifs.get(Addr1, sizeof(Addr1));
	ifs.get(Addr2, sizeof(Addr2));
	ifs.get(Town,  sizeof(Town));
	ifs.get(State, sizeof(State));
	ifs.get(Zip,   sizeof(Zip));	

	char buf9[9+1];
	ifs.get(buf9, sizeof(buf9));
	PrevBal = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	CurBal = DoubleFromChar(buf9) / 100;
	
	ifs.get(BudgetStartMon, sizeof(BudgetStartMon));
	
	ifs.get(buf9, sizeof(buf9));
	BudgetAmt = DoubleFromChar(buf9) / 100;

	ifs.get(buf2, sizeof(buf2));
	BudgetPmtCnt = atoi(buf2);

	ifs.get(buf9, sizeof(buf9));
	BudgetPastDue = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	TotalDue = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	PrepayBudgetCred = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	NonBudgetCharges = DoubleFromChar(buf9) / 100;

	ifs.get(OCR, sizeof(OCR));
	ifs.get(SType, sizeof(SType));	

	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == 'K')
		OpenItem = true;
	else
		OpenItem = false;

	ifs.get(LDCNum, sizeof(LDCNum));
	ifs.get(DocRefNum, sizeof(DocRefNum));
	ifs.get(CompanyCode, sizeof(CompanyCode));

	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == 'Y')
		California = true;
	else if (buf1[0] == 'N' || buf1[0] == ' ')
		California = false;
	else
	{
    	cerr << "Error: California flag not Y/N, record #" << line << '\n';
    	throw;
	}

	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == 'Y')
		MiscAcctFlag = true;
	else if (buf1[0] == 'N' || buf1[0] == ' ')
		MiscAcctFlag = false;
	else
	{
    	cerr << "Error: MiscAcctFlag flag not Y/N, record #" << line << '\n';
    	throw;
	}

	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == 'Y')
		DutyToWarn = true;
	else if (buf1[0] == 'N' || buf1[0] == ' ')
		DutyToWarn = false;
	else
	{
    	cerr << "Error: DutyToWarn flag not Y/N, record #" << line << '\n';
    	throw;
	}

	ifs.get(buf9, sizeof(buf9));
	AltDue = DoubleFromChar(buf9) / 100;

	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == 'Y')
		SpecialHandling = true;
	else if (buf1[0] == 'N')
		SpecialHandling = false;
	else
	{
    	cerr << "Error: SpecialHandling flag not Y/N, record #" << line << '\n';
    	throw;
	}

	char buf4[4+1];
	ifs.get(buf4, sizeof(buf4));
	MessageCode = atoi(buf4);

	ifs.get(DirectCardInd, sizeof(DirectCardInd));
	
	char buf7[7+1];
	ifs.get(buf7, sizeof(buf7));
	ExpAcctNum = atoi(buf7);
	
	if (!IsSvcContr) {
		ifs.get(Language, sizeof(Language));	

		char buf11[11+1];
		ifs.get(buf11, sizeof(buf11));
		CreditLimit = DoubleFromChar(buf11) / 100;

		ifs.get(PrevStmtDate, sizeof(PrevStmtDate));

		if (!opt_old)
		{
			ifs.get(buf2, sizeof(buf2));
			CollectorNum = atoi(buf2);
		}
	}

	if (opt_a || opt_r)
		PrintDetail();
	if (opt_r)
		PrintBalances();

	if (IsSvcContr)
		return;

	int errCnt = 0;
	if (isBudget())
	{
		if (strcmp(SType, "SS") == 0) // settlement
		{
			double diff = CurBal - TotalDue;
			if (diff > 0.001 || diff < -0.001)
			{
				if (!opt_r && errCnt == 0)
				{
					PrintDetail();
					PrintBalances();
				}
				cout << "=========== ERROR: CurBal != TotalDue (Diff ";
				RecordFactory::PrintCurrency(cout, diff);
				cout << ")===========\n";
				errCnt++;
			}
		}
		else
		{
			double diff = (BudgetAmt + NonBudgetCharges + BudgetPastDue)
						  - (TotalDue + PrepayBudgetCred);
			if (diff > 0.001 || diff < -0.001)
			{
				if (!opt_r && errCnt == 0)
				{
					PrintDetail();
					PrintBalances();
				}
				cout << "=========== ERROR: BudgetAmt + NonBudget + BudPastDue != TotalDue + BudPrePay (Diff ";
				RecordFactory::PrintCurrency(cout, diff);
				cout << ")===========\n";
				errCnt++;
			}
		
			if (TotalDue < BudgetAmt)
			{
				if (!opt_r && errCnt == 0)
				{
					PrintDetail();
					PrintBalances();
				}
				cout << "=========== WARNING: TotalDue < BudgetAmt (diff ";
				RecordFactory::PrintCurrency(cout, TotalDue - BudgetAmt);
				cout << ")\n";
				errCnt++;
			}
		}
		if (opt_r)
		{
			cout << "Budget month: " << BudgetStartMon
				 << " Budget count: " << BudgetPmtCnt << '\n';
		}
	}
	else
	{
		double diff = CurBal - TotalDue;
		if (diff > 0.001 || diff < -0.001)
		{
			if (!opt_r && errCnt == 0)
			{
				PrintDetail();
				PrintBalances();
			}
			cout << "=========== ERROR: CurBal != TotalDue (Diff ";
			RecordFactory::PrintCurrency(cout, diff);
			cout << ")===========\n";
			errCnt++;
		}
	}
	if (TotalDue < 0.001)
	{
		if (!opt_r && errCnt == 0)
		{
			PrintDetail();
			PrintBalances();
		}
		cout << "=========== WARNING: TotalDue = $0\n";
		errCnt++;
	}
}


void StatementRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << Statement;

	of.width(6);
	of.fill(' ');
	of << left << AcctNum;

	of.width(2);
	of.fill('0');
	of << right << DivisionNum;
	
	of.width(1);
	of << Type;
	
	of.fill(' ');
	of << Name
	   << Addr1
	   << Addr2
	   << Town
	   << State
	   << Zip;

	of.fill('0');
	RecordFactory::DumpDouble(of, PrevBal);
	RecordFactory::DumpDouble(of, CurBal);

	of << BudgetStartMon;

	of.fill('0');
	RecordFactory::DumpDouble(of, BudgetAmt);

	of.width(2);
	of.fill(' ');
	of << BudgetPmtCnt;

	of.fill('0');
	RecordFactory::DumpDouble(of, BudgetPastDue);
	RecordFactory::DumpDouble(of, TotalDue);
	RecordFactory::DumpDouble(of, PrepayBudgetCred);
	RecordFactory::DumpDouble(of, NonBudgetCharges);

	of << OCR
	   << SType;

	if (OpenItem)
		of << 'K';
	else
		of << ' ';

	of << LDCNum
	   << DocRefNum
	   << CompanyCode;

	if (California)
		of << 'Y';
	else
		of << ' ';

	if (MiscAcctFlag)
		of << 'Y';
	else
		of << ' ';

	if (DutyToWarn)
		of << 'Y';
	else
		of << ' ';

	RecordFactory::DumpDouble(of, AltDue);

	if (SpecialHandling)
		of << 'Y';
	else
		of << 'N';

	of.width(4);
	of.fill(' ');
	of << MessageCode;

	of << DirectCardInd;

	of.width(7);
	of.fill(' ');
	of << left << ExpAcctNum;

	if (!IsSvcContr)
	{
		of << Language;

		of.fill(' ');
		RecordFactory::DumpDouble(of, CreditLimit, 11, 2, true);

		of << PrevStmtDate;
	
		if (!opt_old)
		{
			of << CollectorNum
			   << '\n';
		}
	}

	of.fill(fc);
}

void StatementRec::AddItem(Record *r)
{
	Items.push_back(r);
	if (Items.size() == 50)
		cerr << "Warning: Statement for Acct#" <<  AcctNum << " has 50 or more item records\n";
}


void StatementRec::DumpItems(ofstream &of) const
{
	RecordDumper rd(of);
	for_each(Items.begin(), Items.end(), rd);
}


LineItemRec::LineItemRec(StatementRec *s, RecordFactory::PcodeMap &pcodes,
						 ifstream &ifs, const char * const sf, int line):
	Record(LineItem, sf, line), Statement(s)
{
	ifs.get(TransDate, sizeof(TransDate));
	ifs.get(TransCode, sizeof(TransCode));

	char buf9[9+1];
	ifs.get(buf9, sizeof(buf9));
	TransDollars = DoubleFromChar(buf9) / 100;
	
	ifs.get(buf9, sizeof(buf9));
	TransGallons = DoubleFromChar(buf9) / 10;

	char buf8[8+1];
	ifs.get(buf8, sizeof(buf8));
	TransUnitPrice = DoubleFromChar(buf8) / 10000;

	ifs.get(RefNum, sizeof(RefNum));
	ifs.get(Addr, sizeof(Addr));
	ifs.get(DetailMsg[0], sizeof(DetailMsg[0]));
	ifs.get(DetailMsg[1], sizeof(DetailMsg[1]));

	ifs.get(CylQuantDeliv, sizeof(CylQuantDeliv));
	ifs.get(CylQuantReturn, sizeof(CylQuantReturn));
	ifs.get(PONum, sizeof(PONum));

	char buf1[1+1];
	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == 'Y')
		TransWasMinCharge = true;
	else if (buf1[0] == 'N' || buf1[0] == ' ')
		TransWasMinCharge = false;
	else
	{
    	cerr << "Error: TransWasMinCharge flag not Y/N, record #" << line << '\n';
    	throw;
	}

	ifs.get(buf9, sizeof(buf9));
	OrigTransDollars = DoubleFromChar(buf9) / 100;

	if (opt_old == 0)
	{
		char buf42[42+1];
		ifs.get(buf42, sizeof(buf42));
		if (strcmp(buf42, "                                          ")!=0)
			cerr << "Hmmm, 42 char field is not blank, record #" << line << '\n';
		char buf3[3+1];
		ifs.get(buf3, sizeof(buf3));
		TankNum = atoi(buf3);
	}

/* Not implemented until 2.6.3
	ifs.get(DueDate, sizeof(DueDate));
	ifs.get(NetDays, sizeof(NetDays));

	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == ' ' || buf1[0] == '\0')
		KeyOffFlag = true;
	else if (buf1[0] == 'N')
		KeyOffFlag = false;
	else
	{
    	cerr << "Error: KeyOffFlag flag not Y/N, record #" << line << '\n';
    	throw;
	}
*/

	RecordFactory::PcodeMap::const_iterator i = pcodes.find(TransCode);
	if (i == pcodes.end())
	{
		cerr << "Error: posting code (" << TransCode << ") not in posting code map\n";
	}
	else
	{
		if (i->second->isInval())
		{
			cerr << "Error: invalid posting code used\n";
		}
		else if (opt_r)
		{
			cout << "line item: "
				 << TransDate[0] << TransDate[1] << '/'
				 << TransDate[2] << TransDate[3] << '/'
				 << TransDate[4] << TransDate[5]
			     << " " << i->second->getDescription()
				 << " ";
		 	RecordFactory::PrintCurrency(cout, TransDollars);
			if (TransGallons > 0.01)
				cout << " (" << setprecision(1) << TransGallons << " gal @ $"
				     << setprecision(5) << TransUnitPrice << ")\n" << setprecision(5);
			else
				cout << '\n';
			
			if (Addr[0] != ' ')	
				cout << "Address: " << Addr << '\n';
		}
	}
}


void LineItemRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << LineItem;

	of << TransDate
	   << TransCode;
	
	of.fill('0');
	RecordFactory::DumpDouble(of, TransDollars);
	RecordFactory::DumpDouble(of, TransGallons, 9, 1);
	RecordFactory::DumpDouble(of, TransUnitPrice, 8, 4);

	of << RefNum
	   << Addr
	   << DetailMsg[0]
	   << DetailMsg[1]
	   << CylQuantDeliv
	   << CylQuantReturn
	   << PONum;

	if (TransWasMinCharge)
		of << 'Y';
	else
		of << ' ';

	RecordFactory::DumpDouble(of, OrigTransDollars);

	of << '\n';
	of.fill(fc);
}


ContractLineRec::ContractLineRec(StatementRec *s, RecordFactory::PcodeMap &pcodes,
                                 RecordFactory::ContractInfoMap &contracts,
                                 ifstream &ifs, const char * const sf, int line):
	Record(ContractLine, sf, line), Statement(s)
{
	ifs.get(Date, sizeof(Date));
	ifs.get(PostingCode, sizeof(PostingCode));
	ifs.get(Contract, sizeof(Contract));

	char buf9[9+1];
	ifs.get(buf9, sizeof(buf9));
	Amount = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	Discount = DoubleFromChar(buf9) / 100;
	
	ifs.get(RefNum, sizeof(RefNum));
	ifs.get(SvcAddr, sizeof(SvcAddr));
	ifs.get(CoveragePeriod, sizeof(CoveragePeriod));
	ifs.get(RenewalPeriod, sizeof(RenewalPeriod));
	ifs.get(ConsecutiveMonthsLeft, sizeof(ConsecutiveMonthsLeft));
	ifs.get(ConsecutiveMonthsTotal, sizeof(ConsecutiveMonthsTotal));
	
	ifs.get(buf9, sizeof(buf9));
	Deviation = DoubleFromChar(buf9) / 100;
	
	char buf3[3+1];
	ifs.get(buf3, sizeof(buf3));
	Location = atoi(buf3);

	RecordFactory::PcodeMap::const_iterator pi = pcodes.find(PostingCode);
	if (pi == pcodes.end())
	{
		cerr << "Error: posting code (" << PostingCode << ") not in posting code map\n";
	}
	else
	{
		if (pi->second->isInval())
		{
			cerr << "Error: invalid posting code used\n";
		}
	}

	RecordFactory::ContractInfoMap::const_iterator ci = contracts.find(Contract);
	if (ci == contracts.end())
	{
		cerr << "Error: contract code (" << Contract << ") not in contract code map\n";
	}
	
	if (opt_r)
	{
			cout << "contract line: "
				 << Date[0] << Date[1] << '/'
				 << Date[2] << Date[3] << '/'
				 << Date[4] << Date[5]
			     << " " << ci->second->getText()
				 << " ";
		 	RecordFactory::PrintCurrency(cout, Amount);
			cout << '\n';
			
			if (SvcAddr[0] != ' ')	
				cout << "Address: " << SvcAddr << '\n';
	}
}

void ContractLineRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << ContractLine
       << Date
       << PostingCode
       << Contract;

	RecordFactory::DumpDouble(of, Amount);
	RecordFactory::DumpDouble(of, Discount);

	of << RefNum
       << SvcAddr
       << CoveragePeriod
       << RenewalPeriod
       << ConsecutiveMonthsLeft
       << ConsecutiveMonthsTotal;

	RecordFactory::DumpDouble(of, Deviation);

	of.width(3);
	of << Location
	   << '\n';

	of.fill(fc);
}


ContractTaxLineRec::ContractTaxLineRec(StatementRec *s, 
                                       ifstream &ifs, const char * const sf, int line):
	Record(LineItem, sf, line), Statement(s)
{
	char buf9[9+1];
	ifs.get(buf9, sizeof(buf9));
	SalesTax = DoubleFromChar(buf9) / 100;
	
	ifs.get(buf9, sizeof(buf9));
	TotalAmount = DoubleFromChar(buf9) / 100;
}


void ContractTaxLineRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << ContractTaxLine;
	
	RecordFactory::DumpDouble(of, SalesTax);
	RecordFactory::DumpDouble(of, TotalAmount);

	of << '\n';

	of.fill(fc);
}


CreditInfoRec::CreditInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line):
	Record(CreditInfo, sf, line), Statement(s)
{
	char buf9[9+1];
	ifs.get(buf9, sizeof(buf9));
	FinanceCharge = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	BudgetInterest = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	Current = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	PastDue1 = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	PastDue2 = DoubleFromChar(buf9) / 100;

	ifs.get(buf9, sizeof(buf9));
	PastDue3 = DoubleFromChar(buf9) / 100;
	
	ifs.get(buf9, sizeof(buf9));
	PastDue4 = DoubleFromChar(buf9) / 100;
	
	ifs.get(buf9, sizeof(buf9));
	PriceProgGallonsRemain = DoubleFromChar(buf9) / 100;
	
	ifs.get(Addr, sizeof(Addr));
	ifs.get(DunningMsg[0], sizeof(DunningMsg[0]));
	ifs.get(DunningMsg[1], sizeof(DunningMsg[1]));
	for (int i = 0; i < 24; i++)
	{
		ifs.get(SummaryMsg[i], sizeof(SummaryMsg[0]));
	}

	char buf4[4+1];
	ifs.get(buf4, sizeof(buf4));
	FinanceChargeAnnualRate = DoubleFromChar(buf4) / 100;
	
	ifs.get(buf4, sizeof(buf4));
	FinanceChargeMonthlyRate = DoubleFromChar(buf4) / 100;

	ifs.get(buf9, sizeof(buf9));
	AvgDailyBal = DoubleFromChar(buf9) / 100;
	
	ifs.get(FinanceChargeDate, sizeof(FinanceChargeDate));
	
	if (!opt_old)
	{
		ifs.get(FinanceChargeDueDate, sizeof(FinanceChargeDueDate));

		ifs.get(buf9, sizeof(buf9));
		LateFee = DoubleFromChar(buf9) / 100;
	}

	if (opt_r)
	{
		cout << "Current: ";
		RecordFactory::PrintCurrency(cout, Current);
		cout << " 30 past: ";
		RecordFactory::PrintCurrency(cout, PastDue1);
		cout << " 60 past: ";
		RecordFactory::PrintCurrency(cout, PastDue2);
		cout << " 90 past: ";
		RecordFactory::PrintCurrency(cout, PastDue3);
		cout << " 120 past: ";
		RecordFactory::PrintCurrency(cout, PastDue4);
		cout << "\nGals left: " << PriceProgGallonsRemain
			 << " FinCharge: ";
		RecordFactory::PrintCurrency(cout, FinanceCharge);
		cout << " BudgetInt: ";
		RecordFactory::PrintCurrency(cout, BudgetInterest);
		cout << '\n';
	}
}


void CreditInfoRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << CreditInfo;

	RecordFactory::DumpDouble(of, FinanceCharge);
	RecordFactory::DumpDouble(of, BudgetInterest);
	RecordFactory::DumpDouble(of, Current);
	RecordFactory::DumpDouble(of, PastDue1);
	RecordFactory::DumpDouble(of, PastDue2);
	RecordFactory::DumpDouble(of, PastDue3);
	RecordFactory::DumpDouble(of, PastDue4);

	if (PriceProgGallonsRemain * 100 < 0.99)
		of << "         ";
	else
		RecordFactory::DumpDouble(of, PriceProgGallonsRemain);

	of << Addr
	   << DunningMsg[0]
	   << DunningMsg[1];

	for (int i = 0; i < 24; i++)
		of << SummaryMsg[i];

	RecordFactory::DumpDouble(of, FinanceChargeAnnualRate, 4, 2, true);
	RecordFactory::DumpDouble(of, FinanceChargeMonthlyRate, 4, 2, true);
	RecordFactory::DumpDouble(of, AvgDailyBal, 9, 2, true);

	of << FinanceChargeDate;

	if (!opt_old)
	{
		of << FinanceChargeDueDate;
		RecordFactory::DumpDouble(of, LateFee);
	}

	of << "\n";
	of.fill(fc);
}


DiscntInfoRec::DiscntInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line):
	Record(DiscntInfo, sf, line), Statement(s)
{
	ifs.get(DiscDate, sizeof(DiscDate));
	char buf9[9+1];
	ifs.get(buf9, sizeof(buf9));
	DiscAmt = DoubleFromChar(buf9) / 100;
	if (opt_r && DiscAmt != 0.0)
	{
		cout << "Date: "
			 << DiscDate[0] << DiscDate[1] << '/'
			 << DiscDate[2] << DiscDate[3] << '/'
			 << DiscDate[4] << DiscDate[5]
		     << "  Amt:  ";
		RecordFactory::PrintCurrency(cout, DiscAmt);
		cout << '\n';
	}
}


void DiscntInfoRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << DiscntInfo;

	of << DiscDate;

	RecordFactory::DumpDouble(of, DiscAmt, 9, 2, true);

	of << '\n';

	of.fill(fc);
}


FinChgInfoRec::FinChgInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line):
	Record(FinChgInfo, sf, line), Statement(s)
{
	ifs.get(ChargeDate, sizeof(ChargeDate));
	ifs.get(RefNum, sizeof(RefNum));
	
	if (opt_r)
	{
		cout << "Date: "
			 << ChargeDate[0] << ChargeDate[1] << '/'
			 << ChargeDate[2] << ChargeDate[3] << '/'
			 << ChargeDate[4] << ChargeDate[5]
		     << "  Ref#: " << RefNum << '\n';
	}
}


void FinChgInfoRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << FinChgInfo;

	of << ChargeDate
	   << RefNum
	   << '\n';
	
	of.fill(fc);	
}


ContractFlagsRec::ContractFlagsRec(StatementRec *s, ifstream &ifs, const char * const sf, int line):
	Record(ContractFlags, sf, line), Statement(s)
{
	ifs.get(TotalMessage, sizeof(TotalMessage));
	ifs.get(BudgetIncludeFlag, sizeof(BudgetIncludeFlag));
}


void ContractFlagsRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << ContractFlags;

	of << TotalMessage
	   << BudgetIncludeFlag
	   << '\n';
	
	of.fill(fc);	
}



MeterInfoRec::MeterInfoRec(StatementRec *s, ifstream &ifs, const char * const sf, int line):
	Record(MeterInfo, sf, line), Statement(s)
{
	cerr << "WHOA! Never seen a MeterInfo record before, please debug me!\n";
	throw;

	ifs.get(PrevReadDate, sizeof(PrevReadDate));
	ifs.get(PrevReading, sizeof(PrevReading));
	ifs.get(CurrReading, sizeof(CurrReading));
	ifs.get(Diff, sizeof(Diff));
	ifs.get(Units, sizeof(Units));
	ifs.get(UnitsDesc, sizeof(UnitsDesc));
	ifs.get(Type, sizeof(Type));
	ifs.get(SerialNum, sizeof(SerialNum));

	char buf7[7+1];
	ifs.get(buf7, sizeof(buf7));
	PressureAltConvFactor = DoubleFromChar(buf7) / 10000;

	char buf1[1+1];
	ifs.get(buf1, sizeof(buf1));
	if (buf1[0] == 'Y')
		LastReadingEstimated = true;
	else if (buf1[0] == 'N')
		LastReadingEstimated = false;
	else
	{
    	cerr << "Error: LastReadingEstimated flag not Y/N, record #" << line << '\n';
    	throw;
	}

	ifs.get(ContractRefNum, sizeof(ContractRefNum));

	for (int i = 0; i < 10; i++)
	{
		ifs.get(StepRates[i], sizeof(StepRates[0]));
	}

	ifs.get(buf7, sizeof(buf7));
	ConversionFactor = DoubleFromChar(buf7) / 10000;

	char buf10[10+1];
	ifs.get(buf10, sizeof(buf10));
	UnitPrice = DoubleFromChar(buf10) / 1000000;
}


void MeterInfoRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << MeterInfo;

	of << '\n';
	
	of.fill(fc);
}


TrailerRec::TrailerRec(ifstream &ifs, const char * const sf, int line):
	Record(Trailer, sf, line)
{
	ifs.get(Date, sizeof(Date));
	ifs.get(Time, sizeof(Time));

	char buf7[7+1];	
	ifs.get(buf7, sizeof(buf7));
	TotalItems = atoi(buf7);
	
	char buf10[10+1];
	ifs.get(buf10, sizeof(buf10));
	TotalBilled = DoubleFromChar(buf10) / 100;

	if (opt_r)	{
		cout << "Trailer Info: " << sf << '\n'
			 << "  Item Count:\t" << TotalItems << '\n'
			 << "  Total Billed\t";
		RecordFactory::PrintCurrency(cout, TotalBilled);
		cout << '\n'
			 << "Date: "
			 << Date[0] << Date[1] << '/' << Date[2] << Date[3] << '/' << Date[4] << Date[5]
			 << " Time: " << Time << '\n';
	}
}


void TrailerRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << Trailer;

	of << Date
	   << Time;

	of.fill(' ');
	of.width(7);
	of << TotalItems;
	
	of.fill('0');
	RecordFactory::DumpDouble(of, TotalBilled, 10);
	of << '\n';
	
	of.fill(fc);
}


#include <time.h>

void TrailerRec::dumpNew(ofstream &of, int dumpCount, double dump_stmt_bal) const
{
	of.width(2);
	char fc = of.fill('0');
	of << Trailer;

	time_t tt = time(0);
	struct tm *t = localtime(&tt);
	char timebuf[14+1];
	strftime(timebuf, sizeof(timebuf), "%m%d%y%I:%M %p", t);
	of << timebuf;

	of.fill(' ');
	of.width(7);
	of << dumpCount;
	
	of.fill('0');
	RecordFactory::DumpDouble(of, dump_stmt_bal, 10);

	of << '\n';
	
	of.fill(fc);
}


LineItemAddrRec::LineItemAddrRec(ifstream &ifs, const char * const sf, int line):
	Record(LineItemAddr, sf, line)
{
	ifs.get(Address, sizeof(Address));	
}


void LineItemAddrRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << LineItemAddr;

	of.fill(' ');
	of << Address;

	of.fill(fc);
}


StatementAddrRec::StatementAddrRec(ifstream &ifs, const char * const sf, int line):
	Record(StatementAddr, sf, line)
{
	ifs.get(Address, sizeof(Address));
}


void StatementAddrRec::dump(ofstream &of) const
{
	of.width(2);
	char fc = of.fill('0');
	of << StatementAddr;

	of.fill(' ');
	of << Address;

	of.fill(fc);
}


