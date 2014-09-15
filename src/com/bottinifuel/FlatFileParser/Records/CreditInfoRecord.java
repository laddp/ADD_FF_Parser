/*
 * Created on Sep 1, 2010 by pladd
 *
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
import java.util.Date;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public class CreditInfoRecord extends Record
{
    private      BigDecimal FinanceCharge;
	public final BigDecimal BudgetInterest;
    
	public final BigDecimal OldCurrentDollars;
    public final BigDecimal OldPastDue1;
    public final BigDecimal OldPastDue2;
    public final BigDecimal OldPastDue3;
    public final BigDecimal OldPastDue4;

    public final String     DADShort;
    public final String     DunningMsg;
    public final String     SummaryMsg;
       
    public final BigDecimal RateAnnual;
    public final BigDecimal RateMonthly;
    public final BigDecimal AvgDailyBalance;

    public final Date       FinanceChargeDate;
    public final Date       FinanceChargeDue;
    
    public final BigDecimal LateFee;
    
    public final BigDecimal CurrentDollars;
    public final BigDecimal PastDue1;
    public final BigDecimal PastDue2;
    public final BigDecimal PastDue3;
    public final BigDecimal PastDue4;
    public final BigDecimal Rest;     // 081214
    
    public final BigDecimal PPGallonsRem;

    public CreditInfoRecord(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.CREDIT_INFO, recNum, data, 1896+10); // 081214

        FinanceCharge     = new BigDecimal(data.substring(  3,    12)).movePointLeft(2);
        BudgetInterest    = new BigDecimal(data.substring(  12,   21)).movePointLeft(2);

        OldCurrentDollars = new BigDecimal(data.substring(  21,   30)).movePointLeft(2);
        OldPastDue1       = new BigDecimal(data.substring(  30,   39)).movePointLeft(2);
        OldPastDue2       = new BigDecimal(data.substring(  39,   48)).movePointLeft(2);
        OldPastDue3       = new BigDecimal(data.substring(  48,   57)).movePointLeft(2);
        OldPastDue4       = new BigDecimal(data.substring(  57,   66)).movePointLeft(2);

        String gallonsRem =                data.substring(  66,   75).trim();
        if (gallonsRem.length() > 0)
        	PPGallonsRem = new BigDecimal(gallonsRem).movePointLeft(1);
        else
        	PPGallonsRem = null;
        
        DADShort          =                data.substring(  75,  135);
        DunningMsg        =                data.substring( 135,  263);
        SummaryMsg        =                data.substring( 263, 1799);

        String ratea =                     data.substring(1799, 1803).trim();
        if (ratea.length() > 0)
        	RateAnnual    = new BigDecimal(ratea).movePointLeft(2);
        else
        	RateAnnual    = null;
        
        String ratem =                     data.substring(1803, 1807).trim();
        if (ratem.length() > 0)
        	RateMonthly     = new BigDecimal(ratem).movePointLeft(2);
        else
        	RateMonthly     = null;
        
        String avgd =                     data.substring(1807, 1816).trim();
        if (avgd.length() > 0)
            AvgDailyBalance = new BigDecimal(avgd).movePointLeft(2);
        else
        	AvgDailyBalance = null;

        String fchgdt =                   data.substring(1816, 1822).trim();
        if (fchgdt.length() > 0 && !fchgdt.equals("000000"))
        	FinanceChargeDate = FormatDate(fchgdt, recNum);
        else
        	FinanceChargeDate = null;
        
        String fchgdue =                   data.substring(1822, 1828).trim();
        if (fchgdue.length() > 0)
        	FinanceChargeDue  = FormatDate(fchgdue, recNum);
        else
        	FinanceChargeDue  = null;

        LateFee           = new BigDecimal(data.substring(1828, 1837)).movePointLeft(2);
        
        CurrentDollars    = new BigDecimal(data.substring(1837, 1849)).movePointLeft(2);
        PastDue1          = new BigDecimal(data.substring(1849, 1861)).movePointLeft(2);
        PastDue2          = new BigDecimal(data.substring(1861, 1873)).movePointLeft(2);
        PastDue3          = new BigDecimal(data.substring(1873, 1885)).movePointLeft(2);
        PastDue4          = new BigDecimal(data.substring(1885, 1897)).movePointLeft(2);
        Rest              = new BigDecimal(data.substring(1897,1905)).movePointLeft(2); // 081214
    }
    
    public String dump()
    {
        String rc = super.dump();
        rc += 
        		DecimalFormat(9, FinanceCharge    .movePointRight(2)) +
        		DecimalFormat(9, BudgetInterest   .movePointRight(2)) +
        		DecimalFormat(9, OldCurrentDollars.movePointRight(2)) +
        		DecimalFormat(9, OldPastDue1      .movePointRight(2)) +
        		DecimalFormat(9, OldPastDue2      .movePointRight(2)) +
        		DecimalFormat(9, OldPastDue3      .movePointRight(2)) +
        		DecimalFormat(9, OldPastDue4      .movePointRight(2)) +
        		(PPGallonsRem != null ?
   			    DecimalFormat(9, PPGallonsRem     .movePointRight(1)) : "         ") +
        		String.format("%-60s",   DADShort) +
        		String.format("%-128s",  DunningMsg) +
        		String.format("%-1536s", SummaryMsg) +

        		(RateAnnual != null ?
        		DecimalFormat(4, RateAnnual     .movePointRight(2)) : "    ") +
        		(RateMonthly != null ?
        		DecimalFormat(4, RateMonthly    .movePointRight(2)) : "    ") +
        		(AvgDailyBalance != null ?
        		DecimalFormat(9, AvgDailyBalance.movePointRight(2)) : "         ") +
        		
        		(FinanceChargeDate != null ?
        		FormatDate(FinanceChargeDate) : "000000") +
        		(FinanceChargeDue != null ?
        		FormatDate(FinanceChargeDue)  : "      ") +

        		DecimalFormat(9, LateFee       .movePointRight(2)) +

        		DecimalFormat(12, CurrentDollars.movePointRight(2)) +
        		DecimalFormat(12, PastDue1      .movePointRight(2)) +
        		DecimalFormat(12, PastDue2      .movePointRight(2)) +
        		DecimalFormat(12, PastDue3      .movePointRight(2)) +
        		DecimalFormat(12, PastDue4      .movePointRight(2)) +
        		DecimalFormat(9,  Rest          .movePointRight(2)); // 081214
        return rc;
    }

	/**
	 * @return the financeCharge
	 */
	public BigDecimal getFinanceCharge() {
		return FinanceCharge;
	}

    /**
	 * @param financeCharge the financeCharge to set
	 */
	public void setFinanceCharge(BigDecimal financeCharge) {
		FinanceCharge = financeCharge;
	}
}
