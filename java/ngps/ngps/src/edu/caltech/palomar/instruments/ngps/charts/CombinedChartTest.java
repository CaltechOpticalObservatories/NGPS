/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.charts;

import edu.caltech.palomar.instruments.ngps.dbms.NGPSdatabase;
import edu.caltech.palomar.instruments.ngps.object.Target;
import java.awt.Rectangle;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;
import java.text.DecimalFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.time.Instant;
import java.time.ZoneId;
import java.time.LocalDateTime;

import javax.swing.JPanel;
import org.jfree.chart.ChartMouseEvent;

import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.DateAxis;
import org.jfree.chart.axis.DateTickMarkPosition;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.date.MonthConstants;
import org.jfree.chart.entity.ChartEntity;
import org.jfree.chart.entity.EntityCollection;
import org.jfree.chart.entity.PlotEntity;
import org.jfree.chart.entity.XYItemEntity;
import org.jfree.chart.labels.StandardXYToolTipGenerator;
import org.jfree.chart.panel.CrosshairOverlay;
import org.jfree.chart.plot.Crosshair;
import org.jfree.chart.plot.DatasetRenderingOrder;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.StandardXYItemRenderer;
import org.jfree.chart.renderer.xy.XYBarRenderer;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.title.LegendTitle;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.gantt.TaskSeriesCollection;
import org.jfree.data.general.DatasetUtils;
import org.jfree.data.time.Day;
import org.jfree.data.time.Millisecond;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.data.xy.DefaultIntervalXYDataset;
import org.jfree.data.xy.IntervalXYDataset;
import org.jfree.data.xy.XYDataset;
import org.jfree.chart.axis.TickUnitSource;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import javax.swing.JFrame;
import org.jfree.chart.ChartMouseListener;
import java.awt.BasicStroke;
import java.awt.Font;
import java.awt.Window;
import org.jfree.chart.annotations.XYTextAnnotation;
import org.jfree.chart.annotations.XYLineAnnotation;
import org.jfree.chart.annotations.XYPointerAnnotation;
import org.jfree.chart.renderer.xy.StandardXYBarPainter;
import org.jfree.chart.ui.TextAnchor;
import java.sql.Timestamp;

/**
 * A demonstration application showing a time series line chart overlaid on a
 * bar chart.
 */
public class CombinedChartTest extends JFrame implements ChartMouseListener, MouseMotionListener, MouseListener{
   transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
   public java.lang.String              USERDIR           = System.getProperty("user.dir");
   public java.lang.String              SEP               = System.getProperty("file.separator");
   public TimeSeriesCollection          myOTMTimeSeriesCollection;
   public DefaultIntervalXYDataset      myDefaultIntervalXYDataset;
   public XYSeriesCollection            myXYSeriesCollection;
   private long                         start_first_exposure;
   private NGPSdatabase                 dbms;
   public  ChartPanel                   chart_panel;
   public  CrosshairOverlay             crosshair_overlay ;
   public  Crosshair                    domain_crosshair;
   public  Crosshair                    range_crosshair;
   private int                          selected_series;
   private static double                MIN_DISTANCE_FROM_POINT = 20;
   public java.lang.String              sunset;
   public java.lang.String              sunrise;
   public java.lang.String              reference;
   public Timestamp                     sunset_timestamp;
   public Timestamp                     sunrise_timestamp;
   public Timestamp                     reference_timestamp;
   public long                          sunset_milli;
   public long                          sunrise_milli;
   public long                          reference_milli;
   public long                          ref_milli;
   public double                        sunset_minutes;
   public double                        sunrise_minutes;
   public XYBarRenderer                 renderer1;
   private double                       frequency1 = 0.3;
   private double                       frequency2 = 0.3;
   private double                       frequency3 = 0.3;
   private double                       phase1     = 0;
   private double                       phase2     = 2;
   private double                       phase3     = 4;
   private int                          center     = 230;
   private int                          width      = 25;    
   private int                          previous_index;
   private String                       evening_twilight;
   private String                       morning_twilight;
   private Timestamp                    evening_twilight_timestamp;
   private Timestamp                    morning_twilight_timestamp;
   public long                          evening_twilight_milli;
   public long                          morning_twilight_milli;
   public double                        evening_twilight_minutes;
   public double                        morning_twilight_minutes;
   public Date                          sunset_time_hhmm;
   public Date                          sunrise_time_hhmm;
   public Date                          evening_twilight_time_hhmm;
   public Date                          morning_twilight_time_hhmm;
   public Date                          sunset_time_lower;
   public Date                          sunrise_time_upper;
   
