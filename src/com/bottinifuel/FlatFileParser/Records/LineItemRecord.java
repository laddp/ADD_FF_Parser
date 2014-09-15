/*
 * Created on Jun 14, 2010 by pladd
 *
 *
 *************************************************************************/
 /* Change Log:
 *
 *   Date         Description                                          Pgmr
 *  ------------  --------------------------------------------------   -----
 *  Oct 8, 2013   Code was programmed to handle only a blank           carlonc
 *                TransMinCharge field in the Line item record.
 *                Received a file with a 'Y' in the field. Code has 
 *                been changed to accept this.
 *                Look for comment 100813      
 *  Aug 12, 2014  Record length changed for ADDs 14 upgrade.            carlonc
 *                Look for comment 081214
 **********************************************************************************/
package com.bottinifuel.FlatFileParser.Records;

import java.math.BigDecimal;
import java.text.DecimalFormat;
import java.util.Date;

import com.bottinifuel.FlatFileParser.FileFormatException;

/**
 * @author pladd
 *
 */
public class LineItemRecord extends Record
{
    public final Date       TransDate;
    public final int        TransCode;
    public final BigDecimal OldTransDollars;
    public final BigDecimal OldTransGallons;
    public final BigDecimal TransUnitPrice;
    public final String     RefNum;
    public final String     ShortAddress;
    public final String     ProductMessage;
    public final int        CylDelivered;
    public final int        CylReturned;
    public final String     PONumber;
    public final char       TransMinCharge;  // 100813
    public final BigDecimal OldOrigDollars;
    public final boolean    DueDateBlank;
    public final Date       DueDate;
    public final String     NetDays;
    public final boolean    Keyoff;
    public final char       EFTStatus;
    
    public final BigDecimal Gallons;
    public final BigDecimal Dollars;
    public final BigDecimal OrigDollars;
    
    private      int        LocNum;
    
    public final BigDecimal UnitPrice2;
    
    public final int        PrebuyType;
    public final int        DeliveryType;
    
    public final char       BudgetableChar;
    public final int        ITSProd;
    private      char       LocType;
    private      String     Rest;                  // 081214

    public LineItemRecord(String data, int recNum) throws FileFormatException
    {
        super(RecordTypeEnum.LINE_ITEM, recNum, data, 338+52);  // 081214
        
        TransDate      = FormatDate(data.substring(3, 9), recNum);
        TransCode      = Integer.parseInt(data.substring(9,12));
        OldTransDollars   = new BigDecimal(data.substring(12,21)).movePointLeft(2);
        OldTransGallons   = new BigDecimal(data.substring(21,30)).movePointLeft(1);
        TransUnitPrice = new BigDecimal(data.substring(30,38)).movePointLeft(4);
        RefNum         = data.substring( 38,  47);
        ShortAddress   = data.substring( 47, 107);
        ProductMessage = data.substring(107, 235);
        CylDelivered   = Integer.parseInt(data.substring(235, 238));
        CylReturned    = Integer.parseInt(data.substring(238, 241));
        PONumber       = data.substring(241, 266);
        // Added TransMinCharge for 100813            
        TransMinCharge = data.charAt(266); 
        // commented block for 100813 
/*        ReadFlag(data.charAt(266), "Y", " ", recNum, "TransMinCharge");
        if (ReadFlag(data.charAt(266), "Y", " ", recNum, "TransMinCharge") != true)
              throw new FileFormatException(recNum,
                                            "Never seen a non-blank '" + data.charAt(266) +
                                            "' for TransMinCharge");*/
        OldOrigDollars = new BigDecimal(data.substring(267, 276)).movePointLeft(2);
        if (data.substring(276, 282).trim().length() != 0)
        {
            DueDateBlank = false;
            DueDate = FormatDate(data.substring(276, 282), recNum);
        }
        else
        {
            DueDateBlank = true;
            DueDate = new Date();
        }
        NetDays = data.substring(282, 286);
        
        Keyoff    = ReadFlag(data.charAt(286), " ", "N", recNum, "Keyoff");
        EFTStatus = data.charAt(287);
        
        Gallons     = new BigDecimal(data.substring(288, 298)).movePointLeft(2);
        Dollars     = new BigDecimal(data.substring(298, 308)).movePointLeft(2);
        OrigDollars = new BigDecimal(data.substring(308, 318)).movePointLeft(2);
        
        LocNum = Integer.parseInt(data.substring(318, 321));
        
        UnitPrice2 = new BigDecimal(data.substring(321, 329)).movePointLeft(4);
        
        PrebuyType = Integer.parseInt(data.substring(329, 330));
        
        if (data.charAt(330) != ' ')
            throw new FileFormatException(recNum, "Non-blank California indicator");
        
        DeliveryType = Integer.parseInt(data.substring(331, 333));
//        Budgetable = ReadFlag(data.charAt(333), "Y", "N", recNum, "Budgetable");
        BudgetableChar = data.charAt(333);
        ITSProd = Integer.parseInt(data.substring(334, 337));
        LocType = data.charAt(337);
        Rest = data.substring(338,390); // 081214
    }
    
    /**
     * @return the locNum
     */
    public int getLocNum()
    {
        return LocNum;
    }

    /**
     * @return the locType
     */
    public char getLocType()
    {
        return LocType;
    }

    /**
     * @param locNum the locNum to set
     */
    public void setLocNum(int locNum)
    {
        LocNum = locNum;
    }

    /**
     * @param locType the locType to set
     */
    public void setLocType(char locType)
    {
        LocType = locType;
    }

    public String dump()
    {
        String rc = super.dump();
        rc += FormatDate(TransDate)
           +  String.format("%03d", TransCode)
           +  DecimalFormat(9, OldTransDollars  .movePointRight(2))
           +  DecimalFormat(9, OldTransGallons  .movePointRight(1))
           +  DecimalFormat(8, TransUnitPrice.movePointRight(4))
           +  RefNum
           +  ShortAddress
           +  ProductMessage
           +  String.format("%03d", CylDelivered)
           +  String.format("%03d", CylReturned)
           +  PONumber
           +  TransMinCharge // changed from hard coded ' ' to TransMinCharge var for 100813
           +  DecimalFormat(9, OldOrigDollars.movePointRight(2))
           + (DueDateBlank ? "      " : FormatDate(DueDate))
           +  NetDays
           + (Keyoff ? ' ' : 'N')
           +  EFTStatus
           +  DecimalFormat(10, Gallons    .movePointRight(2))
           +  DecimalFormat(10, Dollars    .movePointRight(2))
           +  DecimalFormat(10, OrigDollars.movePointRight(2))
           +  String.format("%03d", getLocNum())
           +  DecimalFormat(8, UnitPrice2.movePointRight(4))
           +  String.format("%1d",  PrebuyType)
           +  ' ' // California indicator
           +  String.format("%02d", DeliveryType)
//         + (Budgetable ? "Y" : "N")
           + BudgetableChar
           + String.format("%03d", ITSProd)
           + getLocType()
           + Rest;  // 081214
        return rc;
    }
    
    public String toString()
    {
        return "Record #" + RecordNum + " - " + TransDate + " - " + TransCode + " - " + DecimalFormat.getCurrencyInstance().format(OldTransDollars);
    }
}
