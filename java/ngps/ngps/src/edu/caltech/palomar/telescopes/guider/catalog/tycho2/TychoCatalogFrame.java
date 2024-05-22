package edu.caltech.palomar.telescopes.guider.catalog.tycho2;
import java.awt.Color;
import javax.swing.event.*;
import javax.swing.ListSelectionModel;
import java.awt.Point;
import java.awt.geom.Point2D;
import edu.caltech.palomar.util.sindex.client.HTMIndexSearchClient;
import jsky.science.Coordinates;
import jsky.science.CoordinatesOffset;
import java.beans.*;
import edu.caltech.palomar.telescopes.P200.P200Component;
import edu.caltech.palomar.telescopes.P200.TelescopeObject;
import javax.swing.table.TableRowSorter;
import javax.swing.RowFilter;
import edu.dartmouth.jskycalc.coord.Celest;
import edu.dartmouth.jskycalc.coord.RA;
import edu.dartmouth.jskycalc.coord.dec;
import edu.caltech.palomar.telescopes.P200.gui.tables.AstroObject;
import javax.swing.JFrame;
import java.awt.event.WindowListener;
import java.awt.event.WindowEvent;
//import edu.caltech.palomar.telescopes.guider.gui.GuideStarDisplayFrame;
import edu.caltech.palomar.telescopes.guider.sdsstools.DS9ImageDisplayClient;
import javax.swing.ImageIcon;
//=== File Prolog =============================================================
//	This code was developed by John Thorstensen of Dartmouth University
//
//      It has been refactored and modified by Jennifer Milburn for
//      use at the Palomar Observatory.
//
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//    TychoCatalogFrame
//    Jennifer Milburn June 23,2011
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
// TERMS OF USE --
// Anyone is free to use this software for any purpose, and to
// modify it for their own purposes, provided that credit is given to the author
// in a prominent place.  For the present program that means that the green
// title and author banner appearing on the main window must not be removed,
// and may not be altered without premission of the author.
//
//--- DISCLAIMER---------------------------------------------------------------
//
//	This software is provided "as is" without any warranty of any kind, either
//	express, implied, or statutory, including, but not limited to, any
//	warranty that the software will conform to specification, any implied
//	warranties of merchantability, fitness for a particular purpose, and
//	freedom from infringement, and any warranty that the documentation will
//	conform to the program, or any warranty that the software will be error
//	free.
//
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
public class TychoCatalogFrame extends javax.swing.JFrame implements WindowListener{
    Tycho2Reader                 myTycho2Reader;
    HTMIndexSearchClient         myHTMIndexSearchClient         = new HTMIndexSearchClient();
    public static int            TELESCOPE = 1;
    public static int            USER      = 2;
    public static int            RESOLVED  = 3;
    private       int            source;
    private       double         box_size;
    private       double         min_magnitude;
    private       double         max_magnitude;
    private       double         DEFAULT_MIN_MAGNITUDE = -1;
    private       double         DEFAULT_MAX_MAGNITUDE = 20.0;
    private       double         DEFAULT_SEARCH_BOX_SIZE = 60.0;
    private       double         search_RA;
    private       double         search_DEC;
    private       double         telescope_RA;
    private       double         telescope_DEC;
    private       double         user_RA;
    private       double         user_DEC;
    public        P200Component  myP200Component;
    public        TelescopeObject myTelescopeObject;
    private       TableRowSorter<Tycho2TableModel>  sorter;
    private  java.lang.String     selected_type = new java.lang.String();
    private  java.lang.String     spectral_type = new java.lang.String();
    private  java.lang.String     luminosity    = new java.lang.String();
    private       Celest          celest;
//    public        GuideStarDisplayFrame myGuideStarDisplayFrame;
    public        DS9ImageDisplayClient myDS9ImageDisplayClient;
    public  int                   image_size;
    private   ImageIcon             ONImageIcon;
    private   ImageIcon             OFFImageIcon;
    public    java.lang.String      USERDIR           = System.getProperty("user.dir");
    public    java.lang.String      SEP               = System.getProperty("file.separator");
    public    java.lang.String      IMAGE_CACHE       = SEP + "images" + SEP;
    public    java.lang.String      CONFIG            = SEP + "config" + SEP;
/*================================================================================================
/      TychoCatalogFrame()
/=================================================================================================*/
   public TychoCatalogFrame() {
        initComponents();
        initializeTableModel();
        intialize();
        setSearchBoxSize(DEFAULT_SEARCH_BOX_SIZE);
        setMinMagnitude(DEFAULT_MIN_MAGNITUDE);
        setMaxMagnitude(DEFAULT_MAX_MAGNITUDE);
        UpdateButton.setEnabled(false);
        initializeComboBoxes();
        this.setImageSize(30);
        this.setVisible(false);
        this.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
    }
/*===============================================================================================================
/    setSource(int new_source)
/===============================================================================================================*/
 public void test(){
     myP200Component = new P200Component();
     setP200Component(myP200Component);
 }
/*===============================================================================================================
/   setGuideStarDisplayFrame(GuideStarDisplayFrame newGuideStarDisplayFrame)
/===============================================================================================================*/
//  public void setGuideStarDisplayFrame(GuideStarDisplayFrame newGuideStarDisplayFrame){
//    myGuideStarDisplayFrame  = newGuideStarDisplayFrame;
//  }
/*===============================================================================================================
/    setSource(int new_source)
/===============================================================================================================*/
  public void setP200Component(P200Component newP200Component){
      myP200Component = newP200Component;
      setTelescopeObject(myP200Component.getTelescopeObject());
      myTycho2Reader.setCatalogFile(myP200Component.myTelescopesIniReader.TYCHO2_CATALOG);
      myTycho2Reader.setIndexFile(myP200Component.myTelescopesIniReader.TYCHO2_INDEX);
        myP200Component.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            P200Component_propertyChange(e);
         }
        });     
  }
  /*================================================================================================
/     setTelescopeObject(TelescopeObject newTelescopeObject)
/=================================================================================================*/
 public void setTelescopeObject(TelescopeObject newTelescopeObject){
     myTelescopeObject = newTelescopeObject;
     myTelescopeObject.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
      public void propertyChange(java.beans.PropertyChangeEvent e) {
          TelescopeObject_propertyChange(e);
       }
     });
    }
 /*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void P200Component_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
       if(propertyName == "connected"){
        boolean state = (java.lang.Boolean)e.getNewValue();
        if(state){
//          ConnectToTCSToggleButton.setText("Connected to Telescope");
//          UpdateToggleButton.setEnabled(true);
//          UpdateToggleButton.setText("Update");
        }
        if(!state){
//          ConnectToTCSToggleButton.setText("Connect to Telescope");
//          UpdateToggleButton.setEnabled(false);
//          UpdateToggleButton.setText("");
        }
      }
      if(propertyName == "coord_message"){
         java.lang.String statusMessage = (java.lang.String)e.getNewValue();
         StatusMessage.setText(statusMessage);
      }
   }
  /*=============================================================================================
/     Property Change Listener for the _propertyChange
/=============================================================================================*/
  private void TelescopeObject_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
