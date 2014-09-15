/*
 * Created on Jun 11, 2010 by pladd
 *
 */
package com.bottinifuel.FlatFileParser;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.io.PrintStream;
import java.math.BigDecimal;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.bottinifuel.FlatFileParser.Records.ContractLineRecord;
import com.bottinifuel.FlatFileParser.Records.ContractRecord;
import com.bottinifuel.FlatFileParser.Records.CreditInfoRecord;
import com.bottinifuel.FlatFileParser.Records.DocumentHeaderRec;
import com.bottinifuel.FlatFileParser.Records.DocumentRecord;
import com.bottinifuel.FlatFileParser.Records.FinanaceChargeInfoRecord;
import com.bottinifuel.FlatFileParser.Records.GenericRecord;
import com.bottinifuel.FlatFileParser.Records.LineItemRecord;
import com.bottinifuel.FlatFileParser.Records.PostingCodeRecord;
import com.bottinifuel.FlatFileParser.Records.Record;
import com.bottinifuel.FlatFileParser.Records.Record.RecordTypeEnum;
import com.bottinifuel.FlatFileParser.Records.StatementRecord;
import com.bottinifuel.FlatFileParser.Records.TrailerRecord;


/**
 * @author pladd
 *
 */
public class FlatFile implements Cloneable
{
    public enum FileTypeEnum {
        STATEMENT_FILE,
        CONTRACT_FILE
    }
    
    public final FileTypeEnum FileType;
    public final String FileName;
    
    public List<Record>            AllRecords   = new ArrayList<Record>();
    public List<DocumentRecord>    Documents    = new ArrayList<DocumentRecord>();
    public Map<Integer, PostingCodeRecord> PostingCodes = new HashMap<Integer,PostingCodeRecord>();
    
    public DocumentHeaderRec DocumentHeader = null;
    public TrailerRecord     Trailer        = null;
    
    private int        SpecialHandlingCount;
    private BigDecimal TotalFinanceCharges;
    private BigDecimal TotalBudgetInterest;
    private BigDecimal TotalDue;
    
    protected static int PreviousFileFormat;

