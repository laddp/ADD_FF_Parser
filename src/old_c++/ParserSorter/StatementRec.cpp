#include <iostream>
#include <fstream>
#include <map>
#include <iomanip>
using namespace std;

#include "RecordFactory.h"
#include "Util.h"
#include "Record.h"


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
		 << " Spec Flag: " << SpecialHandling
		 << " Open item: " << OpenItem
		 << '\n';
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
	Record(Statement, sf, line),
	IsSvcContr(isSvcContr), StatementAddr(0), CreditRec(0), FinanceChargeRec(0), DiscountInfoRec(0),
	PrevItem(0), CurrentGroupLead(0), ItemCount(0)
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

		if (file_ver >= 2)
		{
			ifs.get(buf2, sizeof(buf2));
			CollectorNum = atoi(buf2);
		}

		if (file_ver >= 1)
		{
			char buf12[12+1];
			ifs.get(buf12, sizeof(buf12));
			PrevBalOld = PrevBal;
			PrevBal = DoubleFromChar(buf12) / 100;

			if (PrevBalOld - PrevBal > 0.001 || PrevBalOld - PrevBal < -0.001)
				cout << "Warning: Extended prev balance field doesn't match short field: Acct #"
				     << AcctNum << "\n";

			ifs.get(buf12, sizeof(buf12));
			CurBalOld = CurBal;
			CurBal = DoubleFromChar(buf12) / 100;				

			if (CurBalOld - CurBal > 0.001 || CurBalOld - CurBal < -0.001)
				cout << "Warning: Extended current balance field doesn't match short field: Acct #"
				     << AcctNum << "\n";
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
	if (file_ver >= 2 && !IsSvcContr)
	{
		RecordFactory::DumpDouble(of, PrevBalOld);
		RecordFactory::DumpDouble(of, CurBalOld);
	}
	else
	{
		RecordFactory::DumpDouble(of, PrevBal);
		RecordFactory::DumpDouble(of, CurBal);
	}
	
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
	
		if (file_ver >= 3)
		{
			of.width(2);
			of.fill('0');
			of << right << CollectorNum;
		}
		if (file_ver >= 2)
		{
			RecordFactory::DumpDouble(of, PrevBal, 12);
			RecordFactory::DumpDouble(of, CurBal,  12);
		}
	}

	of << '\n';
	of.fill(fc);
}