/*=============================================================================================
/  Simple Test Coordinates
/   M57       RA 18:53:35.01  DEC +33:01:42:86
/   IC1296    RA 18:53:18.83  DEC +33:03:59.70
/=============================================================================================*/
     if(propertyName == "right_ascension"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        CurrentRATextField.setText(newValue);
    }
    if(propertyName == "declination"){
        java.lang.String newValue = (java.lang.String)e.getNewValue();
        CurrentDecTextField.setText(newValue);
    }
     if(propertyName == "RA"){
        java.lang.Double newValue = (java.lang.Double)e.getNewValue();
        setTelescopeRA(newValue);
    }
    if(propertyName == "DEC"){
        java.lang.Double newValue = (java.lang.Double)e.getNewValue();
        setTelescopeDEC(newValue);
    }
  }
/*===============================================================================================================
/    setSource(int new_source)
/===============================================================================================================*/
  private void setSource(int new_source){
     this.source = new_source;
  }
  private int getSource(){
      return source;
  }
/*===============================================================================================================
/    setSearchBoxSize(int new_box_size){
/===============================================================================================================*/
  public void setSearchBoxSize(double new_box_size){
      box_size = new_box_size;
  }
  public double getSearchBoxSize(){
      return box_size;
  }
/*===============================================================================================================
/   setSearchRA(double new_search_RA)
/===============================================================================================================*/
  public void setSearchRA(double new_search_RA){
      search_RA = new_search_RA;
  }
  public double getSearchRA(){
      return search_RA;
  }
 /*===============================================================================================================
/   setSearchRA(double new_search_RA)
/===============================================================================================================*/
  public void setSearchDEC(double new_search_DEC){
      search_DEC = new_search_DEC;
  }
  public double getSearchDEC(){
      return search_DEC;
  }
/*===============================================================================================================
/   setSearchRA(double new_search_RA)
/===============================================================================================================*/
  public void setUserRA(double new_user_RA){
      user_RA = new_user_RA;
  }
  public double getUserRA(){
      return user_RA;
  }
 /*===============================================================================================================
/   setSearchRA(double new_search_RA)
/===============================================================================================================*/
  public void setUserDEC(double new_user_DEC){
      user_DEC = new_user_DEC;
  }
  public double getUserDEC(){
      return user_DEC;
  }
/*===============================================================================================================
/   setSearchRA(double new_search_RA)
/===============================================================================================================*/
  public void setTelescopeRA(double new_telescope_RA){
      telescope_RA = new_telescope_RA;
  }
  public double getTelescopeRA(){
      return telescope_RA;
  }
 /*===============================================================================================================
/   setSearchRA(double new_search_RA)
/===============================================================================================================*/
  public void setTelescopeDEC(double new_telescope_DEC){
      telescope_DEC = new_telescope_DEC;
  }
  public double getTelescopeDEC(){
      return telescope_DEC;
  }
/*===============================================================================================================
/    setMinMagnitude(int new_min_magnitude)
/===============================================================================================================*/
  public void setMinMagnitude(double new_min_magnitude){
      min_magnitude = new_min_magnitude;
  }
  public double getMinMagnitude(){
      return min_magnitude;
  }
/*===============================================================================================================
/    setMaxnMagnitude(int new_max_magnitude)
/===============================================================================================================*/
  public void setMaxMagnitude(double new_max_magnitude){
      max_magnitude = new_max_magnitude;
  }
  public double getMaxMagnitude(){
      return max_magnitude;
  }
/*===============================================================================================================
/    setMaxnMagnitude(int new_max_magnitude)
/===============================================================================================================*/
  public void setSpectralType(java.lang.String new_spectral_type){
      spectral_type = new_spectral_type;
  }
  public java.lang.String geSpectralType(){
      return spectral_type;
  }
/*===============================================================================================================
/    setMaxnMagnitude(int new_max_magnitude)
/===============================================================================================================*/
  public void setLuminosity(java.lang.String new_luminosity){
      luminosity = new_luminosity;
  }
  public java.lang.String geLuminosity(){
      return luminosity;
  }
/*===============================================================================================================
/    GetCoordinatesButton  Button ActionListener
/===============================================================================================================*/
  private void intialize(){
      CoordinateSourceButtonGroup.add(ResolvedRadioButton);
      CoordinateSourceButtonGroup.add(TelescopeRadioButton);
      CoordinateSourceButtonGroup.add(UserRadioButton);
      ResolutionSourceButtonGroup.add(SIMBADRadioButton);
      ResolutionSourceButtonGroup.add(NEDRadioButton);
      TelescopeRadioButton.setSelected(true);
      SIMBADRadioButton.setSelected(true);
      setSource(TELESCOPE);
      myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.SIMBAD);
      MinMagnitudeTextField.setText(Double.toString(DEFAULT_MIN_MAGNITUDE));
      MaxMagnitudeTextField.setText(Double.toString(DEFAULT_MAX_MAGNITUDE));
      SearchBoxSizeTextField.setText(Double.toString(DEFAULT_SEARCH_BOX_SIZE));
      ONImageIcon                            = new ImageIcon(USERDIR + IMAGE_CACHE + "ON.gif");
      OFFImageIcon                           = new ImageIcon(USERDIR + IMAGE_CACHE + "OFF.gif");
      DS9ToggleButton.setIcon(ONImageIcon);
      DS9ToggleButton.setSelected(true);
/*===============================================================================================================
/    Add property change listener for the HTMIndexSearchClient
/===============================================================================================================*/
     myHTMIndexSearchClient.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myHTMIndexSearchClient_propertyChange(e);
         }
     });
/*===============================================================================================================
/    GetCoordinatesButton  Button ActionListener
/===============================================================================================================*/
     GetCoordinatesButton.addActionListener(new java.awt.event.ActionListener(){
         public void actionPerformed(java.awt.event.ActionEvent e){
                   try{
                     java.lang.String currentObjectName = ObjectNameTextField.getText();
                     myHTMIndexSearchClient.setObjectName(currentObjectName);
                     double[]  coordinateArray = myHTMIndexSearchClient.resolveObject(currentObjectName);
                     myHTMIndexSearchClient.setRA(coordinateArray[0]);
                     myHTMIndexSearchClient.setDEC(coordinateArray[1]);
                     Coordinates      configuredCoordinates = new Coordinates();
                                      configuredCoordinates.setSeparatorStyle(Coordinates.COLON_SEPARATOR_STYLE);
                                      configuredCoordinates.setRa(coordinateArray[0]);
                                      configuredCoordinates.setDec(coordinateArray[1]);
                     java.lang.String newRaString           = configuredCoordinates.raToString();
                     java.lang.String newDecString          = configuredCoordinates.decToString();
                     myHTMIndexSearchClient.setRAString(newRaString);
                     myHTMIndexSearchClient.setDecString(newDecString);
                     logMessage("RA String = " + newRaString + " " + "RA Double = " + coordinateArray[0]);
                     logMessage("Dec String = " + newDecString + " " + "Dec Double = " + coordinateArray[1]);
                   }
                   catch(Exception e2){
                       java.lang.String myErrorString = new java.lang.String();
                       myErrorString = "A problem occured while executing the Get Coordinates method: " + e2;
                       logMessage(myErrorString);
                         ObjectNameTextField.setText("");
                         myHTMIndexSearchClient.setRA(0.0);
                         myHTMIndexSearchClient.setDEC(0.0);
                         myHTMIndexSearchClient.setRAString("");
                         myHTMIndexSearchClient.setDecString("");
                  }
                 }
        });
