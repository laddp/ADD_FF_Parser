/*
 * Created on Jun 9, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser.Records;

import java.math.BigDecimal;
import java.text.DateFormat;
import java.text.DecimalFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public abstract class Record implements Cloneable
{
    public enum RecordTypeEnum {
        INFO_HEADER      ("01"),
        DOC_HEADER       ("02"),
        DIVISION         ("03"),
        POST_CODE        ("04"),
        PROMPT_PAY_MSG   ("05"),
        CONTRACT_INFO    ("05"),
        STATEMENT        ("11"),
        LINE_ITEM        ("12"),
        CREDIT_INFO      ("13"),
        DISCNT_INFO      ("14"),
        FINCHG_INFO      ("15"),
        CONTRACT_FLAGS   ("16"),
        METER_INFO       ("17"),
        CONTRACT_LINE    ("18"),
        CONTRACT_TAX_LINE("19"),
        SLPP_INFO        ("20"),
        LINE_ITEM_ADDR   ("26"),
        STATEMENT_ADDR   ("28"),
        TRAILER          ("99");

        public final String RecordCode;
        
        RecordTypeEnum(String code)
        {
            RecordCode = new String(code);
        }
        
        public static RecordTypeEnum recordType(String rt) throws FileFormatException
        {
            for (RecordTypeEnum t : RecordTypeEnum.values())
                if (rt.compareTo(t.RecordCode) == 0)
                    return t;
            throw new FileFormatException("Invalid record type " + rt);
        }
    }
    
    public final RecordTypeEnum RecordType;
    public final int            RecordNum;
    
    Record(RecordTypeEnum rt, int recnum)
    {
        RecordType = rt;
        RecordNum = recnum;
    }
    
    Record(RecordTypeEnum rt, int recNum, String data, int expected_len) throws FileFormatException
    {
    	if (expected_len > 0)
    	{
    		if (data.length() != expected_len)
    			throw new FileFormatException(recNum,
    					"Invalid record length: expected " + expected_len + 
    					", got " + data.length());
    	}
    	// negative value for expected_len means at least that long, but possibly longer
    	else if (expected_len < 0)
    	{
    		if (data.length() < -expected_len)
    			throw new FileFormatException(recNum,
    					"Invalid record length: expected at least " + -expected_len + 
    					", got " + data.length());
    	}
    	// Zero value for expected_len means skip check

        RecordType = rt;
        RecordNum = recNum;
    }
    
    public String toString()
    {
        return "Record #" + RecordNum + " - " + RecordType;
    }
    
    public String dump()
    {
        return RecordType.RecordCode;
    }
    
    protected static DateFormat sixCharDateFormat = new SimpleDateFormat("MMddyy");
    protected static Date FormatDate(String date, int recNum) throws FileFormatException
    {
        try {
            return sixCharDateFormat.parse(date);
        }
        catch (ParseException e)
        {
            throw new FileFormatException("Line #" + recNum + " - " + e.toString());
        }
    }
    protected static String FormatDate(Date d)
    {
        return sixCharDateFormat.format(d);
    }
    
    protected static DateFormat dateTimeFormat = new SimpleDateFormat("MMddyyhh:mm aa");
    protected static Date FormatDateTime(String date, int recNum) throws FileFormatException
    {
        try {
            return dateTimeFormat.parse(date);
        }
        catch (ParseException e)
        {
            throw new FileFormatException("Line #" + recNum + " - " + e.toString());
        }
    }
    protected static String FormatDateTime(Date d)
    {
        return dateTimeFormat.format(d);
    }

    
    private static Map<Integer, DecimalFormat> NegFormats = new HashMap<Integer, DecimalFormat>();
    private static Map<Integer, DecimalFormat> PosFormats = new HashMap<Integer, DecimalFormat>();

    protected static String DecimalFormat(int len, BigDecimal d)
    {
        DecimalFormat formatter;
        if (d.signum() == -1)
        {
            if (NegFormats.containsKey(len))
                formatter = NegFormats.get(len);
            else
            {
//              Generate & store format string of form "000000000;-"
                String format = new String();
                format = "%1$0" + (len - 1) + "d;-";
                String negFormat = String.format(format, 0);
                formatter = new DecimalFormat(negFormat);
                NegFormats.put(len, formatter);
            }
        }
        else
        {      
            if (PosFormats.containsKey(len))
                formatter = PosFormats.get(len);
            else
            {
//              Generate & store format string of form "0000000000"
                String format = new String();
                format = "%1$0" + len + "d";
                String posFormat = String.format(format, 0);
                formatter = new DecimalFormat(posFormat);
                PosFormats.put(len, formatter);
            }
        }
        return formatter.format(d);
    }
    
    protected static boolean ReadFlag(char testChar,
                                      String trueChars, String falseChars,
                                      int recNum, String fieldName) throws FileFormatException
    {
        if (trueChars.indexOf(testChar) != -1)
            return true;
        else if (falseChars.indexOf(testChar) != -1)
            return false;
        else
            throw new FileFormatException(recNum,
                                          "Invalid character '" + testChar +
                                          "' for " + fieldName);
    }
    
    public Object clone()
    {
        try
        {
            return super.clone();
        }
        catch( CloneNotSupportedException e )
        {
            return null;
        }
    }
}