     /**
     * Constructs a new demonstration application.
     *
     * @param title  the frame title.
     */
    public CombinedChartTest(String title,DefaultIntervalXYDataset newDefaultIntervalXYDataset,XYSeriesCollection newXYSeriesCollection,
                             String new_sunset,String new_sunrise,String new_reference,String new_evening_twilight,String new_morning_twilight,long new_ref_milli) {
        super(title);
        myDefaultIntervalXYDataset = newDefaultIntervalXYDataset;
        myXYSeriesCollection       = newXYSeriesCollection;
        this.sunset    = new_sunset;
        this.sunrise   = new_sunrise;
        this.reference = new_reference;
        this.ref_milli = new_ref_milli;
        this.evening_twilight = new_evening_twilight;
        this.morning_twilight = new_morning_twilight;
        reference_timestamp = Timestamp.valueOf(reference);
        sunset_timestamp    = Timestamp.valueOf(sunset);
        sunrise_timestamp   = Timestamp.valueOf(sunrise);
        evening_twilight_timestamp = Timestamp.valueOf(evening_twilight);
        morning_twilight_timestamp = Timestamp.valueOf(morning_twilight);
        sunset_milli        = sunset_timestamp.getTime();
        sunrise_milli       = sunrise_timestamp.getTime();
        reference_milli     = reference_timestamp.getTime();
        morning_twilight_milli = morning_twilight_timestamp.getTime();
        evening_twilight_milli = evening_twilight_timestamp.getTime();
        
        long delta = reference_milli  - ref_milli;
        System.out.println("DELTA = "+delta+" Calculated Reference = "+reference_milli+" Original Reference = "+ref_milli);
        
//        sunset_minutes = (double)((sunset_milli - reference_milli)/(1000.0*60))-(24*60);
//        sunrise_minutes = (double)((sunrise_milli - reference_milli)/(1000.0*60))-(24*60);
        sunset_minutes = (double)((sunset_milli - reference_milli)/(1000.0*60));
        sunrise_minutes = (double)((sunrise_milli - reference_milli)/(1000.0*60));
        evening_twilight_minutes = (double)((evening_twilight_milli - reference_milli)/(1000.0*60));
        morning_twilight_minutes = (double)((morning_twilight_milli - reference_milli)/(1000.0*60));
        System.out.println("Sunset Minutes = "+sunset_minutes+" Sunrise Minutes = "+sunrise_minutes);
        System.out.println("Evening Twilight Minutes = "+evening_twilight_minutes+" Morning Twilight Minutes = "+morning_twilight_minutes);
        
        sunset_time_lower = new Date((long)(sunset_milli - 600000));
        sunrise_time_upper = new Date((long)(sunrise_milli + 600000));
//        initializeDBMS();
        chart_panel = createDemoPanel();
        chart_panel.addOverlay(crosshair_overlay);
        chart_panel.setPreferredSize(new java.awt.Dimension(2000, 1000));
        chart_panel.addChartMouseListener(this);
        chart_panel.setMouseWheelEnabled(true);
        setContentPane(chart_panel);
        setSize(2000, 600);
        setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
//        chartPanel.setPreferredSize(new java.awt.Dimension(500, 300));
    }
   public CombinedChartTest(String title) {
        super(title);
        initializeDBMS();
        chart_panel = createDemoPanel();
        chart_panel.addOverlay(crosshair_overlay);
        chart_panel.setPreferredSize(new java.awt.Dimension(2000, 1000));
        chart_panel.addChartMouseListener(this);
        chart_panel.setMouseWheelEnabled(true);
        setContentPane(chart_panel);
        setSize(2000, 600);
//        chartPanel.setPreferredSize(new java.awt.Dimension(500, 300));
    }
/*================================================================================================
/      setSpectrumState(int newSpectrumState)
/=================================================================================================*/
public void setDBMS(NGPSdatabase new_dbms){
   dbms = new_dbms; 
}   
/*================================================================================================
/      setSpectrumState(int newSpectrumState)
/=================================================================================================*/
  public void setSelectedSeries(int new_selected_series) {
    int  old_selected_series = this.selected_series;
    this.selected_series = new_selected_series;
    propertyChangeListeners.firePropertyChange("selected_series", Integer.valueOf(old_selected_series), Integer.valueOf(new_selected_series));
  }
  public int getSelectedSeries() {
    return selected_series;
  } 
  /*================================================================================================
/      setSpectrumState(int newSpectrumState)
/=================================================================================================*/
  public void setPreviousSeries(int new_previous_index) {
    int  old_previous_index = this.previous_index;
    this.previous_index = new_previous_index;
    propertyChangeListeners.firePropertyChange("previous_index", Integer.valueOf(old_previous_index), Integer.valueOf(new_previous_index));
  }
  public int getPreviousSeries() {
    return previous_index;
  } 
 /*================================================================================================
/      add and remove Property Change Listeners
/=================================================================================================*/
  public synchronized void removePropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.removePropertyChangeListener(l);
  }
  public synchronized void addPropertyChangeListener(PropertyChangeListener l) {
    propertyChangeListeners.addPropertyChangeListener(l);
  }
  
    /**
     * Creates an overlaid chart.
     *
     * @return The chart.
     */
    private  JFreeChart createChart() {

        IntervalXYDataset data1 = myDefaultIntervalXYDataset;
        XYDataset         data2 = myXYSeriesCollection;
        renderer1               = new XYBarRenderer();
        renderer1.setUseYInterval(true);
        renderer1.setDrawBarOutline(false);
        renderer1.setShadowVisible(false);
        renderer1.setDefaultShadowsVisible(false);
        renderer1.setBarPainter(new StandardXYBarPainter());

        StandardXYToolTipGenerator tooltipGenerator = new StandardXYToolTipGenerator(){   
             public String generateToolTip(IntervalXYDataset dataset, int series, int item){
             return "Series " +dataset.getSeriesKey(series) + " Item: " + item + " Value: " + dataset.getXValue(series, item) + ";"+ dataset.getYValue(series, item);
         }
         };  
        renderer1.setDefaultToolTipGenerator(tooltipGenerator);
        
//        renderer1. setDefaultStroke(new BasicStroke(1.0f));
//        renderer1.setAutoPopulateSeriesStroke(false);
        
//      NumberAxis domainAxis = new NumberAxis("Time (minutes)");
        DateAxis domainAxis = new DateAxis("UTC Time");
        domainAxis.setDateFormatOverride(new SimpleDateFormat("HH:mm"));
        domainAxis.setVerticalTickLabels(true);
        int series_count_bars = myDefaultIntervalXYDataset.getSeriesCount();
        int series_count_xy   = myXYSeriesCollection.getSeriesCount();
 //       domainAxis.setRange(myDefaultIntervalXYDataset.getStartXValue(0,0),myDefaultIntervalXYDataset.getEndXValue(series_count-1,0));
 //       TickUnitSource units = NumberAxis.createIntegerTickUnits();
 //       domainAxis
 //       domainAxis.setTickMarkPosition(DateTickMarkPosition.MIDDLE);
        NumberAxis rangeAxis = new NumberAxis("Airmass");
        rangeAxis.setRange(1.0, 3.0);

        // add a second dataset and renderer...
        XYPlot plot = new XYPlot(data1, domainAxis, rangeAxis, renderer1);
        domainAxis.setAutoRange(false);
        double upper_bound = domainAxis.getRange().getUpperBound();
        domainAxis.setRange(sunset_time_lower, sunrise_time_upper);

        StandardXYItemRenderer renderer2 = new StandardXYItemRenderer();
        StandardXYToolTipGenerator xy_tooltip =  new StandardXYToolTipGenerator(StandardXYToolTipGenerator.DEFAULT_TOOL_TIP_FORMAT,new SimpleDateFormat("d-MMM-yyyy hh:mm:ss.s"), new DecimalFormat("0.00"));
        renderer2.setDefaultToolTipGenerator(tooltipGenerator);
        renderer2.setBaseShapesVisible(true);
        renderer2. setDefaultStroke(new BasicStroke(3.0f));
        renderer2.setAutoPopulateSeriesStroke(false);


        plot.setDataset(1, data2);
        plot.setRenderer(1, renderer2);
        int dataset_count = plot.getDatasetCount();
        double frequency1 = 0.3;
        double frequency2 = 0.3;
        double frequency3 = 0.3;
        double phase1     = 0;
        double phase2     = 2;
        double phase3     = 4;
        int center        = 230;
        int width         = 25;    
        int center_dark   = 128;
        int width_dark    = 127;
        int center_darker   = 100;
        int width_darker    = 99;
        double rotation_radians = 90.0*(Math.PI/180.0);
        renderer1.setDrawBarOutline(true);
        Font font = new Font("Ariel", Font.BOLD, 15);
        for(int j=0;j<series_count_bars;j++){
            java.awt.Color current_color = makeColorGradient(j,frequency1,frequency2,frequency3,phase1,phase2,phase3,center,width);
            java.awt.Color current_color_dark = makeColorGradient(j,frequency1,frequency2,frequency3,phase1,phase2,phase3,center_dark,width_dark);
            java.awt.Color current_color_darker = makeColorGradient(j,frequency1,frequency2,frequency3,phase1,phase2,phase3,center_darker,width_darker);
            renderer1.setSeriesPaint(j, current_color);
            renderer2.setSeriesPaint(j, current_color_dark);  
            java.lang.String key = (myDefaultIntervalXYDataset.getSeriesKey(j)).toString();
            double x1 = (myDefaultIntervalXYDataset.getStartXValue(j, 0));
            double x2 = (myDefaultIntervalXYDataset.getEndXValue(j, 0));
            double x = (x1+x2)/2;
            XYTextAnnotation annotation = annotation = new XYTextAnnotation(key, x, 2.0);
            annotation.setFont(font);
            // annotation.setPaint(current_color_darker);
            annotation.setPaint(java.awt.Color.BLACK);
            annotation.setTextAnchor(TextAnchor.CENTER);
            annotation.setRotationAnchor(TextAnchor.CENTER);
            annotation.setRotationAngle(rotation_radians);
            plot.addAnnotation(annotation);
            renderer1.setSeriesOutlinePaint(j,current_color_dark);
        }        
        renderer1.setDefaultOutlineStroke(new BasicStroke(0.5f));
        BasicStroke dotted_line = new BasicStroke( 2.0f, BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND,1.0f, new float[] {6.0f, 6.0f}, 0.0f);
        XYLineAnnotation reference_time = new XYLineAnnotation(reference_milli, 1.0, reference_milli, 3.0, new BasicStroke(3.0f), java.awt.Color.blue);
        XYLineAnnotation sunset_time    = new XYLineAnnotation(sunset_milli, 1.0,sunset_milli, 3.0, new BasicStroke(3.0f), java.awt.Color.red);
        XYLineAnnotation sunrise_time   = new XYLineAnnotation(sunrise_milli, 1.0,sunrise_milli, 3.0, new BasicStroke(3.0f), java.awt.Color.red);
        XYLineAnnotation evening_twilight_time   = new XYLineAnnotation(evening_twilight_milli, 1.0,evening_twilight_milli, 3.0, dotted_line, java.awt.Color.BLUE);
        XYLineAnnotation morning_twilight_time   = new XYLineAnnotation(morning_twilight_milli, 1.0,morning_twilight_milli, 3.0, dotted_line, java.awt.Color.BLUE);
       
//        XYPointerAnnotation sunset_pointer = new XYPointerAnnotation( "Sunset = "+sunset, sunset_minutes, 2.5, 3.0 * Math.PI / 4.0);
        XYPointerAnnotation sunset_pointer = new XYPointerAnnotation( "Sunset", sunset_milli, 2.5, 3.0 * Math.PI / 4.0);
         sunset_pointer.setTipRadius(10.0); 
         sunset_pointer.setBaseRadius(35.0);
         sunset_pointer.setFont(new Font("Ariel", Font.BOLD, 14)); sunset_pointer.setPaint(java.awt.Color.red);
         sunset_pointer.setTextAnchor(TextAnchor.CENTER_RIGHT);          
        XYPointerAnnotation sunrise_pointer = new XYPointerAnnotation( "Sunrise", sunrise_milli, 2.5, 3.0 * Math.PI / 4.0);
         sunrise_pointer.setTipRadius(10.0);
         sunrise_pointer.setBaseRadius(35.0);
         sunrise_pointer.setFont(new Font("Ariel", Font.BOLD, 14));
         sunrise_pointer.setPaint(java.awt.Color.green);
         sunrise_pointer.setTextAnchor(TextAnchor.HALF_ASCENT_RIGHT); 

        XYPointerAnnotation reference_pointer = new XYPointerAnnotation( "Start of Observations", reference_milli, 2.5, 3.0 * Math.PI / 4.0);
         reference_pointer.setTipRadius(10.0);
         reference_pointer.setBaseRadius(35.0);
         reference_pointer.setFont(new Font("Ariel", Font.BOLD, 14)); 
         reference_pointer.setPaint(java.awt.Color.blue);
         reference_pointer.setTextAnchor(TextAnchor.HALF_ASCENT_RIGHT); 
         
       XYTextAnnotation sunset_annotation = new XYTextAnnotation("Sunset = "+sunset,(sunset_milli)+(15*60000) , 2.0);
            sunset_annotation.setFont(font);
            sunset_annotation.setPaint(java.awt.Color.black);
            sunset_annotation.setTextAnchor(TextAnchor.CENTER);
            sunset_annotation.setRotationAnchor(TextAnchor.CENTER);
            sunset_annotation.setRotationAngle(rotation_radians);         
        plot.addAnnotation(sunset_annotation); 
     /*  XYTextAnnotation reference_annotation = new XYTextAnnotation("Observation start time = "+reference,-10 , 2.0);
            reference_annotation.setFont(font);
            reference_annotation.setPaint(java.awt.Color.black);
            reference_annotation.setTextAnchor(TextAnchor.CENTER);
            reference_annotation.setRotationAnchor(TextAnchor.CENTER);
            reference_annotation.setRotationAngle(rotation_radians);         
        plot.addAnnotation(reference_annotation);  */
       XYTextAnnotation sunrise_annotation = new XYTextAnnotation("Sunrise = "+sunrise,(sunrise_milli)-(10*60000), 2.0);
            sunrise_annotation.setFont(font);
            sunrise_annotation.setPaint(java.awt.Color.black);
            sunrise_annotation.setTextAnchor(TextAnchor.CENTER);
            sunrise_annotation.setRotationAnchor(TextAnchor.CENTER);
            sunrise_annotation.setRotationAngle(rotation_radians);         
        plot.addAnnotation(sunrise_annotation); 
 
       XYTextAnnotation evening_twilight_annotation = new XYTextAnnotation("Evening Twilight = "+evening_twilight,(evening_twilight_milli)-(10.0*60000), 2.0);
            evening_twilight_annotation.setFont(font);
            evening_twilight_annotation.setPaint(java.awt.Color.black);
            evening_twilight_annotation.setTextAnchor(TextAnchor.CENTER);
            evening_twilight_annotation.setRotationAnchor(TextAnchor.CENTER);
            evening_twilight_annotation.setRotationAngle(rotation_radians);         
        plot.addAnnotation(evening_twilight_annotation); 
        
       XYTextAnnotation morning_twilight_annotation = new XYTextAnnotation("Morning Twilight = "+morning_twilight,(morning_twilight_milli)+(10.0*60000), 2.0);
            morning_twilight_annotation.setFont(font);
            morning_twilight_annotation.setPaint(java.awt.Color.black);
            morning_twilight_annotation.setTextAnchor(TextAnchor.CENTER);
            morning_twilight_annotation.setRotationAnchor(TextAnchor.CENTER);
            morning_twilight_annotation.setRotationAngle(rotation_radians);         
        plot.addAnnotation(morning_twilight_annotation); 
        
//         plot.addAnnotation(sunset_pointer);
//         plot.addAnnotation(sunrise_pointer);
//         plot.addAnnotation(reference_pointer);
//        XYLineAnnotation morning_twilight = new XYLineAnnotation(1.0, 2.0, 3.0, 4.0, new BasicStroke(1.5f), Color.red);
        plot.addAnnotation(reference_time);
        plot.addAnnotation(sunset_time);
        plot.addAnnotation(sunrise_time);
        plot.addAnnotation(evening_twilight_time);
        plot.addAnnotation(morning_twilight_time);

        plot.setDatasetRenderingOrder(DatasetRenderingOrder.FORWARD);
        plot.setDomainPannable(true);
        plot.setRangePannable(true);
       
        crosshair_overlay = new CrosshairOverlay();
        domain_crosshair  = new Crosshair();
        range_crosshair   = new Crosshair();
        domain_crosshair.setVisible(true);
        range_crosshair.setVisible(true);
        crosshair_overlay.addDomainCrosshair(domain_crosshair);
        crosshair_overlay.addRangeCrosshair(range_crosshair);

        // return a new chart containing the overlaid plot...
        JFreeChart chart = new JFreeChart("Predicted Observation Timeline - "+reference, JFreeChart.DEFAULT_TITLE_FONT, plot, false);
        chart.getXYPlot().getRangeAxis().setInverted(true);
        setSelectedSeries(1);       
 //       LegendTitle legend = chart.getLegend();
 //       legend.setPosition(RectangleEdge.LEFT);     
//        ChartUtils.applyCurrentTheme(chart);
        return chart;
    }
    
    public java.awt.Color  makeColorGradient(int i,double frequency1,double frequency2,double frequency3,double phase1,double phase2,double phase3,int center,int width){
       int red = (int)(Math.sin(frequency1*i + phase1) * width + center);
       int grn = (int)(Math.sin(frequency2*i + phase2) * width + center);
       int blu = (int)(Math.sin(frequency3*i + phase3) * width + center);
       java.awt.Color current_color = new java.awt.Color(red,grn,blu);   
     return current_color;
  }
    public void printMouseState(ChartMouseEvent me){
       System.out.println("CLICK COUNT = "+me.getTrigger().getClickCount());
       System.out.println("Parameter String = "+me.getTrigger().paramString());
       System.out.println("Buttons = "+me.getTrigger().getButton());
       System.out.println("Parameter String = "+me.getTrigger().getLocationOnScreen());
       System.out.println("X = "+me.getTrigger().getX());
       System.out.println("Y = "+me.getTrigger().getY());
       System.out.println("X on screen = "+me.getTrigger().getXOnScreen());
       System.out.println("Y on screen = "+me.getTrigger().getYOnScreen());
       System.out.println("Modifiers = "+me.getTrigger().getModifiersEx());         
    }
    
    
    public void chartMouseClicked(ChartMouseEvent chartMouseEvent) {
       domain_crosshair.setValue(chartMouseEvent.getTrigger().getX());
       range_crosshair.setValue(chartMouseEvent.getTrigger().getY());
//       System.out.println("Y = "+chartMouseEvent.getTrigger().translatePoint(WIDTH, WIDTH)); 
   			ChartEntity entity = chartMouseEvent.getEntity();
    			if (entity instanceof PlotEntity){
    			}
    			if (entity instanceof XYItemEntity){
    				// User clicked on an XYDataset
    				XYItemEntity xyEntity = (XYItemEntity) entity;
    				int seriesIndex = xyEntity.getSeriesIndex();
                                        double frequency1 = 0.3;
                                int previous_index = selected_series;
                                setPreviousSeries(previous_index);
//                                java.awt.Color current_color = makeColorGradient(previous_index,frequency1,frequency2,frequency3,phase1,phase2,phase3,center,width);                                                           
                                setSelectedSeries(seriesIndex);
                                System.out.println("SET INDEX = "+selected_series+" Color = "+java.awt.Color.YELLOW.toString()); 
                                updateChartColors();
//                                renderer1.setSeriesPaint(previous_index,current_color);                                
 /*                               renderer1.setSeriesPaint(selected_series, java.awt.Color.YELLOW);
    				// Here I am setting the clicked series to red and increasing the line thickness.
                                int series_count_bars = myDefaultIntervalXYDataset.getSeriesCount();
                                for(int j=0;j<series_count_bars;j++){   
                                    if(j != (selected_series)){
                                      java.awt.Color calculated_color;
                                      Target current_target = dbms.myTargetDBMSTableModel.getRecord(j);
                                      java.lang.String current_state = current_target.getSTATE();
                                      if(current_state.matches("completed")){
                                          calculated_color = java.awt.Color.GRAY;
                                      }
                                      else if(current_state.matches("exposing")){
                                          calculated_color = java.awt.Color.GREEN;
                                      }
                                      else if(current_state.matches("active")){
                                          calculated_color = java.awt.Color.BLUE;
                                      }                                      
                                      else {
                                          calculated_color = makeColorGradient(j,frequency1,frequency2,frequency3,phase1,phase2,phase3,center,width);
                                        System.out.println("SET INDEX = "+previous_index+" Color = "+calculated_color.toString());                                        
                                      }
                                      renderer1.setSeriesPaint(j, calculated_color);
                                    }
                                }
*/
//    				chart_panel.getChart().getXYPlot().getRenderer().setSeriesPaint(seriesIndex, Color.BLUE);
//    				chart_panel.getChart().getXYPlot().getRenderer().setSeriesStroke(seriesIndex, new BasicStroke(2.0f));
    			}       
    }
