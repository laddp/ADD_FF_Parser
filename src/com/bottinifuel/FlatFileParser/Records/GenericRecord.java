/*
 * Created on Jun 14, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser.Records;

/**
 * @author pladd
 *
 */
public class GenericRecord extends Record
{
    private String Data;
    
    public GenericRecord(RecordTypeEnum type, String data, int recNum)
    {
        super(type, recNum);
        Data = data.substring(3);
    }
    
    public String dump()
    {
        String rc = super.dump();
        rc += Data;
        return rc;
    }
}
