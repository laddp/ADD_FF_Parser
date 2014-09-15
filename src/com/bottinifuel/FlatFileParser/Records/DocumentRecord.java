/*
 * Created on Nov 19, 2010 by pladd
 */
/*************************************************************************************
* Change Log:
* 
*   Date         Description                                        Pgmr
*  ------------  ------------------------------------------------   -----
*  Aug 22, 2014  Record length changed for ADDs 14 upgrade.        carlonc
*                Look for comment 082214
***************************************************************************************/ 
package com.bottinifuel.FlatFileParser.Records;

import java.math.BigDecimal;
import java.text.NumberFormat;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.bottinifuel.FlatFileParser.FileFormatException;
import com.bottinifuel.FlatFileParser.FlatFile.FileTypeEnum;


public abstract class DocumentRecord extends Record
{

    protected String Data;
    
    public final String OldAccountNum;
    public final int    OldDivision;
    public final int    Type;
    
    public final String Name;
    public final String Address1;
    public final String Address2;
    public final String Town;
    public final String State;
    public final String Zip;
    
    protected BigDecimal OldPreviousBalance;
    protected BigDecimal OldCurrentBalance;
    protected BigDecimal TotalDue;
    
    public final String  OCRLine;
    public final char    OpenItemFlag;
    public final int     OldCompanyCode;
    private boolean      SpecialHandling;
    public final char    DirectDebit;
    
    public final int     AccountNum;
    protected BigDecimal PreviousBalance;
    protected BigDecimal CurrentBalance;
    public final int     Division;
    public final int     CompanyCode;
    public final String  DocumentViewerID;
    public final int     CategoryCode;
    public final int     FinanceChargeGroup;
    public final String  Other; //082214
    public final String  Rest; //082214
    
    public List<Record> Records = new ArrayList<Record>();

    public DocumentRecord(FileTypeEnum ft, int recNum,
                          String data, int expected_len) throws FileFormatException
    {
        super(RecordTypeEnum.STATEMENT, recNum, data, expected_len);

        Data = data.substring(3);

        OldAccountNum = data.substring( 3, 9).trim();
        OldDivision   = Integer.parseInt(data.substring( 9,11).trim());
        Type          = Integer.parseInt(data.substring(11,12).trim());
        
        Name     = data.substring( 12,  42).trim();
        if (data.charAt(42) == ' ')
            Address1 = " " + data.substring( 42,  72).trim();
        else
            Address1 = data.substring( 42,  72).trim();
        Address2 = data.substring( 72, 102).trim();
        Town     = data.substring(102, 122).trim();
        State    = data.substring(122, 132).trim();
        Zip      = data.substring(132, 141).trim();
        
        OldPreviousBalance = new BigDecimal(data.substring(141, 150)).movePointLeft(2);
        OldCurrentBalance  = new BigDecimal(data.substring(150, 159)).movePointLeft(2);
        TotalDue           = new BigDecimal(data.substring(181, 190)).movePointLeft(2);

        OCRLine      = data.substring(208, 255);
        OpenItemFlag = data.charAt(   257);

        OldCompanyCode = Integer.parseInt(data.substring(271, 273).trim());
        
        SpecialHandling = ReadFlag(data.charAt(285), "Y", "N", recNum, "SpecialHandling");
        Other = data.substring(286, 290);  // 082214
        DirectDebit  = data.charAt(290);
        AccountNum   = Integer.parseInt(data.substring(291, 298).trim());

        if (ft == FileTypeEnum.STATEMENT_FILE)
        {
            Division           = Integer.parseInt(data.substring(351, 355));
            CompanyCode        = Integer.parseInt(data.substring(355, 359));
            DocumentViewerID   =                  data.substring(359, 369);
            CategoryCode       = Integer.parseInt(data.substring(369, 373));
            FinanceChargeGroup = Integer.parseInt(data.substring(624, 626));
        }
        else
        {
            Division           = Integer.parseInt(data.substring(298, 302));
            CompanyCode        = Integer.parseInt(data.substring(302, 306));
            DocumentViewerID   =                  data.substring(306, 316);
            CategoryCode       = Integer.parseInt(data.substring(316, 320));
            FinanceChargeGroup = Integer.parseInt(data.substring(320, 322));
        }
        Rest = data.substring(323).trim();  // 082214
    }


