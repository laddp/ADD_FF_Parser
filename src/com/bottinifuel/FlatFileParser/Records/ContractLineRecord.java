/*
 * Created on Jun 14, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser.Records;

import java.math.BigDecimal;
import java.util.Date;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public class ContractLineRecord extends Record
{
    public final Date       TransDate;
    public final int        TransCode;
    public final char       Letter;
    public final int        BasePrice;
    public final int        SubLevel;
    public final BigDecimal Dollars;
    public final BigDecimal Discount;
    public final String     RefNum;
    public final String     ShortAddress;
    public final String     CoveragePeriod;
    public final char       RenewalPeriod;
    public final String     ConsecutiveBillingMonthsRemaining;
    public final String     ConsecutiveBillingTotalMonths;
    public final BigDecimal Deviation;
    public final int        LocationNum;

    public ContractLineRecord(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.CONTRACT_LINE, recNum, data, 145+1);
        
        TransDate       = FormatDate(data.substring(3, 9), recNum);
        TransCode       = Integer.parseInt(data.substring(9,12));
        Letter          = data.charAt(12);
        BasePrice       = Integer.parseInt(data.substring(13, 15));
        SubLevel        = Integer.parseInt(data.substring(15, 17));
        Dollars         = new BigDecimal(data.substring(17,26)).movePointLeft(2);
        Discount        = new BigDecimal(data.substring(26,35)).movePointLeft(2);
        RefNum         = data.substring( 35,  44);
        ShortAddress   = data.substring( 44, 104);
        CoveragePeriod = data.substring(104, 129);
        RenewalPeriod  = data.charAt(129);
        ConsecutiveBillingMonthsRemaining = data.substring(130, 132);
        ConsecutiveBillingTotalMonths     = data.substring(132, 134);
        Deviation       = new BigDecimal(data.substring(134,143)).movePointLeft(2);
        LocationNum = Integer.parseInt(data.substring(143, 146));
    }
    
    public String dump()
    {
        String rc = super.dump();
        rc += FormatDate(TransDate)
           +  String.format("%03d", TransCode)
           +  Letter
           +  String.format("%02d", BasePrice)
           +  String.format("%02d", SubLevel)
           +  DecimalFormat(9, Dollars .movePointRight(2))
           +  DecimalFormat(9, Discount.movePointRight(2))
           +  RefNum
           +  ShortAddress
           +  CoveragePeriod
           +  RenewalPeriod
           +  ConsecutiveBillingMonthsRemaining
           +  ConsecutiveBillingTotalMonths
           +  DecimalFormat(9, Deviation.movePointRight(2))
           +  String.format("%03d", LocationNum);
        return rc;
    }
}
