/*
 * Created on August 2, 2012 by laddp
 *
 */
package com.bottinifuel.FlatFileParser.Records;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public class PostingCodeRecord extends Record
{
    public final int     Code;
    public final String  ShortDescription;
    public final String  Description;

    public PostingCodeRecord(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.POST_CODE, recNum, data, -10+1);

        Code             = Integer.parseInt(data.substring(3, 6));
        ShortDescription = data.substring(6,11).trim();
        Description      = data.substring(11).trim();
    }

    /* (non-Javadoc)
     * @see com.bottinifuel.FlatFileParser.Record#dump(java.io.OutputStream)
     */
    @Override
    public String dump()
    {
        String rc = super.dump();
        rc += String.format("%03d", Code);
        rc += String.format("%-5s",  ShortDescription);
        rc += String.format("%s",   Description);
        return rc;
    }
    
    @Override
    public String toString()
    {
    	String rc = super.toString();
    	return rc + " - " + Code + " (" + ShortDescription + ") " + Description;
    }
}
