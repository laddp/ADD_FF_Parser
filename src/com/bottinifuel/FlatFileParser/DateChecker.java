/**
 * 
 */
package com.bottinifuel.FlatFileParser;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;

import com.bottinifuel.FlatFileParser.Records.LineItemRecord;
import com.bottinifuel.FlatFileParser.Records.Record;

/**
 * @author laddp
 *
 */
public class DateChecker {

	// Check dates in file - warn if newer than document date
	public static boolean CheckDates(FlatFile ff)
	{
		Date documentDate = ff.DocumentHeader.DocumentDate;
		Calendar c = Calendar.getInstance();
		c.setTime(documentDate);
		c.add(Calendar.DAY_OF_YEAR, -31);
		Date documentDateMinusMonth = c.getTime();
		
		SimpleDateFormat df = new SimpleDateFormat("MM/dd/yyyy");
		
		for (Record r : ff.AllRecords)
		{
			if (r instanceof LineItemRecord)
			{
				LineItemRecord li = (LineItemRecord)r;
				if (li.TransDate.after(documentDate))
					System.out.println("WARNING! Line item " + li.RecordNum + " date " + df.format(li.TransDate) +
							" after document date " + df.format(documentDate));
				
				if (!li.Keyoff && li.TransDate.before(documentDateMinusMonth))
					System.out.println("WARNING! Line item " + li.RecordNum + " date " + df.format(li.TransDate) +
							" more than 31 days before document date " + df.format(documentDate));
			}
		}
		
		return false;
	}
}