/*===============================================================================================================
/    SIMBADRadioButton Action Listener
/===============================================================================================================*/
    SIMBADRadioButton.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(java.awt.event.ActionEvent e){
                   boolean myNewValue =  SIMBADRadioButton.isSelected();
                     try{
                         if(myNewValue){
                           myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.SIMBAD);
                         }
                     }
                     catch(Exception e2){
                         java.lang.String myErrorString = new java.lang.String();
                         myErrorString = "A problem occured while setting the Name Resolution Source. ";
                         logMessage(myErrorString + myNewValue);
                     }
                   }
      });
/*===============================================================================================================
/    NEDRadioButton Action Listener
/===============================================================================================================*/
    NEDRadioButton.addActionListener(new java.awt.event.ActionListener(){
       public void actionPerformed(java.awt.event.ActionEvent e){
                     boolean myNewValue =  NEDRadioButton.isSelected();
                       try{
                           if(myNewValue){
                             myHTMIndexSearchClient.setResolutionSource(HTMIndexSearchClient.NED);
                           }
                       }
                       catch(Exception e2){
                         java.lang.String myErrorString = new java.lang.String();
                         myErrorString = "A problem occured while setting the Name Resolution Source. ";
                         logMessage(myErrorString + myNewValue);
                       }
                   }
        });
/*===============================================================================================================
/     ObjectNameTextField ActionListener   -  Object Text Field
/===============================================================================================================*/
   ObjectNameTextField.addActionListener(new java.awt.event.ActionListener(){
      public void actionPerformed(java.awt.event.ActionEvent e){
        java.lang.String myNewString = new java.lang.String();
        myNewString =  ObjectNameTextField.getText();
          try{
            myHTMIndexSearchClient.setObjectName(myNewString);
          }
          catch(Exception e2){
            java.lang.String myErrorString = new java.lang.String();
            myErrorString = "A problem occured while setting the Object Name.  Attempted Value = ";
            logMessage(myErrorString + myNewString);
          }
        }
      });
    }
/*===============================================================================================================
/       Property Change Listener for the Detector Server Bean
/===============================================================================================================*/
  public void myHTMIndexSearchClient_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
  // Integration Control Related Properties
        if(propertyName.matches("RAString")){
          java.lang.String myValue  = (java.lang.String)e.getNewValue();
          RATextField.setText(myValue);
        }
        if(propertyName.matches("DecString")){
          java.lang.String myValue  = (java.lang.String)e.getNewValue();
           DECTextField.setText(myValue);
       }
    }// end of propertyChange Listener for the Detector Server Bean
/*===============================================================================================================
/       Property Change Listener for the Detector Server Bean
/===============================================================================================================*/
  public void myTycho2Reader_propertyChange(PropertyChangeEvent e)  {
     java.lang.String propertyName = e.getPropertyName();
  // Integration Control Related Properties
        if(propertyName.matches("initialized")){
          java.lang.Boolean myValue  = (java.lang.Boolean)e.getNewValue();
          UpdateButton.setEnabled(myValue.booleanValue());
        }
        if(propertyName.matches("selectedRA")){
          java.lang.Double myValue  = (java.lang.Double)e.getNewValue();
 //         celest               = new Celest(myTycho2Reader.getSelectedRA(),myTycho2Reader.getSelectedDEC(),2000);
          RA newRA = new RA(myValue.doubleValue());
          java.lang.String ra_string  = newRA.RoundedRAString(2, ":");
          selectedRATextField.setText(ra_string);
         }
        if(propertyName.matches("selectedDEC")){
          java.lang.Double myValue  = (java.lang.Double)e.getNewValue();
          celest               = new Celest(myTycho2Reader.getSelectedRA(),myTycho2Reader.getSelectedDEC(),2000);
          java.lang.String dec_string  = celest.Delta.RoundedDecString(2,":");
          selectedDECTextField.setText(dec_string);
          }
    }// end of propertyChange Listener for the Detector Server Bean
/*=============================================================================================
/    logMessage(java.lang.String message)
/=============================================================================================*/
 public void logMessage(java.lang.String message){
     System.out.println(message);
 }
/*=============================================================================================
/    setImageTableModel(DefaultTableModel newDefaultTableModel)
/=============================================================================================*/
 public void initializeComboBoxes(){
    SpecTypeComboBox.removeAllItems();
    LuminosityComboBox.removeAllItems();
    SpecTypeComboBox.addItem("ALL");
    SpecTypeComboBox.addItem("O");
    SpecTypeComboBox.addItem("B");
    SpecTypeComboBox.addItem("A");
    SpecTypeComboBox.addItem("F");
    SpecTypeComboBox.addItem("G");
    SpecTypeComboBox.addItem("K");
    SpecTypeComboBox.addItem("M");
    LuminosityComboBox.addItem("ALL");
    LuminosityComboBox.addItem("0");
    LuminosityComboBox.addItem("1");
    LuminosityComboBox.addItem("2");
    LuminosityComboBox.addItem("3");
    LuminosityComboBox.addItem("4");
    LuminosityComboBox.addItem("5");
    LuminosityComboBox.addItem("6");
    LuminosityComboBox.addItem("7");
    SpecTypeComboBox.setSelectedItem("ALL");
    LuminosityComboBox.setSelectedItem("ALL");
    ImageSizeComboBox.removeAllItems();
    ImageSizeComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "15x15 arcmin", "30x30 arcmin", "60x60 arcmin" }));
    ImageSizeComboBox.setSelectedIndex(1);
 }
/*=============================================================================================
/    Tycho2Reader getTycho2Reader()
/=============================================================================================*/
 public Tycho2Reader getTycho2Reader(){
     return myTycho2Reader;
 }
 /*================================================================================================
/    setImageSize(double newBOX_SIZE)
/=================================================================================================*/
 public void setImageSize(int newIMAGE_SIZE){
     image_size = newIMAGE_SIZE;
 }
 public int getImageSize(){
     return image_size;
 }
 /*=============================================================================================
/    setImageTableModel(DefaultTableModel newDefaultTableModel)
/=============================================================================================*/
public void initializeTableModel(){
    myTycho2Reader = new Tycho2Reader();
    myTycho2Reader.addPropertyChangeListener(new java.beans.PropertyChangeListener() {
        public void propertyChange(java.beans.PropertyChangeEvent e) {
            myTycho2Reader_propertyChange(e);
         }
        });
    TychoObjectsTable.setModel(myTycho2Reader.getTycho2TableModel());
    TychoObjectsTable.getSelectionModel().addListSelectionListener(new SharedListSelectionHandler());
        int endColumn = TychoObjectsTable.getColumnCount()-1;

        TychoObjectsTable.setAutoResizeMode(TychoObjectsTable.AUTO_RESIZE_OFF);
        TychoObjectsTable.getColumnModel().getColumn(0).setMinWidth(120);
        TychoObjectsTable.getColumnModel().getColumn(1).setMinWidth(120);
        TychoObjectsTable.getColumnModel().getColumn(2).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(3).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(4).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(5).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(6).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(7).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(8).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(9).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(10).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(11).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(12).setMinWidth(150);
        TychoObjectsTable.getColumnModel().getColumn(13).setMinWidth(150);

        TychoObjectsTable.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        TychoObjectsTable.setRowSelectionAllowed(true);

   TychoObjectsTable.setBackground(Color.CYAN);
   TychoObjectsTable.setAutoCreateRowSorter(true);
}
/*=============================================================================================
/    setImageTableModel(DefaultTableModel newDefaultTableModel)
/=============================================================================================*/
public void filterResults(){
  RowFilter<Tycho2TableModel,Integer> spec_type_filter = new RowFilter<Tycho2TableModel,Integer>() {
      public boolean include(Entry<? extends Tycho2TableModel, ? extends Integer> entry) {
         Tycho2TableModel currentModel = entry.getModel();
         tycho2star star = (tycho2star)currentModel.getRecord(entry.getIdentifier());
        boolean  spectral_type_check = star.temperature_class.contains(spectral_type);
        boolean  luminosity_check    = star.temperature_subclass.contains(luminosity);
        if(spectral_type.matches("ALL")){
            spectral_type_check = true;
        }
       if(luminosity.matches("ALL")){
            luminosity_check = true;
        }
         boolean magnitude_check     = (star.magnitude >= getMinMagnitude())&(star.magnitude <= getMaxMagnitude());
         if(magnitude_check&spectral_type_check&luminosity_check){
           return true;
         }
         return false;
       }
    };
    sorter = new TableRowSorter<Tycho2TableModel>(myTycho2Reader.getTycho2TableModel());
    sorter.setRowFilter(spec_type_filter);
    TychoObjectsTable.setRowSorter(sorter);
    }
/** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        CoordinateSourceButtonGroup = new javax.swing.ButtonGroup();
        ResolutionSourceButtonGroup = new javax.swing.ButtonGroup();
        jPanel1 = new javax.swing.JPanel();
        jPanel2 = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        TychoObjectsTable = new javax.swing.JTable();
        jLabel15 = new javax.swing.JLabel();
        jPanel5 = new javax.swing.JPanel();
        jPanel6 = new javax.swing.JPanel();
        TelescopeRadioButton = new javax.swing.JRadioButton();
        UserRadioButton = new javax.swing.JRadioButton();
        ResolvedRadioButton = new javax.swing.JRadioButton();
        jPanel7 = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        UserRATextField = new javax.swing.JTextField();
        jLabel3 = new javax.swing.JLabel();
        UserDECTextField = new javax.swing.JTextField();
        jLabel4 = new javax.swing.JLabel();
        jPanel8 = new javax.swing.JPanel();
        jLabel5 = new javax.swing.JLabel();
        CurrentRATextField = new javax.swing.JTextField();
        jLabel6 = new javax.swing.JLabel();
        CurrentDecTextField = new javax.swing.JTextField();
        jLabel7 = new javax.swing.JLabel();
        jPanel9 = new javax.swing.JPanel();
        UpdateButton = new javax.swing.JButton();
        UpdateToggleButton = new javax.swing.JToggleButton();
        jPanel3 = new javax.swing.JPanel();
        ObjectNameTextField = new javax.swing.JTextField();
        SIMBADRadioButton = new javax.swing.JRadioButton();
        NEDRadioButton = new javax.swing.JRadioButton();
        RATextField = new javax.swing.JTextField();
        DECTextField = new javax.swing.JTextField();
        GetCoordinatesButton = new javax.swing.JButton();
        jLabel1 = new javax.swing.JLabel();
        jLabel13 = new javax.swing.JLabel();
        jLabel14 = new javax.swing.JLabel();
        ObjectNameRetrieveImageButton = new javax.swing.JButton();
        jPanel10 = new javax.swing.JPanel();
        SearchBoxSizeTextField = new javax.swing.JTextField();
        jLabel8 = new javax.swing.JLabel();
        jLabel9 = new javax.swing.JLabel();
        jLabel10 = new javax.swing.JLabel();
        jLabel11 = new javax.swing.JLabel();
        MaxMagnitudeTextField = new javax.swing.JTextField();
        MinMagnitudeTextField = new javax.swing.JTextField();
        jLabel12 = new javax.swing.JLabel();
        LuminosityComboBox = new javax.swing.JComboBox();
        jLabel17 = new javax.swing.JLabel();
        SpecTypeComboBox = new javax.swing.JComboBox();
        jPanel11 = new javax.swing.JPanel();
        jLabel18 = new javax.swing.JLabel();
        jLabel19 = new javax.swing.JLabel();
        selectedRATextField = new javax.swing.JTextField();
        jLabel20 = new javax.swing.JLabel();
        selectedDECTextField = new javax.swing.JTextField();
        SendToTelescopeButton = new javax.swing.JButton();
        jLabel21 = new javax.swing.JLabel();
        StatusMessage = new javax.swing.JLabel();
        RetrieveImageButton = new javax.swing.JButton();
        jLabel22 = new javax.swing.JLabel();
        jLabel16 = new javax.swing.JLabel();
        ImageSizeComboBox = new javax.swing.JComboBox();
        DS9ToggleButton = new javax.swing.JToggleButton();
        jLabel23 = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        jPanel1.setBackground(new java.awt.Color(12, 10, 7));
        jPanel1.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jPanel2.setBackground(new java.awt.Color(9, 9, 8));
        jPanel2.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));

        jScrollPane1.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPane1.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);

        TychoObjectsTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null}
            },
            new String [] {
                "Title 1", "Title 2", "Title 3", "Title 4"
            }
        ));
        jScrollPane1.setViewportView(TychoObjectsTable);

        jLabel15.setFont(new java.awt.Font("SansSerif", 1, 18));
        jLabel15.setForeground(new java.awt.Color(253, 148, 28));
        jLabel15.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel15.setText("Objects matching spectral type and magnitude criteria");

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel15, javax.swing.GroupLayout.PREFERRED_SIZE, 1070, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jScrollPane1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 1072, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(jLabel15)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 205, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(17, Short.MAX_VALUE))
        );

        jPanel1.add(jPanel2, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 320, 1100, 260));

        jPanel5.setBackground(new java.awt.Color(7, 6, 4));
        jPanel5.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));
        jPanel5.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jPanel6.setBackground(new java.awt.Color(9, 9, 8));
        jPanel6.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));
        jPanel6.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        TelescopeRadioButton.setBackground(new java.awt.Color(0, 0, 0));
        TelescopeRadioButton.setForeground(new java.awt.Color(5, 183, 54));
        TelescopeRadioButton.setText("Use Current Telescope Coordinates");
        TelescopeRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TelescopeRadioButtonActionPerformed(evt);
            }
        });
        jPanel6.add(TelescopeRadioButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 12, -1, 10));

        UserRadioButton.setBackground(new java.awt.Color(0, 0, 0));
        UserRadioButton.setForeground(new java.awt.Color(5, 183, 54));
        UserRadioButton.setText("User Supplied Coordinates");
        UserRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UserRadioButtonActionPerformed(evt);
            }
        });
        jPanel6.add(UserRadioButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 32, -1, 10));

        ResolvedRadioButton.setBackground(new java.awt.Color(0, 0, 0));
        ResolvedRadioButton.setForeground(new java.awt.Color(5, 183, 54));
        ResolvedRadioButton.setText("Resolved Object Coordinates");
        ResolvedRadioButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ResolvedRadioButtonActionPerformed(evt);
            }
        });
        jPanel6.add(ResolvedRadioButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 52, -1, 10));

        jPanel5.add(jPanel6, new org.netbeans.lib.awtextra.AbsoluteConstraints(570, 140, 280, 70));

        jPanel7.setBackground(new java.awt.Color(9, 9, 8));
        jPanel7.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));

        jLabel2.setForeground(new java.awt.Color(2, 200, 192));
        jLabel2.setText("RA");

        UserRATextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UserRATextFieldActionPerformed(evt);
            }
        });

        jLabel3.setForeground(new java.awt.Color(2, 200, 192));
        jLabel3.setText("DEC");

        UserDECTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UserDECTextFieldActionPerformed(evt);
            }
        });

        jLabel4.setForeground(new java.awt.Color(2, 200, 192));
        jLabel4.setText("User Coordinates");

        javax.swing.GroupLayout jPanel7Layout = new javax.swing.GroupLayout(jPanel7);
        jPanel7.setLayout(jPanel7Layout);
        jPanel7Layout.setHorizontalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel4)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(UserRATextField, javax.swing.GroupLayout.DEFAULT_SIZE, 154, Short.MAX_VALUE)
                .addGap(18, 18, 18)
                .addComponent(jLabel3)
                .addGap(18, 18, 18)
                .addComponent(UserDECTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 143, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        jPanel7Layout.setVerticalGroup(
            jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel7Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel7Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel4)
                    .addComponent(UserDECTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel3)
                    .addComponent(jLabel2)
                    .addComponent(UserRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel5.add(jPanel7, new org.netbeans.lib.awtextra.AbsoluteConstraints(567, 74, -1, -1));

        jPanel8.setBackground(new java.awt.Color(9, 9, 8));
        jPanel8.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));

        jLabel5.setForeground(new java.awt.Color(2, 200, 192));
        jLabel5.setText("RA");

        CurrentRATextField.setBackground(new java.awt.Color(16, 13, 9));
        CurrentRATextField.setEditable(false);
        CurrentRATextField.setFont(new java.awt.Font("SansSerif", 1, 18));
        CurrentRATextField.setForeground(new java.awt.Color(4, 206, 23));
        CurrentRATextField.setBorder(null);

        jLabel6.setForeground(new java.awt.Color(2, 200, 192));
        jLabel6.setText("DEC");

        CurrentDecTextField.setBackground(new java.awt.Color(16, 13, 9));
        CurrentDecTextField.setEditable(false);
        CurrentDecTextField.setFont(new java.awt.Font("SansSerif", 1, 18));
        CurrentDecTextField.setForeground(new java.awt.Color(4, 206, 23));
        CurrentDecTextField.setBorder(null);

        jLabel7.setForeground(new java.awt.Color(2, 200, 192));
        jLabel7.setText("Telescope Coordinates");

        javax.swing.GroupLayout jPanel8Layout = new javax.swing.GroupLayout(jPanel8);
        jPanel8.setLayout(jPanel8Layout);
        jPanel8Layout.setHorizontalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel7)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel5)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(CurrentRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 132, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel6)
                .addGap(18, 18, 18)
                .addComponent(CurrentDecTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 135, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(14, Short.MAX_VALUE))
        );
        jPanel8Layout.setVerticalGroup(
            jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel8Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel8Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel7)
                    .addComponent(CurrentRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel5)
                    .addComponent(jLabel6)
                    .addComponent(CurrentDecTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 29, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jPanel5.add(jPanel8, new org.netbeans.lib.awtextra.AbsoluteConstraints(567, 13, 540, -1));

        jPanel9.setBackground(new java.awt.Color(9, 9, 8));
        jPanel9.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));

        UpdateButton.setBackground(new java.awt.Color(0, 125, 0));
        UpdateButton.setFont(new java.awt.Font("SansSerif", 1, 14));
        UpdateButton.setForeground(new java.awt.Color(255, 255, 255));
        UpdateButton.setText("Find Spectral Type Stars");
        UpdateButton.setBorder(new javax.swing.border.SoftBevelBorder(javax.swing.border.BevelBorder.RAISED));
        UpdateButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UpdateButtonActionPerformed(evt);
            }
        });

        UpdateToggleButton.setText("Update");
        UpdateToggleButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                UpdateToggleButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel9Layout = new javax.swing.GroupLayout(jPanel9);
        jPanel9.setLayout(jPanel9Layout);
        jPanel9Layout.setHorizontalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel9Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(UpdateButton, javax.swing.GroupLayout.PREFERRED_SIZE, 215, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(90, 90, 90)
                .addComponent(UpdateToggleButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        jPanel9Layout.setVerticalGroup(
            jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel9Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel9Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel9Layout.createSequentialGroup()
                        .addComponent(UpdateToggleButton)
                        .addGap(31, 31, 31))
                    .addGroup(jPanel9Layout.createSequentialGroup()
                        .addComponent(UpdateButton, javax.swing.GroupLayout.DEFAULT_SIZE, 48, Short.MAX_VALUE)
                        .addContainerGap())))
        );

        jPanel5.add(jPanel9, new org.netbeans.lib.awtextra.AbsoluteConstraints(860, 140, 240, 70));

        jPanel3.setBackground(new java.awt.Color(9, 9, 8));
        jPanel3.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));
        jPanel3.setForeground(new java.awt.Color(2, 200, 192));
        jPanel3.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());
        jPanel3.add(ObjectNameTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(110, 14, 180, -1));

        SIMBADRadioButton.setBackground(new java.awt.Color(0, 0, 0));
        SIMBADRadioButton.setForeground(new java.awt.Color(5, 183, 54));
        SIMBADRadioButton.setText("SIMBAD");
        jPanel3.add(SIMBADRadioButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(436, 2, -1, -1));

        NEDRadioButton.setBackground(new java.awt.Color(0, 0, 0));
        NEDRadioButton.setForeground(new java.awt.Color(5, 183, 54));
        NEDRadioButton.setText("NED");
        jPanel3.add(NEDRadioButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(436, 30, -1, -1));

        RATextField.setBackground(new java.awt.Color(0, 0, 0));
        RATextField.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14));
        RATextField.setForeground(new java.awt.Color(0, 200, 0));
        RATextField.setText(" ");
        RATextField.setBorder(null);
        jPanel3.add(RATextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(190, 50, 105, 23));

        DECTextField.setBackground(new java.awt.Color(0, 0, 0));
        DECTextField.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14));
        DECTextField.setForeground(new java.awt.Color(0, 200, 0));
        DECTextField.setText(" ");
        DECTextField.setBorder(null);
        jPanel3.add(DECTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 50, 105, 23));

        GetCoordinatesButton.setText("Get Coordinates");
        jPanel3.add(GetCoordinatesButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(14, 47, -1, -1));

        jLabel1.setForeground(new java.awt.Color(2, 200, 192));
        jLabel1.setText("Object Name");
        jPanel3.add(jLabel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(14, 19, -1, -1));

        jLabel13.setForeground(new java.awt.Color(2, 200, 192));
        jLabel13.setText("RA");
        jPanel3.add(jLabel13, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 50, -1, -1));

        jLabel14.setForeground(new java.awt.Color(2, 200, 192));
        jLabel14.setText("DEC");
        jPanel3.add(jLabel14, new org.netbeans.lib.awtextra.AbsoluteConstraints(310, 50, -1, -1));

        ObjectNameRetrieveImageButton.setText("Retrieve Image");
        ObjectNameRetrieveImageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ObjectNameRetrieveImageButtonActionPerformed(evt);
            }
        });
        jPanel3.add(ObjectNameRetrieveImageButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(300, 10, -1, -1));

        jPanel5.add(jPanel3, new org.netbeans.lib.awtextra.AbsoluteConstraints(13, 119, 550, 90));

        jPanel10.setBackground(new java.awt.Color(9, 9, 8));
        jPanel10.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));
        jPanel10.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        SearchBoxSizeTextField.setText(" ");
        SearchBoxSizeTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SearchBoxSizeTextFieldActionPerformed(evt);
            }
        });
        jPanel10.add(SearchBoxSizeTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(200, 30, 138, -1));

        jLabel8.setForeground(new java.awt.Color(2, 200, 192));
        jLabel8.setText("Search Box Size (arcmin)");
        jPanel10.add(jLabel8, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 30, -1, -1));

        jLabel9.setForeground(new java.awt.Color(252, 173, 26));
        jLabel9.setText("Catalog Search Criteria");
        jPanel10.add(jLabel9, new org.netbeans.lib.awtextra.AbsoluteConstraints(100, 10, -1, -1));

        jLabel10.setForeground(new java.awt.Color(2, 200, 192));
        jLabel10.setText("Min Magnitude");
        jPanel10.add(jLabel10, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 60, -1, -1));

        jLabel11.setForeground(new java.awt.Color(2, 200, 192));
        jLabel11.setText("Max Magnitude");
        jPanel10.add(jLabel11, new org.netbeans.lib.awtextra.AbsoluteConstraints(260, 70, -1, -1));

        MaxMagnitudeTextField.setText(" ");
        MaxMagnitudeTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                MaxMagnitudeTextFieldActionPerformed(evt);
            }
        });
        jPanel10.add(MaxMagnitudeTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(370, 60, 160, -1));

        MinMagnitudeTextField.setText(" ");
        MinMagnitudeTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                MinMagnitudeTextFieldActionPerformed(evt);
            }
        });
        jPanel10.add(MinMagnitudeTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(120, 60, 138, -1));

        jLabel12.setForeground(new java.awt.Color(2, 200, 192));
        jLabel12.setText("Luminosity");
        jPanel10.add(jLabel12, new org.netbeans.lib.awtextra.AbsoluteConstraints(450, 10, -1, -1));

        LuminosityComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        LuminosityComboBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                LuminosityComboBoxActionPerformed(evt);
            }
        });
        jPanel10.add(LuminosityComboBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(440, 30, 90, -1));

        jLabel17.setForeground(new java.awt.Color(2, 200, 192));
        jLabel17.setText("Spectral Type");
        jPanel10.add(jLabel17, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 10, -1, -1));

        SpecTypeComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        SpecTypeComboBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SpecTypeComboBoxActionPerformed(evt);
            }
        });
        jPanel10.add(SpecTypeComboBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(350, 30, 90, -1));

        jPanel5.add(jPanel10, new org.netbeans.lib.awtextra.AbsoluteConstraints(17, 13, 544, 100));

        jPanel11.setBackground(new java.awt.Color(8, 7, 6));
        jPanel11.setBorder(new javax.swing.border.LineBorder(new java.awt.Color(0, 200, 0), 2, true));

        jLabel18.setForeground(new java.awt.Color(2, 200, 192));
        jLabel18.setText("Selected Coordinates");

        jLabel19.setForeground(new java.awt.Color(2, 200, 192));
        jLabel19.setText("RA");

        selectedRATextField.setBackground(new java.awt.Color(0, 0, 0));
        selectedRATextField.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14));
        selectedRATextField.setForeground(new java.awt.Color(0, 200, 0));
        selectedRATextField.setText(" ");
        selectedRATextField.setBorder(null);

        jLabel20.setForeground(new java.awt.Color(2, 200, 192));
        jLabel20.setText("DEC");

        selectedDECTextField.setBackground(new java.awt.Color(0, 0, 0));
        selectedDECTextField.setFont(new java.awt.Font("DejaVu LGC Sans", 1, 14));
        selectedDECTextField.setForeground(new java.awt.Color(0, 200, 0));
        selectedDECTextField.setText(" ");
        selectedDECTextField.setBorder(null);

        SendToTelescopeButton.setText("Send To Telescope");
        SendToTelescopeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SendToTelescopeButtonActionPerformed(evt);
            }
        });

        jLabel21.setForeground(new java.awt.Color(2, 200, 192));
        jLabel21.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        jLabel21.setText("Status Message");

        StatusMessage.setForeground(new java.awt.Color(245, 243, 243));
        StatusMessage.setText(" ");

        RetrieveImageButton.setText("Retrieve Image");
        RetrieveImageButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                RetrieveImageButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout jPanel11Layout = new javax.swing.GroupLayout(jPanel11);
        jPanel11.setLayout(jPanel11Layout);
        jPanel11Layout.setHorizontalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel18)
                .addGap(10, 10, 10)
                .addComponent(jLabel19)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(selectedRATextField, javax.swing.GroupLayout.PREFERRED_SIZE, 105, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel20)
                .addGap(18, 18, 18)
                .addComponent(selectedDECTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 105, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(RetrieveImageButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(SendToTelescopeButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel11Layout.createSequentialGroup()
                        .addComponent(StatusMessage, javax.swing.GroupLayout.DEFAULT_SIZE, 373, Short.MAX_VALUE)
                        .addGap(12, 12, 12))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel11Layout.createSequentialGroup()
                        .addComponent(jLabel21, javax.swing.GroupLayout.PREFERRED_SIZE, 373, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())))
        );
        jPanel11Layout.setVerticalGroup(
            jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel11Layout.createSequentialGroup()
                .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel11Layout.createSequentialGroup()
                        .addGap(18, 18, 18)
                        .addComponent(StatusMessage))
                    .addGroup(jPanel11Layout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                .addComponent(jLabel18)
                                .addComponent(jLabel19))
                            .addComponent(selectedRATextField, javax.swing.GroupLayout.DEFAULT_SIZE, 29, Short.MAX_VALUE)
                            .addGroup(jPanel11Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                                .addComponent(selectedDECTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 28, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(RetrieveImageButton)
                                .addComponent(SendToTelescopeButton))
                            .addComponent(jLabel20)))
                    .addComponent(jLabel21))
                .addContainerGap())
        );

        jPanel5.add(jPanel11, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 220, 1100, 50));

        jPanel1.add(jPanel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(10, 40, 1120, 270));

        jLabel22.setFont(new java.awt.Font("SansSerif", 1, 24));
        jLabel22.setForeground(new java.awt.Color(252, 173, 26));
        jLabel22.setText("Spectral Type Catalog Search Tool");
        jPanel1.add(jLabel22, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 10, -1, -1));

        jLabel16.setForeground(new java.awt.Color(2, 200, 192));
        jLabel16.setText("Display in DS9");
        jPanel1.add(jLabel16, new org.netbeans.lib.awtextra.AbsoluteConstraints(890, 20, -1, -1));

        ImageSizeComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        ImageSizeComboBox.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ImageSizeComboBoxActionPerformed(evt);
            }
        });
        jPanel1.add(ImageSizeComboBox, new org.netbeans.lib.awtextra.AbsoluteConstraints(730, 10, 140, -1));

        DS9ToggleButton.setBackground(new java.awt.Color(8, 7, 6));
        DS9ToggleButton.setText(" ");
        DS9ToggleButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(1, 1, 1, 1));
        DS9ToggleButton.setBorderPainted(false);
        DS9ToggleButton.setContentAreaFilled(false);
        DS9ToggleButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                DS9ToggleButtonActionPerformed(evt);
            }
        });
        jPanel1.add(DS9ToggleButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(990, 20, 40, -1));

        jLabel23.setForeground(new java.awt.Color(2, 200, 192));
        jLabel23.setText("Image Retrieval Size");
        jPanel1.add(jLabel23, new org.netbeans.lib.awtextra.AbsoluteConstraints(590, 20, -1, -1));

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 1145, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, 607, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void UpdateButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UpdateButtonActionPerformed
 //       double RA     = 202.48219583;
//        double DEC    = 47.23150889;
        int currentSource = getSource();
        if(source == TELESCOPE){
           setSearchRA(getTelescopeRA());
           setSearchDEC(getTelescopeDEC());
        }
        if(source == USER){
           setSearchRA(getUserRA()*15);
           setSearchDEC(getUserDEC());
       }
        if(source == RESOLVED){
           setSearchRA(myHTMIndexSearchClient.getRA());
           setSearchDEC(myHTMIndexSearchClient.getDEC());
        }
        myTycho2Reader.query(getSearchRA(), getSearchDEC(),getSearchBoxSize());
    }//GEN-LAST:event_UpdateButtonActionPerformed

    private void TelescopeRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TelescopeRadioButtonActionPerformed
        boolean status = TelescopeRadioButton.isSelected();
        if(status){
           setSource(TELESCOPE);
           setSearchRA(getTelescopeRA());
           setSearchDEC(getTelescopeDEC());
        }
    }//GEN-LAST:event_TelescopeRadioButtonActionPerformed

    private void UserRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UserRadioButtonActionPerformed
       boolean status = UserRadioButton.isSelected();
        if(status){
           setSource(USER);
           setSearchRA(getUserRA());
           setSearchDEC(getUserDEC());
       }
    }//GEN-LAST:event_UserRadioButtonActionPerformed

    private void ResolvedRadioButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ResolvedRadioButtonActionPerformed
       boolean status = ResolvedRadioButton.isSelected();
         if(status){
           setSource(RESOLVED);
           setSearchRA(myHTMIndexSearchClient.getRA());
           setSearchDEC(myHTMIndexSearchClient.getDEC());
        }
    }//GEN-LAST:event_ResolvedRadioButtonActionPerformed

    private void MinMagnitudeTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_MinMagnitudeTextFieldActionPerformed

        java.lang.String myValue = MinMagnitudeTextField.getText();
        try{
           double value = Double.parseDouble(myValue);
           setMinMagnitude(value);
           filterResults();
        }catch(Exception e){
           MinMagnitudeTextField.setText(Double.toString(DEFAULT_MIN_MAGNITUDE));
        }
    }//GEN-LAST:event_MinMagnitudeTextFieldActionPerformed

    private void MaxMagnitudeTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_MaxMagnitudeTextFieldActionPerformed
       java.lang.String myValue = MaxMagnitudeTextField.getText();
        try{
           double value = Double.parseDouble(myValue);
           setMaxMagnitude(value);
           filterResults();
        }catch(Exception e){
           MaxMagnitudeTextField.setText(Double.toString(DEFAULT_MAX_MAGNITUDE));
        }        // TODO add your handling code here:
    }//GEN-LAST:event_MaxMagnitudeTextFieldActionPerformed

    private void SearchBoxSizeTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SearchBoxSizeTextFieldActionPerformed
        java.lang.String myValue = SearchBoxSizeTextField.getText();
        try{
           double value = Double.parseDouble(myValue);
           setSearchBoxSize(value);
        }catch(Exception e){
           SearchBoxSizeTextField.setText(Double.toString(DEFAULT_SEARCH_BOX_SIZE));
        }        // TODO add your handling code here:
    }//GEN-LAST:event_SearchBoxSizeTextFieldActionPerformed

    private void UpdateToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UpdateToggleButtonActionPerformed
        // TODO add your handling code here:
                boolean selected = UpdateToggleButton.isSelected();
        if(myP200Component.isConnected()){
            if(selected){
                myP200Component.startPolling();
                UpdateToggleButton.setText("Updating");
            }
            if(!selected){
                myP200Component.stopPolling();
                UpdateToggleButton.setText("Update");
            }
        }
    }//GEN-LAST:event_UpdateToggleButtonActionPerformed

    private void SendToTelescopeButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SendToTelescopeButtonActionPerformed
        // TODO add your handling code here:
      double thisRA = myTycho2Reader.getSelectedRA();
      double thisDec = myTycho2Reader.getSelectedDEC();
      RA     currentRA   = new RA(thisRA);
      dec    currentDec  = new dec(thisDec);
//      int     currentTrackingRatesUnits = myTelescopeObject.getTrackingRatesUnits();
      AstroObject currentPosition = new AstroObject(myTycho2Reader.getSelectedObjectName(),currentRA,currentDec,2000.0,0,0,false,"");
      if(myP200Component.isConnected()){
         myP200Component.sentCoordinates(currentPosition);
      }
    }//GEN-LAST:event_SendToTelescopeButtonActionPerformed

    private void SpecTypeComboBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SpecTypeComboBoxActionPerformed
       java.lang.String spec = (java.lang.String)SpecTypeComboBox.getSelectedItem();
       setSpectralType(spec);
       filterResults();
    }//GEN-LAST:event_SpecTypeComboBoxActionPerformed

    private void LuminosityComboBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_LuminosityComboBoxActionPerformed
       java.lang.String lumin = (java.lang.String)LuminosityComboBox.getSelectedItem();
       setLuminosity(lumin);
       filterResults();
        // TODO add your handling code here:
    }//GEN-LAST:event_LuminosityComboBoxActionPerformed

    private void UserRATextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UserRATextFieldActionPerformed
        // TODO add your handling code here:
        java.lang.String myText = UserRATextField.getText();
        RA myRA = new RA(myText);
        setUserRA(myRA.degrees());

    }//GEN-LAST:event_UserRATextFieldActionPerformed

    private void UserDECTextFieldActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_UserDECTextFieldActionPerformed
         java.lang.String myText = UserDECTextField.getText();
        dec myDec = new dec(myText);
        setUserDEC(myDec.degrees());
        // TODO add your handling code here:
    }//GEN-LAST:event_UserDECTextFieldActionPerformed

private void RetrieveImageButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_RetrieveImageButtonActionPerformed
// TODO add your handling code here:
      double thisRA  = myTycho2Reader.getSelectedRA()*15.0;
      double thisDec = myTycho2Reader.getSelectedDEC();
//   if(myGuideStarDisplayFrame != null){
//      myGuideStarDisplayFrame.retrieveImage(thisRA, thisDec, myGuideStarDisplayFrame.image_size, myGuideStarDisplayFrame.image_size);  
//    }
//   if(myGuideStarDisplayFrame == null){
       if(myDS9ImageDisplayClient == null){
           myDS9ImageDisplayClient = new DS9ImageDisplayClient(this);
       }
       if(myDS9ImageDisplayClient != null){
          myDS9ImageDisplayClient.retrieveImage(thisRA, thisDec, image_size, image_size);
       }
//    }
}//GEN-LAST:event_RetrieveImageButtonActionPerformed

private void ObjectNameRetrieveImageButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ObjectNameRetrieveImageButtonActionPerformed
   double thisRA  = myHTMIndexSearchClient.getRA();
   double thisDec = myHTMIndexSearchClient.getDEC();
//   if(myGuideStarDisplayFrame != null){
//      myGuideStarDisplayFrame.retrieveImage(thisRA, thisDec, myGuideStarDisplayFrame.image_size, myGuideStarDisplayFrame.image_size); 
//    }
//   if(myGuideStarDisplayFrame == null){
       if(myDS9ImageDisplayClient == null){
           myDS9ImageDisplayClient = new DS9ImageDisplayClient(this);
           myDS9ImageDisplayClient.setDisplayDS9(true);
       }
       if(myDS9ImageDisplayClient != null){
          myDS9ImageDisplayClient.retrieveImage(thisRA, thisDec, image_size, image_size);
       }
//   }
}//GEN-LAST:event_ObjectNameRetrieveImageButtonActionPerformed

private void ImageSizeComboBoxActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ImageSizeComboBoxActionPerformed
          int selectedIndex = ImageSizeComboBox.getSelectedIndex();
        if(selectedIndex == 0){
            setImageSize(15);
        }
        if(selectedIndex == 1){
            setImageSize(30);
        }
        if(selectedIndex == 2){
            setImageSize(60);
        }
}//GEN-LAST:event_ImageSizeComboBoxActionPerformed

private void DS9ToggleButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_DS9ToggleButtonActionPerformed
    // TODO add your handling code here:
    boolean state  = DS9ToggleButton.isSelected();
    if(state){
        DS9ToggleButton.setIcon(ONImageIcon);
        if(myDS9ImageDisplayClient != null){
          myDS9ImageDisplayClient.setDisplayDS9(true);
        }
    }
    if(!state){
        DS9ToggleButton.setIcon(OFFImageIcon);
        if(myDS9ImageDisplayClient != null){
           myDS9ImageDisplayClient.setDisplayDS9(false);
        }
    }
}//GEN-LAST:event_DS9ToggleButtonActionPerformed
/*=============================================================================================
/     WindowListener Interface - changing the behavior of the close events
/=============================================================================================*/
    public void windowClosing(WindowEvent e) {
        displayMessage("WindowListener method called: windowClosing.");
 //       this.setVisible(false);
    }
    public void windowClosed(WindowEvent e) {
        //This will only be seen on standard output.
        displayMessage("WindowListener method called: windowClosed.");
        this.setVisible(false);
    }

    public void windowOpened(WindowEvent e) {
        displayMessage("WindowListener method called: windowOpened.");
    }

    public void windowIconified(WindowEvent e) {
        displayMessage("WindowListener method called: windowIconified.");
    }

    public void windowDeiconified(WindowEvent e) {
        displayMessage("WindowListener method called: windowDeiconified.");
    }

    public void windowActivated(WindowEvent e) {
        displayMessage("WindowListener method called: windowActivated.");
    }

    public void windowDeactivated(WindowEvent e) {
        displayMessage("WindowListener method called: windowDeactivated.");
    }
    void displayMessage(String msg) {
        System.out.println(msg);
    }
 /*=============================================================================================
/      Constructor: SharedListSelectionHandler
/=============================================================================================*/
    class SharedListSelectionHandler implements ListSelectionListener {
        public void valueChanged(ListSelectionEvent e) {
          Point2D.Double p;
          ListSelectionModel lsm = (ListSelectionModel)e.getSource();
           lsm.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
             boolean isAdjusting = lsm.getValueIsAdjusting();
            if(!isAdjusting){
            try{
             int     selectionView   = lsm.getLeadSelectionIndex();
             int     selectionModel  = TychoObjectsTable.convertRowIndexToModel(selectionView);
             tycho2star currentStar = (tycho2star)myTycho2Reader.getTycho2TableModel().getRecord(selectionModel);
             myTycho2Reader.setSelectedRA(currentStar.ra/15.0);
             myTycho2Reader.setSelectedDEC(currentStar.dec);
             myTycho2Reader.setSelectedObjectName(currentStar.tycho_name);
             }catch(Exception ex){
   //            System.out.println(ex.toString());
               lsm.setLeadSelectionIndex(0);
             }
 } // end of if !isAdjusting
         }
    }