    public FlatFile(FileTypeEnum ft, String name, InputStreamReader ir, PrintStream messages, int older, boolean skipTrailer) throws FileFormatException, IOException
    {
        FileType = ft;
        FileName = name;
        PreviousFileFormat = older;

        boolean verifyDumpFormat = true;
        int recNum = 0;

        try {
            LineNumberReader reader = new LineNumberReader(ir);

            DocumentRecord currentDocument = null;
            BigDecimal totalDue = new BigDecimal(0);
            BigDecimal totalFinanceCharge  = new BigDecimal(0);
            BigDecimal totalBudgetInterest = new BigDecimal(0);
            int specialHandlingCount = 0;
            int lineItemCount = 0;

            while (reader.ready())
            {
                    String line = reader.readLine();
                    recNum = reader.getLineNumber();
                    RecordTypeEnum type = RecordTypeEnum.recordType(line.substring(0, 2));
                    String data = " " + line;
                    Record lastRec;
                    
                    switch (type)
                    {
//                    case INFO_HEADER:
//                        InfoHeaderRec ihr = new InfoHeaderRec(data, recNum);
//                        lastRec = ihr;
//                        break;
                    case DOC_HEADER:
                        DocumentHeaderRec dhr = new DocumentHeaderRec(data, recNum);
                        if (DocumentHeader == null)
                        	DocumentHeader = dhr;
                        else
                        	throw new FileFormatException(recNum, "Multiple document header records in file");
                        lastRec = dhr;
                        
                        switch (ft)
                        {
                        case CONTRACT_FILE:
                        	if (DocumentHeader.Type.trim().compareTo("SERVICE CONTRACT") != 0)
                        		throw new FileFormatException(recNum, "File is not type 'SERVICE CONTRACT' - got '" + DocumentHeader.Type.trim() + "'");
                        	break;
                        case STATEMENT_FILE:
                        	if (DocumentHeader.Type.trim().compareTo("STATEMENT") != 0)
                        		throw new FileFormatException(recNum, "File is not type 'STATEMENT' - got '" + DocumentHeader.Type.trim() + "'");
                        	break;
                        default:
                        	break;
                        }
                        break;
                    case POST_CODE:
                    	PostingCodeRecord pc = new PostingCodeRecord(data, recNum);
                    	PostingCodes.put(new Integer(pc.Code), pc);
                    	lastRec = pc;
                    	break;
                    case STATEMENT:
                        DocumentRecord stmt;
                        if (FileType == FileTypeEnum.STATEMENT_FILE)
                            stmt = new StatementRecord(data, recNum);
                        else
                            stmt = new ContractRecord(data, recNum);
                        Documents.add(stmt);
                        if (stmt.isSpecialHandling())
                            specialHandlingCount++;
                        totalDue = totalDue.add(stmt.getTotalDue());
                        currentDocument = stmt;
                        lineItemCount = 0;
                        lastRec = stmt;
                        break;
                    case CONTRACT_LINE:
                        if (FileType != FileTypeEnum.CONTRACT_FILE)
                            throw new FileFormatException(recNum, "Line item in non-contract");
                        ContractLineRecord cl = new ContractLineRecord(data, recNum);
                        lastRec = cl;
                        break;
                    case LINE_ITEM:
                        if (FileType != FileTypeEnum.STATEMENT_FILE)
                            throw new FileFormatException(recNum, "Line item in non-statement");
                        // FIXME!
                    	if (data.length() == 337)
                    	{
                    		data += " ";
                    		line += " ";
                    	}
                        LineItemRecord li = new LineItemRecord(data, recNum);
                        if (++lineItemCount == 50)
                            messages.println("Warning: 50+ line items on account " +
                                               currentDocument.AccountNum +
                                               " (" + currentDocument.Name + ")");
                        lastRec = li;
                        break;
                    case CREDIT_INFO:
                        CreditInfoRecord creditInfo = new CreditInfoRecord(data, recNum);
                        totalFinanceCharge  = totalFinanceCharge .add(creditInfo.getFinanceCharge());
                        totalBudgetInterest = totalBudgetInterest.add(creditInfo.BudgetInterest);
                        //TODO: are there CREDIT_INFOs in contracts?
                        if (currentDocument instanceof StatementRecord)
                        	((StatementRecord)currentDocument).CreditInfoRecord = creditInfo;
                        lastRec = creditInfo;
                        break;
                    case FINCHG_INFO:
                    	FinanaceChargeInfoRecord finchg = new FinanaceChargeInfoRecord(data, recNum);
                        if (currentDocument instanceof StatementRecord)
                        	((StatementRecord)currentDocument).FinanceChargeInfo = finchg;
                    	lastRec = finchg;
                    	break;
                    case TRAILER:
                        Trailer = new TrailerRecord(data, recNum);
                        lastRec = Trailer;
                        currentDocument = null;
                        break;
                    default:
                        Record generic = new GenericRecord(type, data, recNum);
                        lastRec = generic;
                        break;
                    }

                    AllRecords.add(lastRec);
                    if (currentDocument != null && !lastRec.equals(currentDocument))
                        currentDocument.Records.add(lastRec);

                    if (verifyDumpFormat)
                    {
                        String dump = lastRec.dump();
                        if (!dump.equals(line))
                        {
                            System.out.println("Expected: " + line);
                            System.out.println("Got:      " + dump);
                            throw new FileFormatException(recNum, "Dump string mismatch");
                        }
                    }
            }

            SpecialHandlingCount = specialHandlingCount;
            TotalFinanceCharges = totalFinanceCharge;
            TotalBudgetInterest = totalBudgetInterest;
            TotalDue = totalDue;
            
            if (Trailer == null)
            {
            	if (!skipTrailer)
            		throw new FileFormatException("Missing trailer record");
            	
            	// Lots of other code relies on having a trailer, fabricate a fake one
            	Trailer = new TrailerRecord(reader.getLineNumber() + 1, new Date(), Documents.size(), TotalDue);
            }
            else
            {
            	if (Trailer.TotalItems != Documents.size())
            		throw new FileFormatException(Trailer.RecordNum,
            				"Mismatch in total item count:" +
            						" have " + Documents.size() +
            						" expecting " + Trailer.TotalItems);
            	if (FileType == FileTypeEnum.CONTRACT_FILE && 
            			!Trailer.TotalAmountBilled.equals(TotalDue))
            		throw new FileFormatException(Trailer.RecordNum,
            				"Mismatch in total amount billed:" +
            						" have " + DecimalFormat.getCurrencyInstance().format(totalDue) +
            						" expecting " + DecimalFormat.getCurrencyInstance().format(Trailer.getTotalAmountBilled()));
            }
        }
        catch (IOException e) {
            System.out.println(e);
            throw e;
        }
        catch (FileFormatException e)
        {
            System.out.println(e + " line number " + recNum);
            throw e;
        }
    }
    
