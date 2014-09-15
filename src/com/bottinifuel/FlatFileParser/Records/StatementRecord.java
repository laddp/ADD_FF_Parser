/*
 * Created on Jun 14, 2010 by pladd
 *
 * Change Log:
 * 
 *   Date         Description                                        Pgmr
 *  ------------  ------------------------------------------------   -----
 *  Aug 12, 2014  Record length changed for ADDs 14 upgrade.        carlonc
 *                Look for comment 081214
 ***************************************************************************************/   
package com.bottinifuel.FlatFileParser.Records;

import java.math.BigDecimal;
import java.util.Collection;
import java.util.Date;
import java.util.LinkedList;

import com.bottinifuel.FlatFileParser.FileFormatException;
import com.bottinifuel.FlatFileParser.FlatFile;
import com.bottinifuel.FlatFileParser.FlatFile.FileTypeEnum;

/**
 * @author pladd
 *
 */
public class StatementRecord extends DocumentRecord
{
    public final int        BudgetStartMonth;
    public final BigDecimal BudgetPayment;
    public final int        NumBudgetPayments;
    public final BigDecimal PastDueBudgetAmount;
    
    public final BigDecimal PrepayBudgetCredit;
    public final BigDecimal NonBudgetCharges;
    
    public final String     StatementType;
    public final String     LDC;
    public final String     DocumentRefNum;
    public final String     Rest;                  // 081214
    public final char       CaliforniaIndicator;
    public final char       MiscallaneousFlag;
    public final char       DutyToWarn;
    
    public final BigDecimal AlternateTotalDue;
    public final int        SMC;
    public final char       Language;
    
    public final BigDecimal CreditDollarLimit;
    public final Date       LastStatementDate;
    public final boolean    NoLastStatement;
    public final int        CollectorNumber;
    
    public final BigDecimal PrepaymentAmount;
    public final boolean    ServiceContractInBudget;
    public final int        StatementDay;
        
    public final String     ElectronicDeliveryInfo;

    public CreditInfoRecord CreditInfoRecord = null;
    public FinanaceChargeInfoRecord FinanceChargeInfo = null;

    public StatementRecord(String data, int recNum) throws FileFormatException
    {
        super(FileTypeEnum.STATEMENT_FILE, recNum, data, 629+22);   // 081214
                
        BudgetStartMonth    = Integer.parseInt(data.substring(159, 161) .trim());
        BudgetPayment       = new BigDecimal(  data.substring(161, 170)).movePointLeft(2);
        NumBudgetPayments   = Integer.parseInt(data.substring(170, 172) .trim());
        PastDueBudgetAmount = new BigDecimal(  data.substring(172, 181)).movePointLeft(2);
        
        PrepayBudgetCredit = new BigDecimal(data.substring(190, 199)).movePointLeft(2);
        NonBudgetCharges   = new BigDecimal(data.substring(199, 208)).movePointLeft(2);
        
        StatementType       = data.substring(255, 257);
        LDC                 = data.substring(258, 262);
        DocumentRefNum      = data.substring(262, 271);
        CaliforniaIndicator = data.charAt(273);
        MiscallaneousFlag   = data.charAt(274);
        DutyToWarn          = data.charAt(275);

        AlternateTotalDue = new BigDecimal(data.substring(276, 285)).movePointLeft(2);
        SMC               = Integer.parseInt(data.substring(286, 290).trim());
        Language          = data.charAt(298);
        
        CreditDollarLimit = new BigDecimal(  data.substring(299, 310)).movePointLeft(2);
        if (data.substring(310, 316).equals("000000"))
        {
            NoLastStatement = true;
            LastStatementDate = null;
        }
        else
        {
            NoLastStatement = false;
            LastStatementDate = FormatDate(data.substring(310, 316), recNum);
        }
        CollectorNumber   = Integer.parseInt(data.substring(316, 318).trim());
        
        PreviousBalance  = new BigDecimal(data.substring(318, 330)).movePointLeft(2);
        CurrentBalance   = new BigDecimal(data.substring(330, 342)).movePointLeft(2);
        PrepaymentAmount = new BigDecimal(data.substring(342, 351)).movePointLeft(2);
        
        ServiceContractInBudget = ReadFlag(data.charAt(373), "Y", "N", recNum, "ServiceContractInBudget");

        ElectronicDeliveryInfo = data.substring(374, 624);
        StatementDay = Integer.parseInt(data.substring(626, 629));
        Rest = data.substring(629,651); // 081214
    }


