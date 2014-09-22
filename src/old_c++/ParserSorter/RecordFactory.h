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
extern bool opt_f;
extern bool opt_s;
extern bool opt_i;
extern bool opt_e;
extern bool patricks_special_flag;
extern int  file_ver;
extern string *opt_fa;
extern string *opt_fb;

class PostingCode;
class ContractInfoRec;
class StatementRec;
class LineItemRec;

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
	void FinishPrevStatement(StatementRec *currentStatement, double current_stmt_running_bal,
                                 double credit_li_total, double debit_li_total, double finance_charge);

	bool FilterOut(const StatementRec * const stmt);
	bool FilterOut(const LineItemRec  * const stmt);

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