void StatementRec::AddItem(Record *r)
{
	if (!opt_d || !opt_s)
	{
            StmtItems.push_back(r);
    }

    switch (r->Type)
    {
    case Record::StatementAddr:
        if (StatementAddr != 0)
        {
            cerr << "Error: Multiple statement addresses, line #" << r->RecordNum << "\n";
            throw;
        }
        StatementAddr = r;
        break;
    case Record::CreditInfo:
        if (CreditRec != 0)
        {
            cerr << "Error: Multiple CreditRec, line #" << r->RecordNum << "\n";
            throw;
        }
        CreditRec = r;
        break;
    case Record::FinChgInfo:
        if (FinanceChargeRec != 0)
        {
            cerr << "Error: Multiple FinanceChargeRec, line #" << r->RecordNum << "\n";
            throw;
        }
        FinanceChargeRec = r;
        break;
    case Record::DiscntInfo:
        if (DiscountInfoRec != 0)
        {
            cerr << "Error: Multiple DiscountInfoRec, line #" << r->RecordNum << "\n";
            throw;
        }
        DiscountInfoRec = r;
        break;
    case Record::LineItem:
    {
        ItemCount++;
        if (ItemCount == 50) // == because we only want to warn the first time, not every time!
            cerr << "Warning: Statement for Acct#" <<  AcctNum << " has 50 or more item records\n";
        if (opt_d || opt_s)
        {
            LineItemRec *li = (LineItemRec *)r;
            int location = li->GetTankNum();
            li->SetPrevItem(PrevItem);
			
            if (location == 0)
            {
                const string refNum = li->GetRefNum();
                const string date   = li->GetTransDate();
				
                LineItemRec *gl = 0;
                if (!IsSvcContr)
                	gl = (LineItemRec *)CurrentGroupLead;

                if (gl &&
                    gl->GetRefNum()    == refNum &&
                    gl->GetTransDate() == date)
                {
                    li->SetTankNum(gl->GetTankNum());
                    GroupItems->push_back(li);
                }
                else
                {
                    CurrentGroupLead = 0;
                    LineItems.push_back(li);
                }
            }
            else
            {
                CurrentGroupLead = li;

                switch (li->GetItemType())
                {
                case LineItemRec::Tank:
                    GroupItems = &TankItems[location];
                    GroupAddrs = &TankAddrs;
                    break;
                case LineItemRec::Service:
//                    GroupItems = &ServiceItems[location];
//                    GroupAddrs = &ServiceAddrs;
//                    break;
                case LineItemRec::Account:
                default:
//                    cerr << "Warning: Account type item with a location set: line #" << r->RecordNum << "\n";
                      GroupItems = 0;
                      GroupAddrs = 0;
                    break;
                }
                if (GroupItems)
                    GroupItems->push_back(li);
                else
                {
                    CurrentGroupLead = 0;
                    LineItems.push_back(li);
                }
            }
        }
        break;
    }
    case Record::LineItemAddr:
        if (opt_d || opt_s)
        {
            if (CurrentGroupLead == 0)
            {
                cerr << "Error: LineItemAddr after item with no location, line #" << r->RecordNum << "\n";
                throw;
            }
            else
            {
                LineItemAddrRec *addr = (LineItemAddrRec *)r;
                CurrentGroupLead->SetAddr(addr);
                if (!IsSvcContr)
                {
                	LineItemRec *gl = (LineItemRec *)CurrentGroupLead;
	                AddressMap *addrs = 0;
	                switch (gl->GetItemType())
	                {
	                case LineItemRec::Tank:
	                    addrs = &TankAddrs;
	                    break;
    	            case LineItemRec::Service:
        	            addrs = &ServiceAddrs;
            	        break;
                	case LineItemRec::Account:
	                default:
    	                cerr << "Warning: Account type item with an address record following: line #" << r->RecordNum << "\n";
        	            break;
            	    }
			
	                if (addrs)
    	            {
        	            int location = gl->GetTankNum();
            	        AddressMap::const_iterator i = addrs->find(location);
                	    if (i == addrs->end())
                    	    (*addrs)[location] = addr;
	                    else
    	                {
        	                // check location matches
            	            if ((*addrs)[location]->getAddr() != addr->getAddr())
                	        {
                    	        cerr << "Error: addresses for location #" << location << " don't match, line #" << r->RecordNum << "\n";
                        	    throw;
	                        }
    	                }
	                }
                }
            }
        }
        break;
    case Record::ContractLine:
    	if (IsSvcContr)
            CurrentGroupLead = (ContractLineRec *)r;
    	else
    		cerr << "Error: ContractLine record in Statement file, line #" << r->RecordNum << "\n";
        break;
    case Record::ContractFlags:
    	if (!IsSvcContr)
            cerr << "Error: ContractFlags record in Statement file, line #" << r->RecordNum << "\n";
        break;
    case Record::ContractTaxLine:
        if (!IsSvcContr)
            cerr << "Error: ContractTaxLine record in Statement file, line #" << r->RecordNum << "\n";
        break;
    default:
        cerr << "Error: unkown record type # " << r->Type << " in statement, line #" << r->RecordNum << "\n";
        throw;
        break;
    }
}


class StatementRec::RecordDumper : public unary_function<Record *, void>
{
public:
	RecordDumper(ofstream& out) : os(out) {}
    void operator() (Record *x) { x->dump(os); }
private:
    ofstream& os;
};


inline bool date_order(LineItemRec *r1, LineItemRec *r2)
{
	if (r1->getDate() < r2->getDate())
		return true;
	else
		return false;
}


class StatementRec::RecordDumper2 : public unary_function<LocItemMap::mapped_type, void>
{
public:
	RecordDumper2(ofstream& out, const StatementRec::AddressMap &a):
		os(out), Addrs(a) {}
    void operator() (pair<int,  LocItemMap::mapped_type> x)
    {
    	{
	    	AddressMap::const_iterator i = Addrs.find(x.first);
	    	if (i == Addrs.end())
	    	{
    			cerr << "address not found!?\n";
    			throw;
    		}

	    	LineItemAddrRec *addr = i->second;
	    	addr->dump(os);
    	}
    	
    	RecordDumper dumper(os);
    	stable_sort(x.second.begin(), x.second.end(), date_order);
		for_each(x.second.begin(), x.second.end(), dumper);
    }
private:
    ofstream& os;
    const StatementRec::AddressMap &Addrs;
};


void StatementRec::DumpItems(ofstream &of)
{
	RecordDumper rd(of);
	if (!opt_s)
	{
		for_each(StmtItems.begin(), StmtItems.end(), rd);
	}
	else
	{
		if (StatementAddr != 0) StatementAddr->dump(of);

		stable_sort(LineItems.begin(), LineItems.end(), date_order);
		for_each(LineItems.begin(), LineItems.end(), rd);
	
		if (CreditRec != 0) CreditRec->dump(of);
		if (FinanceChargeRec != 0) FinanceChargeRec->dump(of);
	
		RecordDumper2 td(of, TankAddrs);
		for_each(TankItems.begin(),    TankItems.end(),    td);
		RecordDumper2 sd(of, ServiceAddrs);
		for_each(ServiceItems.begin(), ServiceItems.end(), sd);

		if (DiscountInfoRec != 0) DiscountInfoRec->dump(of);
	}
}