public void updateChartColors(){
        int previous_index = getPreviousSeries();
        int center_dark   = 128;
        int width_dark    = 127;
          renderer1.setDrawBarOutline(true);
//        renderer1.setDefaultOutlinePaint(java.awt.Color.BLACK, true);
//        renderer1.setDefaultOutlineStroke(new BasicStroke(1.0f));
        // Here I am setting the clicked series to red and increasing the line thickness.
        int series_count_bars = myDefaultIntervalXYDataset.getSeriesCount();
        for(int j=0;j<series_count_bars;j++){  
            if(j != (selected_series)){
              java.awt.Color calculated_color;
              if(dbms.myTargetDBMSTableModel.getRowCount() >= j){
              Target current_target = dbms.myTargetDBMSTableModel.getRecord(j);
              java.lang.String current_state = current_target.getSTATE();
              if(current_state.matches("completed")){
                  calculated_color = java.awt.Color.GRAY;
                  renderer1.setSeriesOutlinePaint(j, java.awt.Color.gray);
                  renderer1.setSeriesOutlineStroke(j,new BasicStroke(0.5f));
              }
              else if(current_state.matches("exposing")){
                  calculated_color = java.awt.Color.GREEN;
                  renderer1.setSeriesOutlinePaint(j, java.awt.Color.BLACK);
                  renderer1.setSeriesOutlineStroke(j,new BasicStroke(2.5f));
              }
              else if(current_state.matches("active")){
                  calculated_color = java.awt.Color.BLUE;
                  renderer1.setSeriesOutlinePaint(j, java.awt.Color.BLACK);
                  renderer1.setSeriesOutlineStroke(j,new BasicStroke(2.5f));
              }                                      
              else{
                  calculated_color = makeColorGradient(j,frequency1,frequency2,frequency3,phase1,phase2,phase3,center,width);
               // System.out.println("SET INDEX = "+previous_index+" Color = "+calculated_color.toString());                                        
                 java.awt.Color current_color_dark = makeColorGradient(j,frequency1,frequency2,frequency3,phase1,phase2,phase3,center_dark,width_dark);
                  renderer1.setSeriesOutlinePaint(j, current_color_dark);
                  renderer1.setSeriesOutlineStroke(j,new BasicStroke(0.5f));
              }
              renderer1.setSeriesPaint(j, calculated_color);                               
              }
            }   
            if(j == selected_series){
                renderer1.setSeriesPaint(selected_series, java.awt.Color.YELLOW);
                renderer1.setSeriesOutlinePaint(selected_series, java.awt.Color.BLACK);
                renderer1.setSeriesOutlineStroke(selected_series,new BasicStroke(2.5f));
            }

        } 
}       
    public void chartMouseMoved(ChartMouseEvent chartMouseEvent) {
            Rectangle2D dataArea = this.chart_panel.getScreenDataArea();
            JFreeChart chart = chartMouseEvent.getChart();
            XYPlot plot = (XYPlot) chart.getPlot();
            ValueAxis xAxis = plot.getDomainAxis();
//            ValueAxis yAxis = plot.getRangeAxis();
            
            double x = xAxis.java2DToValue(chartMouseEvent.getTrigger().getX(), dataArea, RectangleEdge.BOTTOM);
//            double y = yAxis.java2DToValue(chartMouseEvent.getTrigger().getY(), dataArea, RectangleEdge.BOTTOM);
            // make the crosshairs disappear if the mouse is out of range
            if (!xAxis.getRange().contains(x)) { 
                x = Double.NaN;                  
            }
            double y = DatasetUtils.findYValue(plot.getDataset(), 0, x);
            int[] index = DatasetUtils.findItemIndicesForX(plot.getDataset(), 0, x);
            
            domain_crosshair.setValue(x);
            range_crosshair.setValue(y);        
    }  
    	public void mousePressed(MouseEvent e) {
		Point2D p2d = chart_panel.translateJava2DToScreen(e.getPoint());
		
		p2d = getNearestPointWithEntity(p2d);
		ChartEntity ce = chart_panel.getEntityForPoint((int)p2d.getX(), (int)p2d.getY());
	}

	private Point2D getNearestPointWithEntity(Point2D p) {
		double minDistance = MIN_DISTANCE_FROM_POINT;
		
		Point2D point = null;
		EntityCollection entities = chart_panel.getChartRenderingInfo().getEntityCollection();
		for (Iterator iter = entities.iterator(); iter.hasNext();) {
			ChartEntity element = (ChartEntity) iter.next();
			
			Rectangle rect = element.getArea().getBounds();
			Point2D centerPoint = new Point2D.Double(rect.getCenterX(), rect.getCenterY());
			
			if (p.distance(centerPoint) < minDistance) {
				minDistance = p.distance(centerPoint);
				point = centerPoint;
			}                       
		}
		
		return point;
	}

	public void mouseDragged(MouseEvent e) {
        }

	public void mouseReleased(MouseEvent e) {
		Point2D p2d = chart_panel.translateJava2DToScreen(e.getPoint());
	}
	
	public void mouseMoved(MouseEvent e) {
		Point2D p2d = getNearestPointWithEntity(e.getPoint());
	}
	public void mouseClicked(MouseEvent e) {}

	public void mouseEntered(MouseEvent e) {}

	public void mouseExited(MouseEvent e) {}
