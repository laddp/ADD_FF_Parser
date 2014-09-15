/*
 * Created on Jun 10, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser.Records;

import java.util.Date;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public class DocumentHeaderRec extends Record
{
    public final String  Type;
    public final Date    DocumentDate;
    public final int     ShortDB;
    public final Date    BudgetDue;
    public final boolean BudgetDateSet;
    public final int     BudgetMonth;
    public final boolean BudgetMonthSet;
    public final String  Message;
    public final char    Logo;
    public final int     ExpDB;
    
    public DocumentHeaderRec(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.DOC_HEADER, recNum, data, 752+1);
        
        Type         = data.substring(3, 28);
        DocumentDate = FormatDate(data.substring(28,34), recNum);
        ShortDB      = Integer.parseInt(data.substring(34,36));
        if (data.substring(36, 42).trim().length() != 0)
        {
            BudgetDue     = FormatDate(data.substring(36, 42), recNum);
            BudgetDateSet = true;
        }
        else
        {
            BudgetDateSet = false;
            BudgetDue     = new Date();
        }
        if (data.substring(42,44).trim().length() != 0)
        {
        	BudgetMonth  = Integer.parseInt(data.substring(42,44));
        	BudgetMonthSet = true;
        }
        else
        {
        	BudgetMonth  = 0;
        	BudgetMonthSet = false;
        }
        Message      = data.substring(44,748);
        Logo         = data.charAt(748);
        ExpDB        = Integer.parseInt(data.substring(749, 753));
    }

    /* (non-Javadoc)
     * @see com.bottinifuel.FlatFileParser.Record#dump(java.io.OutputStream)
     */
    @Override
    public String dump()
    {
        String rc = super.dump();
        rc += String.format("%25s", Type);
        rc += FormatDate(DocumentDate);
        rc += String.format("%02d", ShortDB);
        if (BudgetDateSet)
            rc += FormatDate(BudgetDue);
        else
            rc += "      ";
        if (BudgetMonthSet)
        	rc += String.format("%02d", BudgetMonth);
        else
        	rc += "  ";
        rc += Message;
        rc += Logo;
        rc += String.format("%04d", ExpDB);
        return rc;
    }
}
