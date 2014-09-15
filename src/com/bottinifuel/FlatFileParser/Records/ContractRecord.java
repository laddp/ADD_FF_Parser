/*
 * Created on Nov 19, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser.Records;

import com.bottinifuel.FlatFileParser.FileFormatException;
import com.bottinifuel.FlatFileParser.FlatFile.FileTypeEnum;

/**
 * @author pladd
 *
 */
/*************************************************************************************
*  Change Log:
* 
*   Date         Description                                        Pgmr
*  ------------  ------------------------------------------------   -----
*  Aug 22, 2014  Record length changed for ADDs 14 upgrade.        carlonc
*                Look for comment 082214
***************************************************************************************/ 
public class ContractRecord extends DocumentRecord
{

    /**
     * @param rt
     * @param recnum
     */
	
	public final int CreditTimeLimit;
	
    public ContractRecord(String data, int recNum) throws FileFormatException
    {
        super(FileTypeEnum.CONTRACT_FILE, recNum, data, 323+13);  // 082214
        
        CreditTimeLimit = Integer.parseInt(data.substring(322, 323));
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
            String.format("%22s", " ") +
            DecimalFormat(9, TotalDue.movePointRight(2)) +
            String.format("%18s", " ") +
            OCRLine +
            String.format("%2s", " ") +
            OpenItemFlag +
            String.format("%13s", " ") +
            String.format("%02d", OldCompanyCode) +
            String.format("%12s", " ") +
            (isSpecialHandling() ? "Y" : "N") +
            String.format("%4s", Other) + // 082214 added Other  
            DirectDebit +
            String.format("%-7s", AccountNum) +
            String.format("%04d", Division) +
            String.format("%04d", CompanyCode) +
            DocumentViewerID +
            String.format("%04d", CategoryCode) +
            String.format("%02d", FinanceChargeGroup) +
            String.format("%1d", CreditTimeLimit) +
            Rest;  // 082214
        return rc;
    }
}
