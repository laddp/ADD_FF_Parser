#include <math.h>
#include <iostream>
#include <fstream>
#include <iterator>   // for ostreambuf_iterator
using namespace std;

#include "Util.h"
#include "Record.h"
#include "RecordFactory.h"
#include "pml_moneypunct.h"

extern ofstream dumpFile;

RecordFactory *RecordFactory::theRF = 0;

RecordFactory &RecordFactory::getFactory(void)
{
	if (!theRF) theRF = new RecordFactory;
		return *theRF;
}


RecordFactory::RecordFactory():
	Files(0), TotalStatements(0), LineItems(0), TotalPleasePay(0.0), TotalBal(0.0), SpecialCount(0), FinanceCharged(0),TotalFinanceCharge(0.0)
{
	Record::InitStatics();
}


void RecordFactory::PrintCurrency(ostream &os, double value)
{
	long double dv = value * 100; // stupid library! divides by 100 for some reason...
	// Construct a ostreamb12uf_iterator on cout
    typedef std::ostreambuf_iterator<char, std::char_traits<char> > Iter;
    Iter begin(os);
    // Get a money put facet
	const std::money_put<char, Iter> &mp =
		std::use_facet<std::money_put<char, Iter> >(std::locale());

	ios::fmtflags flgs = os.setf(ios_base::showbase|ios_base::internal);
	streamsize orig = os.width(5);
	mp.put(begin, false, os, '0', dv);
	os.width(orig);
	os.flags(flgs);
}


void RecordFactory::DumpDouble(ostream &os, double value,
                               int width, int decPlaces, bool blankIfZero)
{
	double multiplier = pow(10.0, decPlaces);
	double outd = value * multiplier;

	double frac, integral;
	frac = modf(outd, &integral);
	
	int outi = (int)integral;
	if (frac >= 0.5)
		outi += 1;
	if (frac <= -0.5)
		outi -= 1;

	if (blankIfZero && outi == 0)
	{
		for (int i = 0; i < width; i++)
			os << ' ';
	}
	else
	{
		os.width(width);
		os << internal << outi;
		os.width(0);
	}
}


static void CheckEndOfRec(ifstream &ifs, int line)
{
	char eol = ifs.get();
    if (eol != '\n')
    {
    	cerr << "Error: buffer size incorrect, record #" << line << "\n";
    	throw;
    }
}