/*=============================================================================================
/                          
/=============================================================================================*/
/*=============================================================================================
/    main(String args[])
/=============================================================================================*/
    /**
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new TychoCatalogFrame().setVisible(true);
            }
        });
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.ButtonGroup CoordinateSourceButtonGroup;
    private javax.swing.JTextField CurrentDecTextField;
    private javax.swing.JTextField CurrentRATextField;
    private javax.swing.JTextField DECTextField;
    private javax.swing.JToggleButton DS9ToggleButton;
    private javax.swing.JButton GetCoordinatesButton;
    private javax.swing.JComboBox ImageSizeComboBox;
    private javax.swing.JComboBox LuminosityComboBox;
    private javax.swing.JTextField MaxMagnitudeTextField;
    private javax.swing.JTextField MinMagnitudeTextField;
    private javax.swing.JRadioButton NEDRadioButton;
    private javax.swing.JButton ObjectNameRetrieveImageButton;
    private javax.swing.JTextField ObjectNameTextField;
    private javax.swing.JTextField RATextField;
    private javax.swing.ButtonGroup ResolutionSourceButtonGroup;
    private javax.swing.JRadioButton ResolvedRadioButton;
    private javax.swing.JButton RetrieveImageButton;
    private javax.swing.JRadioButton SIMBADRadioButton;
    private javax.swing.JTextField SearchBoxSizeTextField;
    private javax.swing.JButton SendToTelescopeButton;
    private javax.swing.JComboBox SpecTypeComboBox;
    private javax.swing.JLabel StatusMessage;
    private javax.swing.JRadioButton TelescopeRadioButton;
    private javax.swing.JTable TychoObjectsTable;
    private javax.swing.JButton UpdateButton;
    private javax.swing.JToggleButton UpdateToggleButton;
    private javax.swing.JTextField UserDECTextField;
    private javax.swing.JTextField UserRATextField;
    private javax.swing.JRadioButton UserRadioButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel12;
    private javax.swing.JLabel jLabel13;
    private javax.swing.JLabel jLabel14;
    private javax.swing.JLabel jLabel15;
    private javax.swing.JLabel jLabel16;
    private javax.swing.JLabel jLabel17;
    private javax.swing.JLabel jLabel18;
    private javax.swing.JLabel jLabel19;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel20;
    private javax.swing.JLabel jLabel21;
    private javax.swing.JLabel jLabel22;
    private javax.swing.JLabel jLabel23;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel10;
    private javax.swing.JPanel jPanel11;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanel6;
    private javax.swing.JPanel jPanel7;
    private javax.swing.JPanel jPanel8;
    private javax.swing.JPanel jPanel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTextField selectedDECTextField;
    private javax.swing.JTextField selectedRATextField;
    // End of variables declaration//GEN-END:variables

}
