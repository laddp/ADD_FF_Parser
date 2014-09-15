/*
 * Created on Jun 14, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser;

/**
 * @author pladd
 *
 */
public class FileFormatException extends Exception
{
    private static final long serialVersionUID = -7523823720918010354L;

    public FileFormatException(int line, String msg)
    {
        super("Line #" + line + ": " + msg);
    }

    public FileFormatException(String msg)
    {
        super(msg);
    }
}
