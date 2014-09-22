#ifndef RECORDFACTORY_H
#define RECORDFACTORY_H
#include <fstream>
#include <map>
#include <set>
#include <string>
using namespace std;

extern bool opt_a;
extern bool opt_n;
extern bool opt_r;
extern bool opt_d;
extern int  opt_old;

class PostingCode;
class ContractInfoRec;

class RecordFactory{
public:
	static RecordFactory &getFactory(void);

	int parseStatements(ifstream &stmtfile, const char *fileName,
						bool printAccts = false, bool printRec = false);

	int parseServiceContracts(ifstream &stmtfile, const char *fileName,
						bool printAccts = false, bool printRec = false);

	typedef pair<const char *, const int> file_recnum_pair;
	typedef multimap<const int, file_recnum_pair> acct_mapping;

	typedef map<string, PostingCode *> PcodeMap;
	typedef map<string, ContractInfoRec *> ContractInfoMap;

	void CheckForDups(void);

	void PrintInfo(void) const;
	
	static void PrintCurrency(ostream &os, double value);
	static void DumpDouble(ostream &os, double value,
	                       int width = 9, int decPlaces = 2,
	                       bool blankIfZero = false);
private:
	RecordFactory();
	
	static RecordFactory *theRF;

	set<int> acct_numbers;
	acct_mapping acct_map;
	
	int Files;
	int TotalStatements;
	int LineItems;
	double TotalPleasePay;
	double TotalBal;
	int SpecialCount;
	int FinanceCharged;
	double TotalFinanceCharge;
	
	int dumpCount;
	double dump_stmt_bal;
};

#endif // RECORDFACTORY_H
