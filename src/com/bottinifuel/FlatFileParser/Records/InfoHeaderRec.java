/*
 * Created on Jun 9, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser.Records;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public class InfoHeaderRec extends Record
{
    public final String ClientNumber;
    public final String ProjectNumber;
    
    public InfoHeaderRec(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.INFO_HEADER, recNum, data, 14);
        ClientNumber = data.substring(3, 9);
        ProjectNumber = data.substring(9, 14);
//        if (data.charAt(14) != ' ')
//            throw new FileFormatException(recNum, "Non-blank character at InfoHeaderRec(14)");
    }

    /* (non-Javadoc)
     * @see com.bottinifuel.FlatFileParser.Record#dump(java.io.OutputStream)
     */
    @Override
    public String dump()
    {
        String rc = super.dump();
        rc += ClientNumber;
        rc += ProjectNumber;
//        rc += ' ';
        return rc;
    }
}