    public String dump()
    {
        return super.dump();
    }

    /**
     * Only use this function if you are making other changes that have a net zero effect on
     * the starting and ending balances on the statement
     * (i.e. changing a line item to a finance charge item or replacing with another line item)
     */
    public void RemoveLineItemsNoBalanceAdjust(Collection <Record> remove)
    {
    	Records.remove(remove);
    }
    
    public BigDecimal RemoveNewLineItems(Collection<Record> removeList)
    {
        BigDecimal totalRemoved         = new BigDecimal(0);
        BigDecimal totalBillableRemoved = new BigDecimal(0);
    
        for (Record r : removeList)
        {
            if (r instanceof LineItemRecord)
            {
                LineItemRecord li = (LineItemRecord)r;
                
                if (OpenItemFlag != 'K' || li.Keyoff == true)
                {
                    if (li.BudgetableChar != 'Y')
                        totalBillableRemoved = totalBillableRemoved.add(li.Dollars);
                    totalRemoved = totalRemoved.add(li.Dollars);
                }
            }
        }
        Records.remove(removeList);
        
        NumberFormat currency = DecimalFormat.getCurrencyInstance();
        System.out.println("Acct#" + AccountNum + " - " + Name + "\n" +
                           "Dropped " + removeList.size() +
                           " lines totalling " + currency.format(totalRemoved) +
                           " (billable total " + currency.format(totalBillableRemoved) + ")\n" +
                           "Current Balance was: " + currency.format(CurrentBalance) + "\n" +
                           "Total Due was:       " + currency.format(TotalDue));
        CurrentBalance    = CurrentBalance.subtract(totalRemoved);
        OldCurrentBalance = OldCurrentBalance.subtract(totalRemoved);
    
        TotalDue = TotalDue.subtract(totalBillableRemoved);
        if (TotalDue.signum() == -1)
            TotalDue = new BigDecimal(0);
    
        System.out.println("New Balance:         " + currency.format(CurrentBalance) + "\n" +
                           "New Total Due:       " + currency.format(TotalDue));
        
        return totalRemoved;
    }


    public void RemoveOldLineItems(Collection<Record> removeList)
    {
        BigDecimal totalRemoved         = new BigDecimal(0);
        BigDecimal totalBillableRemoved = new BigDecimal(0);
    
        for (Record r : removeList)
        {
            if (r instanceof LineItemRecord)
            {
                LineItemRecord li = (LineItemRecord)r;
                
                if (OpenItemFlag != 'K' || li.Keyoff == true)
                {
                    if (li.BudgetableChar != 'Y')
                        totalBillableRemoved = totalBillableRemoved.add(li.Dollars);
                    totalRemoved = totalRemoved.add(li.Dollars);
                }
            }
        }
        Records.remove(removeList);
        
        NumberFormat currency = DecimalFormat.getCurrencyInstance();
        System.out.println("Acct#" + AccountNum + " - " + Name + "\n" +
                           "Dropped " + removeList.size() +
                           " lines totalling " + currency.format(totalRemoved) +
                           " (billable total " + currency.format(totalBillableRemoved) + ")\n" +
                           "Previous Balance was: " + currency.format(PreviousBalance));
        PreviousBalance    = PreviousBalance.add(totalRemoved);
    
        System.out.println("New Previous Balance: " + currency.format(PreviousBalance));
    }


    public String toString()
    {
        return "Record #" + RecordNum + " - Statement - " + Name; 
    }


    /**
     * @return the currentBalance
     */
    public BigDecimal getCurrentBalance()
    {
        return CurrentBalance;
    }


    /**
     * @return the totalDue
     */
    public BigDecimal getTotalDue()
    {
        return TotalDue;
    }


    /**
     * @return the previousBalance
     */
    public BigDecimal getPreviousBalance()
    {
        return PreviousBalance;
    }


	/**
	 * @return the specialHandling
	 */
	public boolean isSpecialHandling() {
		return SpecialHandling;
	}


	/**
	 * @param specialHandling the specialHandling to set
	 */
	public void setSpecialHandling(boolean specialHandling) {
		SpecialHandling = specialHandling;
	}

}