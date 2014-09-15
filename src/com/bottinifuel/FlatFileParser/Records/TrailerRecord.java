/*
 * Created on Jun 9, 2010 by pladd
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
public class TrailerRecord extends Record implements Cloneable
{
    public Date       FileDateTime;
    public int        TotalItems;
    public BigDecimal TotalAmountBilled;
    
    public TrailerRecord(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.TRAILER, recNum, data, 34);
        
        FileDateTime      = FormatDateTime(data.substring  ( 3,17), recNum);
        TotalItems        = Integer.parseInt(data.substring(17,24).trim());
        TotalAmountBilled = new BigDecimal(data.substring  (24,34)).movePointLeft(2);
    }
    
    public TrailerRecord(int recNum, Date d, Integer totalItems, BigDecimal totalBilled)
    {
    	super(RecordTypeEnum.TRAILER, recNum);

    	FileDateTime      = d;
    	TotalItems        = totalItems;
    	TotalAmountBilled = totalBilled;
    }

    /* (non-Javadoc)
     * @see com.bottinifuel.FlatFileParser.Record#dump(java.io.OutputStream)
     */
    @Override
    public String dump()
    {
        String rc = super.dump();
        rc += FormatDateTime(FileDateTime)
           +  String.format("%7d", TotalItems)
           +  DecimalFormat(10, TotalAmountBilled.movePointRight(2));
        return rc;
    }

    public Object clone()
    {
        TrailerRecord t = null;
        t = (TrailerRecord) super.clone();
        t.FileDateTime = (Date)FileDateTime.clone();
        t.TotalAmountBilled = new BigDecimal(TotalAmountBilled.toString());
        return t;
    }
    
    /**
     * @return the fileDateTime
     */
    public Date getFileDateTime()
    {
        return FileDateTime;
    }

    /**
     * @param fileDateTime the fileDateTime to set
     */
    public void setFileDateTime(Date fileDateTime)
    {
        FileDateTime = fileDateTime;
    }

    /**
     * @return the totalItems
     */
    public int getTotalItems()
    {
        return TotalItems;
    }

    /**
     * @param totalItems the totalItems to set
     */
    public void setTotalItems(int totalItems)
    {
        TotalItems = totalItems;
    }

    /**
     * @return the totalAmountBilled
     */
    public BigDecimal getTotalAmountBilled()
    {
        return TotalAmountBilled;
    }

    /**
     * @param totalAmountBilled the totalAmountBilled to set
     */
    public void setTotalAmountBilled(BigDecimal totalAmountBilled)
    {
        TotalAmountBilled = totalAmountBilled;
    }
}