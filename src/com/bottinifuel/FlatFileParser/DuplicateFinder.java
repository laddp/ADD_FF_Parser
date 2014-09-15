/*
 * Created on Sep 17, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser;

import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import com.bottinifuel.FlatFileParser.Records.DocumentRecord;

/**
 * @author pladd
 *
 */
public class DuplicateFinder
{
    public static Map<Integer, List<FlatFile>> FindDuplicates(List<FlatFile> flatFiles)
    {
        Map<Integer, FlatFile> accounts = new LinkedHashMap<Integer, FlatFile>();
        Map<Integer, List<FlatFile>> duplicates = new LinkedHashMap<Integer, List<FlatFile>>();
        
        // load all account numbers
        for (FlatFile ff : flatFiles)
        {
            for (DocumentRecord s : ff.Documents)
            {
                Integer acct_num = new Integer(s.AccountNum);
                if (accounts.containsKey(acct_num))
                {
                    if (duplicates.containsKey(acct_num))
                    {
                        List<FlatFile> list = duplicates.get(acct_num);
                        list.add(ff);
                    }
                    else
                    {
                        FlatFile firstFF = accounts.get(acct_num);
                        List<FlatFile> list = new LinkedList<FlatFile>();
                        list.add(firstFF);
                        list.add(ff);
                        duplicates.put(acct_num, list);
                    }
                }
                else
                    accounts.put(acct_num, ff);
            }
        }

        if (duplicates.size() == 0)
            return null;
        else
            return duplicates;
    }
}
