/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.caltech.palomar.telescopes.telemetry;

import java.awt.Color;
import javax.swing.table.AbstractTableModel;
import java.sql.Timestamp;
import java.util.ArrayList;
import edu.caltech.palomar.telescopes.telemetry.TelemetryObject;

/**
 *
 * @author developer
 */
public class TelemetryTableModel extends AbstractTableModel{
   public ArrayList          telemetry_object_arraylist = new ArrayList(); 
   public int                NUM_COLUMNS     = 50;
   private int               columncount = 50;
   private int               rowcount    = 0;
   public java.lang.String[] columnNameArray = new java.lang.String[columncount];
    


   public TelemetryTableModel(){
      init();  
   }
   private void init(){
     columnNameArray[0]   = "ID";  
     columnNameArray[1]   = "TIMESTAMP_UTC";  
     columnNameArray[2]   = "UTCDATETIME";  
     columnNameArray[3]   = "LOCALDATETIME";  
     columnNameArray[4]   = "UTCSTART";  
     columnNameArray[5]   = "RA";  
     columnNameArray[6]   = "DEC";  
     columnNameArray[7]   = "HTM_7";  
     columnNameArray[8]   = "HTM_20";  
     columnNameArray[9]   = "AIRMASS";  
     columnNameArray[10]   = "AZIMUTH";  
     columnNameArray[11]   = "CASSANG";  
     columnNameArray[12]   = "DECOFF";  
     columnNameArray[13]   = "DECTRAC";  
     columnNameArray[14]   = "DOMEAZI";       
     columnNameArray[15]   = "DOMESHU";  
     columnNameArray[16]   = "EQUINOX";  
     columnNameArray[17]   = "HA";  
     columnNameArray[18]   = "INSTRMNT";  
     columnNameArray[19]   = "LST";  
     columnNameArray[20]   = "OBJECT";  
     columnNameArray[21]   = "RAOFF";  
     columnNameArray[22]   = "RATEDEC";  
     columnNameArray[23]   = "RATERA";  
     columnNameArray[24]   = "TELFOCUS";  
     columnNameArray[25]   = "TELID";  
     columnNameArray[26]   = "TMOTION";  
     columnNameArray[27]   = "WINDSCR";  
     columnNameArray[28]   = "ZA";  
     columnNameArray[29]   = "ALTITUDE";  
     columnNameArray[30]   = "ALTPARA";  
     columnNameArray[31]   = "BARYJD";  
     columnNameArray[32]   = "BARYVCOR";  
     columnNameArray[33]   = "CONSTEL";  
     columnNameArray[34]   = "JD";  
     columnNameArray[35]   = "MOONALT";  
     columnNameArray[36]   = "MOONANG";  
     columnNameArray[37]   = "MOONAZ";  
     columnNameArray[38]   = "MOONDEC";  
     columnNameArray[39]   = "MOONLIG";  
     columnNameArray[40]   = "MOONILM";  
     columnNameArray[41]   = "MOONPH";  
     columnNameArray[42]   = "MOONRA";  
     columnNameArray[43]   = "PARA";  
     columnNameArray[44]   = "PLANETW";  
     columnNameArray[45]   = "SUNALT";  
     columnNameArray[46]   = "SUNAZ";  
     columnNameArray[47]   = "SUNDEC";  
     columnNameArray[48]   = "SUNRA";  
     columnNameArray[49]   = "ZTWILIGH";  
   }
/*================================================================================================
/      setArrayList(ArrayList new_resultset)
/=================================================================================================*/
public void setArrayList(ArrayList new_resultset){
    telemetry_object_arraylist = new_resultset;
    fireTableDataChanged();
}
/*================================================================================================
/       addRecord(Centroid newRecord)
/=================================================================================================*/
 public void addRecord(TelemetryObject newRecord){
    telemetry_object_arraylist.add(newRecord);
    rowcount = rowcount + 1;
    fireTableDataChanged();
 }
/*================================================================================================
/       addRecord(Centroid newRecord)
/=================================================================================================*/
 public void removeRecord(int index){    
    telemetry_object_arraylist.remove(index);
    rowcount = rowcount - 1;
    fireTableDataChanged();
 }   
 /*================================================================================================
/         getRecord(int recordNumber)
/=================================================================================================*/
  public synchronized TelemetryObject getRecord(int recordNumber) {
     TelemetryObject myTelemetry =((TelemetryObject)(telemetry_object_arraylist.get(recordNumber)));
    return myTelemetry;
  }
 /*================================================================================================
/        getRecordNumber(AstroObject selectedObject)
/=================================================================================================*/
 public int getRecordNumber(TelemetryObject selectedObject){
     int recordNumber = 0;
     java.util.ListIterator li = telemetry_object_arraylist.listIterator();
     while(li.hasNext()){
        TelemetryObject currentObject = (TelemetryObject)(li.next());
        if(currentObject.equals(selectedObject)){
           recordNumber = li.previousIndex();
        }
     }
     return recordNumber;
 }
/*================================================================================================
/         clearTable()
/=================================================================================================*/
 public void clearTable(){
     telemetry_object_arraylist.clear();
     rowcount = 0;
     fireTableDataChanged();
 }
/*================================================================================================
/     setColumnName(int col)
/=================================================================================================*/
 public void setColumnName(int col,java.lang.String newColumnName){
     columnNameArray[col] = newColumnName;
 }
/*================================================================================================
/     getColumnName(int col)
/=================================================================================================*/
  public String getColumnName(int col) {
    return columnNameArray[col];
  }
/*================================================================================================
/         getColumnCount() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public int getColumnCount() {
      return columncount;
  }
/*================================================================================================
/         setColumnCount() -
/=================================================================================================*/
  public void setColumnCount(int newColumnCount){
      columncount = newColumnCount;
  } 
 /*================================================================================================
/         getRowCount() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public int getRowCount() {
        return rowcount;
  } 
/*================================================================================================
/       isCellEditable(int rowIndex, int vColIndex)
/=================================================================================================*/
  public boolean isCellEditable(int rowIndex, int vColIndex) {
    boolean editable = false;
    return editable;
}
/*================================================================================================
/        getColumnClass(int c)
/=================================================================================================*/
  public Class getColumnClass(int c) {
    java.lang.Class myClass = null;
    if(c == 0){
        myClass = Long.class; // ID
        return myClass;
     }
    if(c == 1){
        myClass = Timestamp.class; //TIMESTAMP_UTC 
        return myClass;
     }
    if(c == 2){
        myClass = String.class;  //UTCDATETIME
        return myClass;
     }
    if(c == 3){
        myClass = String.class;  //LOCALDATETIME 
        return myClass;
     }
    if(c == 4){
        myClass = String.class; //UTCSTART
        return myClass;
     }
     if(c == 5){
        myClass = String.class; //RA 
        return myClass;
     }
     if(c == 6){
        myClass =  String.class;  //DEC
        return myClass;
     }
     if(c == 7){
        myClass =  Long.class; // HTM_7
        return myClass;
     }
     if(c == 8){
        myClass =  Long.class; //HTM_20
        return myClass;
     }
     if(c == 9){
        myClass =  Float.class; // AIRMASS
        return myClass;
     }
     if(c == 10){
        myClass =  Float.class; // AZIMUTH
        return myClass;
     }
     if(c == 11){
        myClass =  Float.class;  // CASSANG
        return myClass;
     }
     if(c == 12){
        myClass =  Float.class; // DECOFF
        return myClass;
     }
     if(c == 13){
        myClass =  Float.class; //DECTRAC
        return myClass;
     }
     if(c == 14){
        myClass =  Float.class; // DOMEAZI
        return myClass;
     }
     if(c == 15){
        myClass =  String.class; // DOMESHU
        return myClass;
     }
     if(c == 16){
        myClass =  Float.class; // EQUINOX
        return myClass;
     }
     if(c == 17){
        myClass =  String.class; // HA
        return myClass;
     }
     if(c == 18){
        myClass =  String.class; // INSTRMNT
        return myClass;
     }
     if(c == 19){
        myClass =  Float.class;  // LST
        return myClass;
     }
     if(c == 20){
        myClass =  String.class; // OBJECT
        return myClass;
     }
     if(c == 21){
        myClass =  Float.class; // RAOFF 
        return myClass;
     }
     if(c == 22){
        myClass =  Float.class; // RATEDEC
        return myClass;
     }
     if(c == 23){
        myClass =  Float.class; // RATERA
        return myClass;
     }
     if(c == 24){
        myClass =  Float.class; // TELFOCUS
        return myClass;
     }
     if(c == 25){
        myClass =  String.class; // TELID
        return myClass;
     }
     if(c == 26){
        myClass =  String.class; // TMOTION
        return myClass;
     }
     if(c == 27){
        myClass =  String.class; // WINDSCR
        return myClass;
     }
     if(c == 28){
        myClass =  Float.class;  // ZA
        return myClass;
     }
     if(c == 29){
        myClass =  Float.class; //ALTITUDE
        return myClass;
     }
     if(c == 30){
        myClass =  Float.class; //ALTPARA
        return myClass;
     }
     if(c == 31){
        myClass =  Double.class; //BARYJD
        return myClass;
     }
     if(c == 32){
        myClass =  String.class; //BARYVCOR
        return myClass;
     }
     if(c == 33){
        myClass = String.class;  //CONSTEL
        return myClass;
     }
     if(c == 34){
        myClass =  Double.class; // JD
        return myClass;
     }
     if(c == 35){
        myClass =  Float.class; //MOONALT
        return myClass;
     }
     if(c == 36){
        myClass =  Float.class; //MOONANG
        return myClass;
     }
     if(c == 37){
        myClass =  Float.class; //MOONAZ
        return myClass;
     }
     if(c == 38){
        myClass =  String.class; //MOONDEC
        return myClass;
     }
     if(c == 39){
        myClass =  String.class; //MOONLIG
        return myClass;
     }
     if(c == 40){
        myClass =  String.class; //MOONILM
        return myClass;
     }
     if(c == 41){
        myClass =  String.class; //MOONPH
        return myClass;
     }
     if(c == 42){
        myClass =  String.class; //MOONRA
        return myClass;
     }
     if(c == 43){
        myClass =  Float.class; //PARA
        return myClass;
     }
     if(c == 44){
        myClass =  String.class; //PLANETW
        return myClass;
     }
     if(c == 45){
        myClass =  Float.class; //SUNALT
        return myClass;
     }
     if(c == 46){
        myClass =  Float.class; //SUNAZ
        return myClass;
     }
     if(c == 47){
        myClass =  String.class; //SUNDEC
        return myClass;
     }
     if(c == 48){
        myClass =  String.class; //SUNRA
        return myClass;
     }
     if(c == 49){
        myClass =  String.class; //ZTWILIGH
        return myClass;
     }     
    return myClass;
  }
/*================================================================================================
/         getValueAt() - Required Method for the Abstract Table Model Class
/=================================================================================================*/
  public Object getValueAt(int row, int col) {
    java.lang.Object returnObject = null;
    TelemetryObject selectedRow = (TelemetryObject)telemetry_object_arraylist.get(row);
    if(col == 0){
       returnObject = Long.valueOf(selectedRow.ID);
    }
    if(col == 1){
       returnObject = selectedRow.UTC_TIMESTAMP;
    }
    if(col == 2){
       returnObject = selectedRow.UTC_DATE_TIME;
    }
    if(col == 3){
       returnObject = selectedRow.LOCAL_DATE_TIME;
    }
    if(col == 4){
       returnObject = selectedRow.UTCSTART;
    }
    if(col == 5){
       returnObject = selectedRow.RA;
    }
    if(col == 6){
       returnObject = selectedRow.DEC;
    }
    if(col == 7){
       returnObject = Long.valueOf(selectedRow.HTM_LEVEL_7);
    }
    if(col == 8){
       returnObject = Long.valueOf(selectedRow.HTM_LEVEL_20);
    }
    if(col == 9){
       returnObject = Float.valueOf(selectedRow.AIRMASS);
    }
    if(col == 10){
       returnObject = Float.valueOf(selectedRow.AZIMUTH);
    }
    if(col == 11){
       returnObject = Float.valueOf(selectedRow.CASSANG);
    }
    if(col == 12){
       returnObject = Float.valueOf(selectedRow.DECOFF);
    }
    if(col == 13){
       returnObject = Float.valueOf(selectedRow.DECTRAC);
    }
    if(col == 14){
       returnObject = Float.valueOf(selectedRow.DOMEAZI);
    }
    if(col == 15){
       returnObject = new String(selectedRow.DOMESHU);
    }
    if(col == 16){
       returnObject = Float.valueOf(selectedRow.EQUINOX);
    }
    if(col == 17){
       returnObject = new String(selectedRow.HA);
    }
    if(col == 18){
       returnObject = new String(selectedRow.INSTRUMNT);
    }
    if(col == 19){
       returnObject = Float.valueOf(selectedRow.LST);
    }
    if(col == 20){
       returnObject = new String(selectedRow.OBJECT);
    }
    if(col == 21){
       returnObject = Float.valueOf(selectedRow.RAOFF);
    }
    if(col == 22){
       returnObject = Float.valueOf(selectedRow.RATEDEC);
    }
    if(col == 23){
       returnObject = Float.valueOf(selectedRow.RATERA);
    }
    if(col == 24){
       returnObject = Float.valueOf(selectedRow.TELFOCUS);
    }
    if(col == 25){
       returnObject = new String(selectedRow.TELID);
    }
    if(col == 26){
       returnObject = new String(selectedRow.TMOTION);
    }
    if(col == 27){
       returnObject = new String(selectedRow.WINDSCR);
    }
    if(col == 28){
       returnObject = Float.valueOf(selectedRow.ZA);
    }
    if(col == 29){
       returnObject = Float.valueOf(selectedRow.ALTITUDE);
    }
    if(col == 30){
       returnObject = Float.valueOf(selectedRow.ALTPARA);
    }
    if(col == 31){
       returnObject = Double.valueOf(selectedRow.BARYJD);
    }
    if(col == 32){
       returnObject = selectedRow.BARYVCOR;
    }
    if(col == 33){
       returnObject = selectedRow.CONSTEL;
    }
    if(col == 34){
       returnObject = Double.valueOf(selectedRow.JD);
    }
    if(col == 35){
       returnObject = Float.valueOf(selectedRow.MOONALT);
    }
    if(col == 36){
       returnObject = Float.valueOf(selectedRow.MOONANG);
    }
    if(col == 37){
       returnObject = Float.valueOf(selectedRow.MOONAZ);
    }
    if(col == 38){
       returnObject = selectedRow.MOONDEC;
    }
    if(col == 39){
       returnObject = selectedRow.MOONLIG;
    }
    if(col == 40){
       returnObject = selectedRow.MOONILM;
    }
    if(col == 41){
       returnObject = selectedRow.MOONPH;
    }
    if(col == 42){
       returnObject = selectedRow.MOONRA;
    }
    if(col == 43){
       returnObject = Float.valueOf(selectedRow.PARA);
    }
    if(col == 44){
       returnObject = selectedRow.PLANETW;
    }
    if(col == 45){
       returnObject = Float.valueOf(selectedRow.SUNALT);
    }
    if(col == 46){
       returnObject = Float.valueOf(selectedRow.SUNAZ);
    }
    if(col == 47){
       returnObject = selectedRow.SUNDEC;
    }
    if(col == 48){
       returnObject = selectedRow.SUNRA;
    }
    if(col == 49){
       returnObject = selectedRow.ZTWILIGH;
    }
    return returnObject;
  }  
/*================================================================================================
/     setValueAt(Object value, int row, int col)
/=================================================================================================*/
 public void setValueAt(Object value, int row, int col) {
   TelemetryObject selectedRow;
   if(telemetry_object_arraylist.size() >= row){
    selectedRow = (TelemetryObject)telemetry_object_arraylist.get(row);
    if(col == 0){
       selectedRow.ID = ((java.lang.Long)value).longValue();
    }
    if(col == 1){
       selectedRow.UTC_TIMESTAMP = ((java.sql.Timestamp)value);
    }
     if(col == 2){
       selectedRow.UTC_DATE_TIME = ((java.lang.String)value);
    }
    if(col == 3){
       selectedRow.LOCAL_DATE_TIME = ((java.lang.String)value);
    }
    if(col == 4){
       selectedRow.UTCSTART = ((java.lang.String)value);
    }
    if(col == 5){
       selectedRow.RA = ((java.lang.String)value);
    }
    if(col == 6){
       selectedRow.DEC = ((java.lang.String)value);
    }
    if(col == 7){
       selectedRow.HTM_LEVEL_7 = ((java.lang.Long)value).longValue();
    }
    if(col == 8){
       selectedRow.HTM_LEVEL_20 = ((java.lang.Long)value).longValue();
    }
    if(col == 9){
       selectedRow.AIRMASS = ((java.lang.Float)value).floatValue();
    }
    if(col == 10){
       selectedRow.AZIMUTH = ((java.lang.Float)value).floatValue();
    }
    if(col == 11){
       selectedRow.CASSANG = ((java.lang.Float)value).floatValue();
    }
    if(col == 12){
       selectedRow.DECOFF = ((java.lang.Float)value).floatValue();
    }
    if(col == 13){
       selectedRow.DECTRAC = ((java.lang.Float)value).floatValue();
    }
    if(col == 14){
       selectedRow.DOMEAZI = ((java.lang.Float)value).floatValue();
    }
    if(col == 15){
       selectedRow.DOMESHU = ((java.lang.String)value);
    }
    if(col == 16){
       selectedRow.EQUINOX = ((java.lang.Float)value).floatValue();
    }
    if(col == 17){
       selectedRow.HA = ((java.lang.String)value);
    }
    if(col == 18){
       selectedRow.INSTRUMNT = ((java.lang.String)value);
    }
    if(col == 19){
       selectedRow.LST = ((java.lang.Float)value).floatValue();
    }
    if(col == 20){
       selectedRow.OBJECT = ((java.lang.String)value);
    }
    if(col == 21){
       selectedRow.RAOFF = ((java.lang.Float)value).floatValue();
    }
    if(col == 22){
       selectedRow.RATEDEC = ((java.lang.Float)value).floatValue();
    }
    if(col == 23){
       selectedRow.RATERA = ((java.lang.Float)value).floatValue();
    }
    if(col == 24){
       selectedRow.TELFOCUS = ((java.lang.Float)value).floatValue();
    }
    if(col == 25){
       selectedRow.TELID = ((java.lang.String)value);
    }
    if(col == 26){
       selectedRow.TMOTION = ((java.lang.String)value);
    }
    if(col == 27){
       selectedRow.WINDSCR = ((java.lang.String)value);
    }
    if(col == 28){
       selectedRow.ZA = ((java.lang.Float)value).floatValue();
    }
    if(col == 29){
       selectedRow.ALTITUDE = ((java.lang.Float)value).floatValue();
    }
    if(col == 30){
       selectedRow.ALTPARA = ((java.lang.Float)value).floatValue();
    }
    if(col == 31){
       selectedRow.BARYJD = ((java.lang.Double)value).doubleValue();
    }
    if(col == 32){
       selectedRow.BARYVCOR = ((java.lang.String)value);
    }
    if(col == 33){
       selectedRow.CONSTEL = ((java.lang.String)value);
    }
    if(col == 34){
       selectedRow.JD = ((java.lang.Double)value).doubleValue();
    }
    if(col == 35){
       selectedRow.MOONALT = ((java.lang.Float)value).floatValue();
    }
    if(col == 36){
       selectedRow.MOONANG = ((java.lang.Float)value).floatValue();
    }
    if(col == 37){
       selectedRow.MOONAZ = ((java.lang.Float)value).floatValue();
    }
    if(col == 38){
       selectedRow.MOONDEC = ((java.lang.String)value);
    }
    if(col == 39){
       selectedRow.MOONLIG = ((java.lang.String)value);
    }
    if(col == 40){
       selectedRow.MOONILM = ((java.lang.String)value);
    }
    if(col == 41){
       selectedRow.MOONPH = ((java.lang.String)value);
    }
    if(col == 42){
       selectedRow.MOONRA = ((java.lang.String)value);
    }
    if(col == 43){
       selectedRow.PARA = ((java.lang.Float)value).floatValue();
    }
    if(col == 44){
       selectedRow.PLANETW = ((java.lang.String)value);
    }
    if(col == 45){
       selectedRow.SUNALT = ((java.lang.Float)value).floatValue();
    }
    if(col == 46){
       selectedRow.SUNAZ = ((java.lang.Float)value).floatValue();
    }
    if(col == 47){
       selectedRow.SUNDEC = ((java.lang.String)value);
    }
    if(col == 48){
       selectedRow.SUNRA = ((java.lang.String)value);
    }
    if(col == 49){
       selectedRow.ZTWILIGH = ((java.lang.String)value);
    }
    telemetry_object_arraylist.set(row,selectedRow);
   }
   fireTableCellUpdated(row, col);
 }  
}