/*=============================================================================================
/   constructTask(Target current)
/=============================================================================================*/
public void addXIntervalXYDataset(Target current){
    long start_time = (current.otm.getOTMstart().getTime());
    long end_time   = (current.otm.getOTMend().getTime());
//    long start_time = (current.otm.getOTMstart().getTime());
//    long end_time   = (current.otm.getOTMend().getTime());
    long center_of_observation = (end_time - start_time)/2;
    double airmass_start = current.otm.getOTMAirmass_start();
    double airmass_end   = current.otm.getOTMAirmass_end();
    double mean_airmass  = (airmass_start + airmass_end)/2;
    double[] x = {center_of_observation};
    double[] startx = {(double)start_time};
    double[] endx = {(double)end_time};
    double[] y = {mean_airmass};
    double[] starty = {0.00};
    double[] endy = {5.0};
    double[][] data = new double[][] {x, startx, endx, y, starty, endy};
    myDefaultIntervalXYDataset.addSeries(current.getName(), data);   
}
public void constructXYSeries(int sequence_number,Target current){
      XYSeries series = new XYSeries(current.getName()+"-"+sequence_number);
      long start_time = (current.otm.getOTMstart().getTime());
      long end_time   = (current.otm.getOTMend().getTime());      
      series.add(start_time,current.otm.getOTMAirmass_start());
      series.add(end_time,current.otm.getOTMAirmass_end());
      myXYSeriesCollection.addSeries(series);
}
/*================================================================================================
/     constructTimeSeries(Target current)
/=================================================================================================*/
public TimeSeries constructTimeSeries(Target current){
    TimeSeries current_time_series = new TimeSeries(current.getName());
    Millisecond start = new Millisecond(new Date((current.otm.getOTMstart().getTime())));
    Millisecond end   = new Millisecond(new Date((current.otm.getOTMend().getTime())));
    current_time_series.add(start,current.otm.getOTMAirmass_start(), true);
    current_time_series.add(end,current.otm.getOTMAirmass_end(), true);
    return current_time_series;
}
public void addTimeSeries(TimeSeries current_series){
    myOTMTimeSeriesCollection.addSeries(current_series);
}
/*=============================================================================================
/   
/     CREATION OF DBMS TABLE FOR TESTING - REMOVE FOR DEPLOYMENT
/
/=============================================================================================*/
/*================================================================================================
/     string_to_timestamp(java.lang.String current_datetime)
/=================================================================================================*/
public java.sql.Timestamp string_to_timestamp(java.lang.String current_datetime){
java.sql.Timestamp timestamp =new java.sql.Timestamp(System.currentTimeMillis());
 try {
    java.text.SimpleDateFormat dateFormat = new java.text.SimpleDateFormat("yyyy-MM-dd'T'hh:mm:ss.SSS");
    java.util.Date parsedDate = dateFormat.parse(current_datetime);
    timestamp = new java.sql.Timestamp(parsedDate.getTime());
} catch(Exception e) { //this generic but you can control another types of exception
    // look the origin of excption 
}
return timestamp;
}
public java.lang.String timestamp_to_string(java.sql.Timestamp current){
//    java.sql.Timestamp timestamp = java.sql.Timestamp.valueOf("2018-12-12 01:02:03.123456789");
    java.time.format.DateTimeFormatter formatter = java.time.format.DateTimeFormatter.ISO_LOCAL_DATE_TIME;
    String timestampAsString = formatter.format(current.toLocalDateTime());
//    assertEquals("2018-12-12T01:02:03.123456789", timestampAsString);
return timestampAsString;
}   
public void setSlewgoStart_FirstExposure(long new_start_first_exposure){
    start_first_exposure = new_start_first_exposure;
}