    public String dump()
    {
        String rc = super.dump();
        rc +=
            String.format("%-6s", OldAccountNum) +
            String.format("%02d", OldDivision) +
            String.format("%1d",  Type) +
            String.format("%-30s", Name) +
            String.format("%-30s", Address1) +
            String.format("%-30s", Address2) +
            String.format("%-20s", Town) +
            String.format("%-10s", State) +
            String.format("%-9s",  Zip) +
            DecimalFormat(9, OldPreviousBalance.movePointRight(2)) +
            DecimalFormat(9, OldCurrentBalance .movePointRight(2)) +
            String.format("%2d", BudgetStartMonth) +
            DecimalFormat(9, BudgetPayment.movePointRight(2)) +
            String.format("%2d", NumBudgetPayments) +
            DecimalFormat(9, PastDueBudgetAmount.movePointRight(2)) +
            DecimalFormat(9, TotalDue.movePointRight(2)) +
            DecimalFormat(9, PrepayBudgetCredit.movePointRight(2)) +
            DecimalFormat(9, NonBudgetCharges.movePointRight(2)) +
            OCRLine +
            StatementType +
            OpenItemFlag +
            LDC +
            DocumentRefNum +
            String.format("%02d", OldCompanyCode) +
            CaliforniaIndicator +
            MiscallaneousFlag +
            DutyToWarn +
            DecimalFormat(9, AlternateTotalDue) +
            (isSpecialHandling() ? "Y" : "N") +
            String.format("%4d", SMC) +
            DirectDebit +
            String.format("%-7s", AccountNum) +
            Language +
            DecimalFormat(11, CreditDollarLimit.movePointRight(2)) +
            (NoLastStatement ? "000000" : FormatDate(LastStatementDate)) +
            String.format("%02d", CollectorNumber) +
            DecimalFormat(12, PreviousBalance.movePointRight(2)) +
            DecimalFormat(12, CurrentBalance.movePointRight(2)) +
            DecimalFormat(9, PrepaymentAmount.movePointRight(2)) +
            String.format("%04d", Division) +
            String.format("%04d", CompanyCode) +
            DocumentViewerID +
            String.format("%04d", CategoryCode) +
            (ServiceContractInBudget ? "Y" : "N") +
            ElectronicDeliveryInfo +
            String.format("%02d", FinanceChargeGroup) +
            String.format("%03d", StatementDay)+
            String.format("%-22s", Rest); // 081214
            
        return rc;
    }
    
    public BigDecimal CombineFinanceCharges(Date financeChargeDate, int posting_code, FlatFile ff)
    {
    	Collection <Record> removeList = new LinkedList<Record>();
    	BigDecimal rc = null;
    	
    	String refNum = "9999";

    	for (Record r : Records)
        {
            if (r instanceof LineItemRecord)
            {
            	LineItemRecord li = (LineItemRecord)r;
            	if (li.TransCode == posting_code &&
            		li.Dollars.compareTo(BigDecimal.ZERO) > 0 &&
            		li.TransDate.equals(financeChargeDate))
            	{
            		CreditInfoRecord.setFinanceCharge(CreditInfoRecord.getFinanceCharge().add(li.Dollars));
            		refNum = li.RefNum;
            		
            		removeList.add(li);
            	
            		if (rc == null)
            			rc = li.Dollars;
            		else
            			rc = rc.add(li.Dollars);
            	}
            }
        }

    	if (removeList.size() > 0)
    	{
    		ff.RemoveLineItemsNoBalanceAdjust(this, removeList);
    		if (FinanceChargeInfo == null)
    		{
				try {
	    			FinanaceChargeInfoRecord fi = new FinanaceChargeInfoRecord(ff.DocumentHeader.DocumentDate, refNum);
	    			Record lastRecord = Records.get(Records.size() - 1);
	    			int insertLoc = ff.AllRecords.indexOf(lastRecord) + 1;
	    			ff.AllRecords.add(insertLoc, fi);
	    			FinanceChargeInfo = fi;
	    			Records.add(fi);
				} catch (FileFormatException e) {
					e.printStackTrace();
					System.exit(1);
				}
    		}
    	}
    	
    	return rc;
    }
}
