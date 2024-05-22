// Copyright 2002
// Association for Universities for Research in Astronomy, Inc.,
// Observatory Control System, Gemini Telescopes Project.
//
// $Id: PlotableCatalog.java,v 1.2 2009/02/20 23:10:11 abrighto Exp $


package jsky.catalog;

/**
 * Defines the interface for catalogs whose tabular query results
 * can be plotted on an image.
 *
 * @version $Revision: 1.2 $
 * @author Allan Brighton
 */
public abstract interface PlotableCatalog extends Catalog {

    /** Return the number of plot symbol definitions associated with this catalog. */
    public int getNumSymbols();

    /** Return the ith plot symbol description */
    public TablePlotSymbol getSymbolDesc(int i);

    /** Return an array of symbol descriptions, or null if none are defined. */
    public TablePlotSymbol[] getSymbols();

    /** Set the plot symbol descriptions to use to plot tables returned from this catalog */
    public void setSymbols(TablePlotSymbol[] symbols);

    /** Set to true if the user edited the plot symbol definitions (default: false) */
    public void setSymbolsEdited(boolean edited);

    /** Return true if the user edited the plot symbol definitions otherwise false */
    public boolean isSymbolsEdited();

    /** Save the catalogs's symbol definitions to disk with the user's changes */
    public void saveSymbolConfig();

}