    public int getSpecialHandlingCount()
    {
        return SpecialHandlingCount;
    }
    
    public DocumentRecord FindDocument(int account)
    {
        for (DocumentRecord s : Documents)
            if (s.AccountNum == account)
                return s;
        return null;
    }

    public void RemoveDocuments(Collection<DocumentRecord> removeList) throws Exception
    {
        // verify items are all in this file
        for (DocumentRecord remove : removeList)
        {
            if (!Documents.contains(remove))
                throw new Exception("Error: can't remove document for account " + remove.AccountNum +
                                    " (" + remove.Name + ") from flat file " + FileName +
                                    " because it is not in the file");
        }

        AllRecords.removeAll(removeList);
        Documents.removeAll(removeList);

        BigDecimal totalRemoved = new BigDecimal(0);
        for (DocumentRecord remove : removeList)
        {
            AllRecords.removeAll(remove.Records);
            if (remove.isSpecialHandling())
                SpecialHandlingCount--;
            if (remove instanceof StatementRecord &&
                ((StatementRecord)remove).CreditInfoRecord != null)
            {
                if (((StatementRecord)remove).CreditInfoRecord.getFinanceCharge().compareTo(BigDecimal.ZERO) != 0)
                    TotalFinanceCharges = TotalFinanceCharges.subtract(((StatementRecord)remove).CreditInfoRecord.getFinanceCharge());
                if (((StatementRecord)remove).CreditInfoRecord.BudgetInterest.compareTo(BigDecimal.ZERO) != 0)
                    TotalBudgetInterest = TotalBudgetInterest.subtract(((StatementRecord)remove).CreditInfoRecord.BudgetInterest);
            }
            totalRemoved = totalRemoved.add(remove.getTotalDue());
        }

        Trailer.setFileDateTime(new Date());
        Trailer.setTotalItems(Trailer.getTotalItems() - removeList.size());
        Trailer.setTotalAmountBilled(Trailer.getTotalAmountBilled().subtract(totalRemoved));
    }
    
    /**
     * Only use this function if you are making other changes that have a net zero effect on
     * the starting and ending balances on the statement
     * (i.e. changing a line item to a finance charge item or replacing with another line item)
     */
    public void RemoveLineItemsNoBalanceAdjust(DocumentRecord stmt, Collection<Record> removeList)
    {
    	AllRecords.removeAll(removeList);
    	stmt.RemoveLineItemsNoBalanceAdjust(removeList);
    }

    public void RemoveNewLineItems(DocumentRecord stmt, Collection<Record> removeList)
    {
        AllRecords.removeAll(removeList);

        BigDecimal totalRemoved = stmt.RemoveNewLineItems(removeList);
        Trailer.setFileDateTime(new Date());
        Trailer.setTotalAmountBilled(Trailer.getTotalAmountBilled().subtract(totalRemoved));
    }

    public void RemoveOldLineItems(DocumentRecord stmt, Collection<Record> removeList)
    {
        AllRecords.removeAll(removeList);

        stmt.RemoveOldLineItems(removeList);
        Trailer.setFileDateTime(new Date());
    }

    public Object clone()
    {
        FlatFile ff = null;
        try
        {
            ff = (FlatFile) super.clone();
        }
        catch( CloneNotSupportedException e )
        {
            return null;
        }
        
        ff.AllRecords = new ArrayList<Record>(AllRecords.size());
        for (Record r : AllRecords)
        {
            ff.AllRecords.add(r);
        }

        ff.Documents = new ArrayList<DocumentRecord>(Documents.size());
        for (DocumentRecord r : Documents)
        {
            ff.Documents.add(r);
        }
        
        ff.AllRecords.remove(Trailer);
        ff.Trailer = (TrailerRecord)Trailer.clone();
        ff.AllRecords.add(ff.Trailer);

        return ff;
    }

    /**
     * @return the totalFinanceCharges
     */
    public BigDecimal getTotalFinanceCharges()
    {
        return TotalFinanceCharges;
    }

    /**
     * @return the totalBudgetInterest
     */
    public BigDecimal getTotalBudgetInterest()
    {
        return TotalBudgetInterest;
    }

	/**
	 * @return the totalDue
	 */
	public BigDecimal getTotalDue() {
		return TotalDue;
	}
}