/*=============================================================================================
/   parseText()
/=============================================================================================*/
 public void initializeDBMS(){
     dbms = new NGPSdatabase();
     dbms.initializeDBMS();
     if(dbms.isConnected()){
         int set_id = 15;
         dbms.queryObservations(set_id);
         readOTMoutput();
    }
 }
/*=============================================================================================
/   parseText()
/=============================================================================================*/
public ArrayList parseHeaderLine(java.lang.String header_line){
    java.util.StringTokenizer st = new java.util.StringTokenizer(header_line,",");
    int token_count = st.countTokens();
    ArrayList header_list = new ArrayList();
    for(int i=0;i<token_count;i++){
        java.lang.String token = st.nextToken();
        header_list.add(token);
//        System.out.println(token.toUpperCase());
//        token = token.toUpperCase();
    }
  return header_list;
}  
/*=============================================================================================
/   parseObservationRecord(java.util.ArrayList current_list,java.lang.String current)
/=============================================================================================*/
 public Target parseOTMRecord(Target current_target,java.util.ArrayList current_list,java.lang.String current){
     StringTokenizer st = new StringTokenizer(current,",");
     java.lang.String[] token_array = current.split(",");
     int count_header = current_list.size();
     int count_current = st.countTokens();
     System.out.println("Counted Tokens Header = "+count_header + " Counted Tokens String = "+count_current);
     for(int i=0;i<count_header;i++){
        java.lang.String field = ((java.lang.String)current_list.get(i)).trim();
        java.lang.String current_value = token_array[i];
        current_value = current_value.trim();
        if(current_value == null){
            System.out.println("Null value");
        }
        System.out.println(field + " "+ current_value);
        if(field.matches("name")){
            current_target.otm.setOTMname(current_value);
        }        
        if(field.matches("OTMpa")){
           try{
              java.lang.Double value = Double.valueOf(current_value);
              current_target.otm.setOTMpa(value);
           }catch(Exception e){
              System.out.println("Error parsing the OTMpa value from the OTM output"+e.toString());
           }
        }
        if(field.matches("OTMslitangle")){
           try{
              java.lang.Double value = Double.valueOf(current_value);
              current_target.otm.setOTMcass(value);
           }catch(Exception e){
              System.out.println("Error parsing the OTMcass value from the OTM output"+e.toString());
           }        
        }
        if(field.matches("OTMwait")){
            try{                
              java.lang.Double value = Double.valueOf(current_value);
              current_target.otm.setOTMwait(value);
            }catch(Exception e){
                System.out.println("Error parsing the OTMwait value from the OTM output"+e.toString());
            }            
        }
        if(field.matches("OTMflag")){
            try{
                current_target.otm.setOTMflag(current_value);
             }catch(Exception e){
                System.out.println("Error parsing the OTMflag value from the OTM output"+e.toString());
            }            
        }
        if(field.matches("OTMslewgo")){
            try{  
                java.sql.Timestamp current_timestamp = string_to_timestamp(current_value);
                current_target.otm.setOTMslewgo(current_timestamp);
            }catch(Exception e){
                System.out.println("Error parsing the OTMslewgo value from the OTM output"+e.toString());
            }            
        }  
        if(field.matches("OTMlast")){
            try{
               current_target.otm.setOTMlast(current_value);
             }catch(Exception e){
                System.out.println("Error parsing the OTMlast value from the OTM output"+e.toString());
            }            
        }
      if(field.matches("OTMslew")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMslew(value);
            }catch(Exception e){
                System.out.println("Error parsing the OTMslew value from the OTM output"+ e.toString());
            }             
       }  
      if(field.matches("OTMdead")){
            try{            
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMdead(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }       
       if(field.matches("OTMstart")){
            try{  
                java.sql.Timestamp current_timestamp = string_to_timestamp(current_value);
                current_target.otm.setOTMstart(current_timestamp);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
       if(field.matches("OTMend")){
            try{  
                java.sql.Timestamp current_timestamp = string_to_timestamp(current_value);
                current_target.otm.setOTMend(current_timestamp);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       } 
       if(field.matches("OTMSNR")){
           try{
               java.lang.String value = new java.lang.String(current_value);
               current_target.otm.setOTMSNR(value);
           }catch(Exception e){
              System.out.println(e.toString()); 
           }
       }
       if(field.matches("OTMairmass_start")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMAirmass_start(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }  
       if(field.matches("OTMairmass_end")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMAirmass_end(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }        
       if(field.matches("OTMsky")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setSkymag(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }   
       if(field.matches("OTMexptime")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMexpt(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }          
       if(field.matches("OTMslitwidth")){
            try{
                java.lang.Double value = Double.valueOf(current_value);
                current_target.otm.setOTMslitwidth(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }    
        if(field.matches("OTMmoon")){
            try{
                java.lang.String value = new java.lang.String(current_value);
                current_target.otm.setOTMmoon(value);
            }catch(Exception e){
                System.out.println(e.toString());
            }            
       }       
      }
     return current_target;
 }
/*================================================================================================
/    readOTMoutput()
/=================================================================================================*/
public void readOTMoutput(){
    try{
//       myTaskSeriesCollection    = new TaskSeriesCollection();
       myOTMTimeSeriesCollection  = new TimeSeriesCollection();  
       myDefaultIntervalXYDataset = new DefaultIntervalXYDataset();  
       myXYSeriesCollection       = new XYSeriesCollection();
         
       java.lang.String OTM_OUTPUT_FILE_STRING    = USERDIR+SEP+"config"+SEP+"otm"+SEP+"OTM_JAVA_OUTPUT.csv";
       java.io.File OTM_OUTPUT_FILE = new java.io.File(OTM_OUTPUT_FILE_STRING);
       java.io.FileReader fr = new java.io.FileReader(OTM_OUTPUT_FILE);
       java.io.BufferedReader br = new java.io.BufferedReader(fr);
       int num_targets = dbms.myTargetDBMSTableModel.getRowCount(); 
       java.lang.String header_line = br.readLine();
       ArrayList        header_list = parseHeaderLine(header_line);
         for(int i=0;i<num_targets;i++){
             Target current = dbms.myTargetDBMSTableModel.getRecord(i);
             java.lang.String current_line = br.readLine();
             current =  parseOTMRecord(current,header_list,current_line);
             if(i==0){
                 long first_image_start = current.otm.getOTMstart().getTime();
                 setSlewgoStart_FirstExposure(first_image_start);
             }
             if(current.otm.getOTMslewgo().getTime() <= current.otm.getOTMend().getTime()){
                 addXIntervalXYDataset(current);
                 TimeSeries current_time_series = constructTimeSeries(current);
                 addTimeSeries(current_time_series);
                 constructXYSeries(i,current);
//                 System.out.println("Added "+current.getName()+" to the chart");
             }           
         }        
       dbms.myTargetDBMSTableModel.fireTableDataChanged();
//       myOTM_Gantt_Chart = new OTM_Gantt_Chart(dbms.getSelectedSetName(),myTaskSeries);
//       myOTM_Gantt_Chart.setVisible(true);
    }catch(Exception e){
       System.out.println("ERROR parsing the OTM output."+e.toString()); 
    }
    dbms.myTargetDBMSTableModel.fireTableDataChanged();
}
    /**
     * Creates a chart_panel for the demo (used by SuperDemo.java).
     *
     * @return A chart_panel.
     */
    public  ChartPanel createDemoPanel() {
        JFreeChart chart = createChart();
        return new ChartPanel(chart);
    }

    /**
     * Starting point for the demonstration application.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        CombinedChartTest demo = new CombinedChartTest("Observation Timeline Model - NGPS");
        demo.pack();
        UIUtils.centerFrameOnScreen(demo);
        demo.setVisible(true);
    }

}