int RecordFactory::parseStatements(ifstream &stream, const char *fileName,
								   bool printAccts, bool printRec)
{
	int rc = 0;
	
	cout << "============================================================\n"
         << "Processing statement file: " << fileName << '\n';

	int	InfoHeader_cnt = 0;
	int	StmtHeader_cnt = 0;
	int	Division_cnt   = 0;
	int	PostCode_cnt   = 0;
	int	PromptPayMsg_cnt  = 0;
	int	Statement_cnt  = 0;
	int	LineItem_cnt   = 0;
	int	CreditInfo_cnt = 0;
	int	DiscntInfo_cnt = 0;
	int	FinChgInfo_cnt = 0;
	int	MeterInfo_cnt  = 0;
	int	Trailer_cnt    = 0;	
	int FinanceCharged_cnt = 0;
	
	double please_pay_bal = 0.0, stmt_bal = 0.0, finance_charges = 0.0;

    int current_rec = 0;
    bool done = false;

	InfoHeaderRec      *ih = 0;
	DocumentHeaderRec  *dh = 0;
	PromptPayMsgRec    *pp = 0;
	TrailerRec         *tr = 0;

	vector<DivisionRec *> Divisions;
	Divisions.resize(64, 0);
	PcodeMap PostingCodes;

	StatementRec *currentStatement = 0;

	Files++;

	bool dump = true;
	dumpCount = 0;
	dump_stmt_bal = 0.0;
	double current_stmt_running_bal = 0.0;

    do
    {
        // loop increment
        current_rec++;

        // read the data
		char buf[2+1];
        if (!stream.get(buf, sizeof(buf)))
        {
        	if (stream.eof())
        	{
        		done = 1;
        		continue;
        	}
            cerr << "Error: read error, record #" << current_rec << "\n";
            return 1;
        }

		// Report record type
		Record::RecordType rt = (Record::RecordType)atoi(buf);
		if (opt_r)
		{
			if (rt == Record::Statement)
				cout << "*****************************************************************************************\n";				
			cout << "#### Record #" << current_rec << " Type: " << rt << " (" << Record::RT_to_String(rt) << ")\n";
		}

		// Process records
		switch (rt)
		{
		case Record::InfoHeader:
			if (++InfoHeader_cnt > 1)
			{
				cerr << "Error: multiple InfoHeader records, record #" << current_rec << "\n";
				rc++;
			}
			try { ih = new InfoHeaderRec(stream, fileName, current_rec); }
			catch (...) { }
			if (opt_d && dump) ih->dump(dumpFile);
			break;
		case Record::DocHeader:
			if (++StmtHeader_cnt > 1)
			{
				cerr << "Error: multiple StatementHeader records, record #" << current_rec << "\n";
				rc++;
			}
			try { dh = new DocumentHeaderRec(stream, fileName, current_rec, DocumentHeaderRec::Statement); }
			catch (...) { }
			if (opt_d && dump) dh->dump(dumpFile);
			break;
		case Record::Division:
			Division_cnt++;
			try
			{
				DivisionRec *dr = new DivisionRec(stream, fileName, current_rec);
				Divisions[dr->getDivisionNum()] = dr;
				if (opt_d && dump) dr->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::PostCode:
			PostCode_cnt++;
			try
			{
				PostingCode *code = new PostingCode(stream, fileName, current_rec);
				PostingCodes.insert(PcodeMap::value_type(code->getCode(), code));
				if (opt_d && dump) code->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::PromptPayMsg:
			if (++PromptPayMsg_cnt > 1)
			{
				cerr << "Error: multiple PromptPayMsg records, record #" << current_rec << '\n';
				rc++;
			}
			try { pp = new PromptPayMsgRec(stream, fileName, current_rec); }
			catch (...) { }
			if (opt_d && dump) pp->dump(dumpFile);
			break;
		case Record::Statement:
			Statement_cnt++;
			try
			{
				currentStatement = new StatementRec(stream, fileName, current_rec);
				stmt_bal += currentStatement->getCurBal();
				please_pay_bal += currentStatement->getTotalDue();
				if (currentStatement->specialHandling())
					SpecialCount++;

				if (!Divisions[currentStatement->getDivisionNum()])
				{
					cerr << "Error: invalid division, record #" << current_rec << '\n';
				}

				if (opt_d)
			    {
			   		dumpCount++;
			   		dump_stmt_bal += currentStatement->getCurBal();
#ifdef DUMP_IMMED
			   		currentStatement->dump(dumpFile);
#else
					current_stmt_running_bal = currentStatement->getCurBal();
#endif
			    }

				acct_numbers.insert(currentStatement->getAcctNum());
				file_recnum_pair acctrec(fileName, current_rec);
				acct_map.insert(acct_mapping::value_type(currentStatement->getAcctNum(), acctrec));
			}
			catch (...) { }
			break;
		case Record::LineItem:
			LineItem_cnt++;
			if (currentStatement == 0)
			{
				cerr << "Error: Line item not inside a statement, record #" << current_rec << "\n";
				throw;
			}
			try
			{
				LineItemRec *li =
					new LineItemRec(currentStatement, PostingCodes, stream, fileName, current_rec);
				if (opt_d)
				{
				    if(dump)
				    {
					    const char *transDate = li->getDate();
					    char wBuf[3];
					    wBuf[2] = '\0';

					    wBuf[0] = transDate[4];
					    wBuf[1] = transDate[5];
						int year = atoi(wBuf);

					    wBuf[0] = transDate[0];
					    wBuf[1] = transDate[1];
						int month = atoi(wBuf);

					    wBuf[0] = transDate[2];
					    wBuf[1] = transDate[3];
						int day = atoi(wBuf);

					    if (year > 4 &&
					    	((month == 4 && day >= 15) ||
					    	 month >= 5))
						{
#ifdef DUMP_IMMED
							li->dump(dumpFile);
#else
							current_stmt_running_bal -= li->getAmt();
#endif
							currentStatement->AddItem(li);
						}
				    }
				}
				else
					currentStatement->AddItem(li);
			}
			catch (...) { }
			break;
		case Record::CreditInfo:
			CreditInfo_cnt++;
			if (currentStatement == 0)
			{
				cerr << "Error: Credit info not inside a statement, record #" << current_rec << "\n";
				throw;
			}
			try
			{
				CreditInfoRec *ci = new CreditInfoRec(currentStatement, stream, fileName, current_rec);
				currentStatement->AddItem(ci);
				if (ci->getFinanceCharge() > 0.0)
				{
					finance_charges += ci->getFinanceCharge();
					FinanceCharged_cnt++;
				}
				if (opt_d)
				{
					 if (dump)
					 {
#ifdef DUMP_IMMED
						ci->dump(dumpFile);
#else
						// correct previous balance
						current_stmt_running_bal -= ci->getFinanceCharge();
						currentStatement->setPrevBal(current_stmt_running_bal);

						// dump previous records
						currentStatement->dump(dumpFile);
						currentStatement->DumpItems(dumpFile);
#endif
					 }
				}
			}
			catch (...) { }
			break;
		case Record::DiscntInfo:
			DiscntInfo_cnt++;
			if (currentStatement == 0)
			{
				cerr << "Error: Discount info not inside a statement, record #" << current_rec << "\n";
				throw;
			}
			try
			{
				DiscntInfoRec *di = new DiscntInfoRec(currentStatement, stream, fileName, current_rec);
				currentStatement->AddItem(di);
				if (opt_d && dump) di->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::FinChgInfo:
			FinChgInfo_cnt++;
			if (currentStatement == 0)
			{
				cerr << "Error: Finance charge info not inside a statement, record #" << current_rec << "\n";
				throw;
			}
			try
			{
				FinChgInfoRec *fi = new FinChgInfoRec(currentStatement, stream, fileName, current_rec);
				currentStatement->AddItem(fi);
				if (opt_d && dump) fi->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::MeterInfo:
			MeterInfo_cnt++;
			if (currentStatement == 0)
			{
				cerr << "Error: Meter info not inside a statement, record #" << current_rec << "\n";
				throw;
			}
			try
			{
				MeterInfoRec *mi = new MeterInfoRec(currentStatement, stream, fileName, current_rec);
				currentStatement->AddItem(mi);
				if (opt_d && dump) mi->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::LineItemAddr:
			if (opt_old > 0)
			{
				cerr << "Error: LineItemAddrRec present in old format file, record #" << current_rec << "\n";
				throw;
			}
			else
			{
				LineItemAddrRec *lia = new LineItemAddrRec(stream, fileName, current_rec);
				currentStatement->AddItem(lia);
				if (opt_d && dump) lia->dump(dumpFile);
			}
			break;
		case Record::StatementAddr:
			if (opt_old > 0)
			{
				cerr << "Error: StatementAddrRec present in old format file, record #" << current_rec << "\n";
				throw;
			}
			else
			{
				StatementAddrRec *sa = new StatementAddrRec(stream, fileName, current_rec);
				currentStatement->AddItem(sa);
				if (opt_d && dump) sa->dump(dumpFile);
			}
			break;
		case Record::Trailer:
			if (++Trailer_cnt > 1)
			{
				cerr << "Error: multiple Trailer records, record #" << current_rec << "\n";
				rc++;
			}
			try
			{
				tr = new TrailerRec(stream, fileName, current_rec);
				if (opt_d)
				{
					tr->dumpNew(dumpFile, dumpCount, dump_stmt_bal);
				}
			}
			catch (...) { }
			break;
		default:
			cerr << "Error: invalid record type " << rt << ", record #" << current_rec << "\n";
			throw;
			break;
		}
		CheckEndOfRec(stream, current_rec);
    } while (!done);


	// Check for missing records
	if (InfoHeader_cnt != 1)
	{
		cerr << "Error: missing InfoHeader\n";
		rc++;
	}
	
	if (StmtHeader_cnt != 1)
	{
		cerr << "Error: missing StmtHeader\n";
		rc++;
	}
	
	if (PromptPayMsg_cnt != 1)
	{
		cerr << "Error: missing PromptPayMsg header\n";
		rc++;
	}
	
	if (Trailer_cnt != 1)
	{
		cerr << "Error: missing Trailer\n";
		rc++;
	}

	if (Statement_cnt != CreditInfo_cnt)
	{
		cerr << "Error: number of statements does not match number of credit infos\n";
		rc++;
	}

	if (Statement_cnt != DiscntInfo_cnt)
	{
		cerr << "Error: number of statements does not match number of discount infos\n";
		rc++;
	}

	if (FinanceCharged_cnt != FinChgInfo_cnt)
	{
		cerr << "Error: number of statements with finance charges does not match number of finance charge infos\n";
		rc++;
	}

	if (opt_r)
		cout << "\nRecord Counts:\n"
		<< "InfoHeader: " << InfoHeader_cnt   << "\n"
		<< "StmtHeader: " << StmtHeader_cnt   << "\n"
		<< "Division:   " << Division_cnt     << "\n"
		<< "PostCode:   " << PostCode_cnt     << "\n"
		<< "PromptPay:  " << PromptPayMsg_cnt << "\n"
		<< "Statement:  " << Statement_cnt    << "\n"
		<< "LineItem:   " << LineItem_cnt     << "\n"
		<< "CreditInfo: " << CreditInfo_cnt   << "\n"
		<< "DiscntInfo: " << DiscntInfo_cnt   << "\n"
		<< "FinChgInfo: " << FinChgInfo_cnt   << "\n"
		<< "MeterInfo:  " << MeterInfo_cnt    << "\n"
		<< "Trailer:    " << Trailer_cnt      << "\n";

	// Verify counts & totals
	if (tr->getTotalItems() != Statement_cnt)
	{
		cerr << "Error: count of statements (" << Statement_cnt
			 << ") does not match count in trailer record (" << tr->getTotalItems()
			 << ")\n";
		rc++;
	}

	if ((tr->getTotalBilled() - please_pay_bal) >= 0.001)
	{
		cerr << "Error: total billed in trailer record (";
		PrintCurrency(cerr, tr->getTotalBilled());
		cerr << ") does not match total of statement balances (";
		PrintCurrency(cerr, please_pay_bal);
		cerr << ")\n";
		rc++;
	}

	TotalPleasePay += please_pay_bal;
	TotalBal += stmt_bal;
	TotalStatements += Statement_cnt;
	FinanceCharged += FinanceCharged_cnt;
	TotalFinanceCharge += finance_charges;

    cout << "\nTotals:\n"
         << "Please pay:      ";
    PrintCurrency(cout, please_pay_bal);
    cout << '\n'
         << "Statement bal:   ";
    PrintCurrency(cout, stmt_bal);
    cout << '\n'
         << "Statements:      " << Statement_cnt << '\n'
         << "Finance charged: " << FinanceCharged_cnt << '\n'
         << "Finance charges: ";
    PrintCurrency(cout, finance_charges);
    cout << '\n'
         << "============================================================\n\n";

    return rc;
}


int RecordFactory::parseServiceContracts(ifstream &stream, const char *fileName,
                                         bool printAccts, bool printRec)
{
	int rc = 0;
	
	cout << "============================================================\n"
         << "Processing service contract file: " << fileName << '\n';

	int	InfoHeader_cnt     = 0;
	int	StmtHeader_cnt     = 0;
	int special_cnt        = 0;
	int	Division_cnt       = 0;
	int	PostCode_cnt       = 0;
	int	ContractInfo_cnt   = 0;
	int	Statement_cnt      = 0;
	int	LineItem_cnt       = 0;
	int	TaxLineItem_cnt    = 0;
	int ContrFlags_cnt     = 0;
	int	Trailer_cnt        = 0;	

	double please_pay_bal = 0.0;

    int current_rec = 0;
    bool done = false;

	InfoHeaderRec      *ih = 0;
	DocumentHeaderRec  *dh = 0;
	TrailerRec         *tr = 0;

	vector<DivisionRec *> Divisions;
	Divisions.resize(64, 0);
	PcodeMap PostingCodes;
	ContractInfoMap ContractInfo;

	StatementRec *currentStatement = 0;

	Files++;

	bool dump = true;
	dumpCount = 0;
	dump_stmt_bal = 0.0;

    do
    {
        // loop increment
        current_rec++;

        // read the data
		char buf[2+1];
        if (!stream.get(buf, sizeof(buf)))
        {
        	if (stream.eof())
        	{
        		done = 1;
        		continue;
        	}
            cerr << "Error: read error, record #" << current_rec << "\n";
            return 1;
        }

		// Report record type
		Record::RecordType rt = (Record::RecordType)atoi(buf);
		if (opt_r)
		{
			if (rt == Record::Statement)
				cout << "*****************************************************************************************\n";				
			cout << "#### Record #" << current_rec << " Type: " << rt << " (" << Record::RT_to_String(rt) << ")\n";
		}

		// Process records
		switch (rt)
		{
		case Record::InfoHeader:
			if (++InfoHeader_cnt > 1)
			{
				cerr << "Error: multiple InfoHeader records, record #" << current_rec << "\n";
				rc++;
			}
			try { ih = new InfoHeaderRec(stream, fileName, current_rec); }
			catch (...) { }
			if (opt_d && dump) ih->dump(dumpFile);
			break;
		case Record::DocHeader:
			if (++StmtHeader_cnt > 1)
			{
				cerr << "Error: multiple StatementHeader records, record #" << current_rec << "\n";
				rc++;
			}
			try { dh = new DocumentHeaderRec(stream, fileName, current_rec, DocumentHeaderRec::SvcContr); }
			catch (...) { }
			if (opt_d && dump) dh->dump(dumpFile);
			break;
		case Record::Division:
			Division_cnt++;
			try
			{
				DivisionRec *dr = new DivisionRec(stream, fileName, current_rec);
				Divisions[dr->getDivisionNum()] = dr;
				if (opt_d && dump) dr->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::PostCode:
			PostCode_cnt++;
			try
			{
				PostingCode *code = new PostingCode(stream, fileName, current_rec);
				PostingCodes.insert(PcodeMap::value_type(code->getCode(), code));
				if (opt_d && dump) code->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::ContractInfo:
			ContractInfo_cnt++;
			try
			{
				ContractInfoRec *info = new ContractInfoRec(stream, fileName, current_rec);
				ContractInfo.insert(ContractInfoMap::value_type(info->getContr(), info));
				if (opt_d && dump) info->dump(dumpFile);
			}
			catch (...) { }
			break;
		case Record::Statement:
			Statement_cnt++;
			try
			{
				currentStatement = new StatementRec(stream, fileName, current_rec, true);
				if (currentStatement->specialHandling())
					special_cnt++;

				if (!Divisions[currentStatement->getDivisionNum()])
				{
					cerr << "Error: invalid division, record #" << current_rec << '\n';
				}

				if (opt_d)
			    {
			    }

				acct_numbers.insert(currentStatement->getAcctNum());
				file_recnum_pair acctrec(fileName, current_rec);
				acct_map.insert(acct_mapping::value_type(currentStatement->getAcctNum(), acctrec));
			}
			catch (...) { }
			break;
		case Record::ContractLine:
			LineItem_cnt++;
			if (currentStatement == 0)
			{
				cerr << "Error: Contract line not inside a statement, record #" << current_rec << "\n";
				throw;
			}
			try
			{
				ContractLineRec *li =
					new ContractLineRec(currentStatement, PostingCodes, ContractInfo,
					                    stream, fileName, current_rec);
				currentStatement->AddItem(li);
			}
			catch (...) { }
			break;
		case Record::ContractTaxLine:
			TaxLineItem_cnt++;
			if (currentStatement == 0)
			{
				cerr << "Error: Contract tax line not inside a statement, record #" << current_rec << "\n";
				throw;
			}
			try
			{
				ContractTaxLineRec *li =
					new ContractTaxLineRec(currentStatement, stream, fileName, current_rec);
				currentStatement->AddItem(li);

				please_pay_bal += li->getTotal();
			}
			catch (...) { }
			break;
		case Record::ContractFlags:
			ContrFlags_cnt++;
			{
				ContractFlagsRec *flgs = new ContractFlagsRec(currentStatement, stream, fileName, current_rec);
				currentStatement->AddItem(flgs);
			}
			break;
		case Record::LineItemAddr:
			if (opt_old > 0)
			{
				cerr << "Error: LineItemAddrRec present in old format file, record #" << current_rec << "\n";
				throw;
			}
			else
			{
				LineItemAddrRec *lia = new LineItemAddrRec(stream, fileName, current_rec);
				currentStatement->AddItem(lia);
				if (opt_d && dump) lia->dump(dumpFile);
			}
			break;
		case Record::StatementAddr:
			if (opt_old > 0)
			{
				cerr << "Error: StatementAddrRec present in old format file, record #" << current_rec << "\n";
				throw;
			}
			else
			{
				StatementAddrRec *sa = new StatementAddrRec(stream, fileName, current_rec);
				currentStatement->AddItem(sa);
				if (opt_d && dump) sa->dump(dumpFile);
			}
			break;
		case Record::Trailer:
			if (++Trailer_cnt > 1)
			{
				cerr << "Error: multiple Trailer records, record #" << current_rec << "\n";
				rc++;
			}
			try
			{
				tr = new TrailerRec(stream, fileName, current_rec);
				if (opt_d)
				{
					tr->dumpNew(dumpFile, dumpCount, dump_stmt_bal);
				}
			}
			catch (...) { }
			break;
		default:
			cerr << "Error: invalid record type " << rt << ", record #" << current_rec << "\n";
			throw;
			break;
		}

		CheckEndOfRec(stream, current_rec);
    } while (!done);

	// Check for missing records
	if (InfoHeader_cnt != 1)
	{
		cerr << "Error: missing InfoHeader\n";
		rc++;
	}
	
	if (StmtHeader_cnt != 1)
	{
		cerr << "Error: missing StmtHeader\n";
		rc++;
	}
	
	if (Trailer_cnt != 1)
	{
		cerr << "Error: missing Trailer\n";
		rc++;
	}

	if (opt_r)
		cout << "\nRecord Counts:\n"
		<< "InfoHeader: " << InfoHeader_cnt   << "\n"
		<< "StmtHeader: " << StmtHeader_cnt   << "\n"
		<< "Division:   " << Division_cnt     << "\n"
		<< "PostCode:   " << PostCode_cnt     << "\n"
		<< "ContrInfo:  " << ContractInfo_cnt << "\n"
		<< "Statement:  " << Statement_cnt    << "\n"
		<< "LineItem:   " << LineItem_cnt     << "\n"
		<< "ContrTax:   " << TaxLineItem_cnt  << "\n"
		<< "ContrFlags: " << ContrFlags_cnt   << "\n"
		<< "Trailer:    " << Trailer_cnt      << "\n";

	// Verify counts & totals
	if (tr->getTotalItems() != Statement_cnt)
	{
		cerr << "Error: count of statements (" << Statement_cnt
			 << ") does not match count in trailer record (" << tr->getTotalItems()
			 << ")\n";
		rc++;
	}

	if ((tr->getTotalBilled() - please_pay_bal) >= 0.001)
	{
		cerr << "Error: total billed in trailer record (";
		PrintCurrency(cerr, tr->getTotalBilled());
		cerr << ") does not match total of statement balances (";
		PrintCurrency(cerr, please_pay_bal);
		cerr << ")\n";
		rc++;
	}

	cout << "Total Statements:        " << Statement_cnt << '\n'
	     << "Total Contracts:         " << LineItem_cnt << '\n'
	     << "Total Special handling:  " << special_cnt << '\n'
		 << "Total Please Pay:        ";
	PrintCurrency(cout, please_pay_bal);
    cout << '\n'
         << "============================================================\n\n";

	LineItems += LineItem_cnt;
	SpecialCount += special_cnt;
	TotalPleasePay += please_pay_bal;
	TotalStatements += Statement_cnt;

	return rc;
}



void print_file_recnum(RecordFactory::acct_mapping::const_iterator i)
{
    cout << "\t  f: " << i->second.first << " r: " << i->second.second << '\n';
}


class find_dups
{
private:
    int count;
    int num_dups;

    int num_doubles;
    int num_triples;
    int num_more;

    friend ostream &operator<<(ostream &os, const find_dups fd);
    
    RecordFactory::acct_mapping acct_map;
public:
    find_dups(RecordFactory::acct_mapping m):
    	count(0), num_dups(0), num_doubles(0), num_triples(0), num_more(0), acct_map(m) { }

    void operator()(const int i)
    {
        int c = acct_map.count(i);
        if (c > 1)
        {
            count++;
            num_dups += (c - 1);
            if (c == 2) num_doubles++;
            if (c == 3) num_triples++;
            if (c > 3)  num_more++;

            pair<RecordFactory::acct_mapping::const_iterator, 
            	 RecordFactory::acct_mapping::const_iterator> range =
                acct_map.equal_range(i);

            cout << "Duplicate: acct# " << i << ", count = " << c << '\n';

            for (RecordFactory::acct_mapping::const_iterator iter = range.first; iter != range.second; iter++)
            {
                print_file_recnum(iter);
            }
        }
    }
};


ostream &operator<<(ostream &os, const find_dups fd)
{
    os << "duplicate accounts: " << fd.count  << "\tduplicate count: " << fd.num_dups
       << "\ndoubles: " << fd.num_doubles << "\ttriples: " << fd.num_triples
       << "\tmore than 3: " << fd.num_more << '\n';
    return os;
}


void RecordFactory::CheckForDups(void)
{
	find_dups f(acct_map);
    f = for_each(acct_numbers.begin(), acct_numbers.end(), f);
    cout << f << '\n';
}


void RecordFactory::PrintInfo(void) const
{
	cout << "Statements summary:\n"
		 << "Total Files:             " << Files << '\n'
	     << "Total Statements:        " << TotalStatements << '\n'
	     << "Total Line items:        " << LineItems << '\n'
	     << "Total Special handling:  " << SpecialCount << '\n'
	     << "Total /w Finance charge: " << FinanceCharged << '\n'
	     << "Total Finance Charges:   ";
	PrintCurrency(cout, TotalFinanceCharge);	
	cout << '\n'
		 << "Total Please Pay:        ";
	PrintCurrency(cout, TotalPleasePay);
	cout << '\n'
	     << "Total Balance:           ";
	PrintCurrency(cout, TotalBal);
	cout << '\n';
}
