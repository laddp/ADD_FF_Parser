/*
 * Created on Sep 20, 2010 by pladd
 *
 */
package com.bottinifuel;

import jargs.gnu.CmdLineParser;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * This example shows how to dynamically create basic output for a --help option.
 */
public class AutoHelpParser extends CmdLineParser {
    private List<String> optionHelpStrings = new ArrayList<String>();

    public Option addHelp(Option option, String helpString) {
        optionHelpStrings.add(" -" + option.shortForm() + "/--" + option.longForm() + ": " + helpString);
        return option;
    }

    public void printUsage() {
        System.err.println("usage: prog [options]");
        for (Iterator<String> i = optionHelpStrings.iterator(); i.hasNext(); ) {
            System.err.println(i.next());
        }
    }
}