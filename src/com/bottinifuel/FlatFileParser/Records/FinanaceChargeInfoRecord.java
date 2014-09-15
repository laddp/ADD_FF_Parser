/*
 * Created on Sep 1, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser.Records;

import java.util.Date;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public class FinanaceChargeInfoRecord extends Record
{
    public final Date       FinanceChargeDate;
    public final String     RefNum;

    public FinanaceChargeInfoRecord(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.FINCHG_INFO, recNum, data, 17+1);

        String fchgdt = data.substring(3, 9).trim();
        if (fchgdt.length() > 0)
        	FinanceChargeDate = FormatDate(fchgdt, recNum);
        else
        	FinanceChargeDate = null;

        RefNum = data.substring(9);
    }
    
    public FinanaceChargeInfoRecord(Date chargeDate, String refNum) throws FileFormatException
    {
        super(RecordTypeEnum.FINCHG_INFO, 0, null, 0);
    	FinanceChargeDate = chargeDate;
    	RefNum = refNum;
    }
    
    
    public String dump()
    {
        String rc = super.dump();
        rc += 
        		(FinanceChargeDate != null ?
        		FormatDate(FinanceChargeDate) : "      ") +
        		String.format("%-9s", RefNum);
        return rc;
    }
}
