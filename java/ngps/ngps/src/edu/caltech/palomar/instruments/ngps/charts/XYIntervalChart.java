/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.charts;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.text.DateFormat;
import java.util.Date;
import javax.swing.JPanel;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYBarRenderer;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.UIUtils;
import org.jfree.data.xy.DefaultIntervalXYDataset;
import org.jfree.data.xy.IntervalXYDataset;
import javax.swing.JFrame;
import org.jfree.chart.labels.CategoryItemLabelGenerator;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.IntervalCategoryDataset;
import org.jfree.chart.labels.IntervalXYItemLabelGenerator;
import org.jfree.chart.labels.ItemLabelPosition;
import org.jfree.chart.labels.ItemLabelAnchor;
import org.jfree.chart.ui.TextAnchor;
import org.jfree.chart.labels.CustomXYToolTipGenerator;
import org.jfree.chart.labels.StandardXYToolTipGenerator;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.jfree.chart.ChartMouseListener;
import org.jfree.chart.ChartMouseEvent;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.entity.ChartEntity;
import org.jfree.chart.entity.EntityCollection;
import org.jfree.chart.entity.StandardEntityCollection;
import org.jfree.chart.entity.XYItemEntity;
import org.jfree.chart.plot.Crosshair;
import org.jfree.chart.panel.CrosshairOverlay;
import org.jfree.chart.ui.RectangleEdge;
import org.jfree.data.general.DatasetUtils;
import org.jfree.chart.title.LegendTitle;
import org.jfree.data.xy.XYDataItem;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;
import org.jfree.chart.entity.PlotEntity;
import java.awt.BasicStroke;
import java.beans.PropertyChangeListener;
import java.beans.PropertyChangeSupport;

/**
 * A simple demonstration application showing an {@link XYBarRenderer}
 * plotting data from a {@link DefaultIntervalXYDataset}.
 */
public class XYIntervalChart extends JFrame implements ChartMouseListener, MouseMotionListener, MouseListener{
   transient private PropertyChangeSupport propertyChangeListeners = new PropertyChangeSupport(this);
    public   CrosshairOverlay crosshair_overlay ;
    public   Crosshair domain_crosshair;
    public   Crosshair range_crosshair;
    public   ChartPanel chart_panel;
    public  IntervalXYDataset current_dataset;
    private static double MIN_DISTANCE_FROM_POINT = 20;
    private  int selected_series;
    /**
     * 
     * Constructs the demo application.
     *
     * @param title  the frame title.
     */
    public XYIntervalChart(IntervalXYDataset new_current_dataset) {
        current_dataset = new_current_dataset;
        ChartPanel chartPanel = createDemoPanel(current_dataset);
        chartPanel.setPreferredSize(new java.awt.Dimension(500, 300));
        setContentPane(chartPanel);
        this.setSize(1024,856);
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
   /**
     * Creates a chart_panel for the demo.
     *   
     * @return A chart_panel.
     */
    public ChartPanel createDemoPanel(IntervalXYDataset current_dataset) {
        JFreeChart chart = createChart(current_dataset);
        chart_panel = new ChartPanel(chart);
        chart_panel.addChartMouseListener(this);
        chart_panel.setMouseWheelEnabled(true);
        crosshair_overlay = new CrosshairOverlay();
        domain_crosshair  = new Crosshair();
        range_crosshair   = new Crosshair();
        domain_crosshair.setVisible(true);
        range_crosshair.setVisible(true);
        crosshair_overlay.addDomainCrosshair(domain_crosshair);
        crosshair_overlay.addDomainCrosshair(range_crosshair);
        LegendTitle legend = chart.getLegend();
        legend.setPosition(RectangleEdge.LEFT);     
        chart_panel.addOverlay(crosshair_overlay);
        return chart_panel;
    }
    public void chartMouseClicked(ChartMouseEvent chartMouseEvent) {
       System.out.println("CLICK COUNT = "+chartMouseEvent.getTrigger().getClickCount());
       System.out.println("Parameter String = "+chartMouseEvent.getTrigger().paramString());
       System.out.println("Buttons = "+chartMouseEvent.getTrigger().getButton());
       System.out.println("Parameter String = "+chartMouseEvent.getTrigger().getLocationOnScreen());
       System.out.println("X = "+chartMouseEvent.getTrigger().getX());
       System.out.println("Y = "+chartMouseEvent.getTrigger().getY());
       System.out.println("X on screen = "+chartMouseEvent.getTrigger().getXOnScreen());
       System.out.println("Y on screen = "+chartMouseEvent.getTrigger().getYOnScreen());
       System.out.println("Modifiers = "+chartMouseEvent.getTrigger().getModifiersEx()); 
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
                                setSelectedSeries(seriesIndex);
    				// Here I am setting the clicked series to red and increasing the line thickness.
//    				chart_panel.getChart().getXYPlot().getRenderer().setSeriesPaint(seriesIndex, Color.BLUE);
//    				chart_panel.getChart().getXYPlot().getRenderer().setSeriesStroke(seriesIndex, new BasicStroke(2.0f));
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
       System.out.println("CLICK COUNT = "+chartMouseEvent.getTrigger().getClickCount());
       System.out.println("Parameter String = "+chartMouseEvent.getTrigger().paramString());
       System.out.println("Buttons = "+chartMouseEvent.getTrigger().getButton());
       System.out.println("Parameter String = "+chartMouseEvent.getTrigger().getLocationOnScreen());
       System.out.println("X = "+chartMouseEvent.getTrigger().getX());
       System.out.println("Y = "+chartMouseEvent.getTrigger().getY());
       System.out.println("X on screen = "+chartMouseEvent.getTrigger().getXOnScreen());
       System.out.println("Y on screen = "+chartMouseEvent.getTrigger().getYOnScreen());
       System.out.println("Modifiers = "+chartMouseEvent.getTrigger().getModifiersEx()); 
    }  
    private static JFreeChart createChart(IntervalXYDataset dataset) {
        JFreeChart chart = ChartFactory.createXYBarChart(
            "OTM Timeline model",
            "Time (seconds)",
            false,
            "Airmass",
            dataset,   
            PlotOrientation.VERTICAL,
            true,
            true,
            true
        );

        XYPlot plot = (XYPlot) chart.getPlot();
        plot.setDomainPannable(true);
        plot.setRangePannable(true);
        XYBarRenderer renderer = (XYBarRenderer) plot.getRenderer();
        renderer.setUseYInterval(true);
        renderer.setDrawBarOutline(true);
        
        CustomXYToolTipGenerator cxy_tooltip = new CustomXYToolTipGenerator();
        
         StandardXYToolTipGenerator tooltipGenerator = new StandardXYToolTipGenerator(){
    
    public String generateToolTip(IntervalXYDataset dataset, int series, int item){
        return "Series " + series + " Item: " + item + " Value: "
            + dataset.getXValue(series, item) + ";"
            + dataset.getYValue(series, item);
         }
    };        
        
        
        
        ArrayList tooltip_array = new ArrayList();
        int count = dataset.getSeriesCount();
        for(int i=0;i<count;i++){
         renderer.setSeriesItemLabelsVisible(i, Boolean.TRUE);  
//         tooltip_array.add(dataset.getSeriesKey(i));
        }    
        cxy_tooltip.addToolTipSeries(tooltip_array);
        
        for(int j=0;j<count;j++){
            renderer.setSeriesToolTipGenerator(j, tooltipGenerator);
        }
        
        renderer.setSeriesItemLabelsVisible(0, Boolean.TRUE);
        renderer.setDefaultItemLabelGenerator(new CustomIntervalXYItemLabelGenerator());
        renderer.setDefaultItemLabelsVisible(true);        
        renderer.setShadowVisible(false);
 //        renderer.setDefaultPositiveItemLabelPosition(new ItemLabelPosition(ItemLabelAnchor.OUTSIDE3, TextAnchor.CENTER_LEFT));
        ItemLabelPosition p1 = new ItemLabelPosition(ItemLabelAnchor.CENTER,TextAnchor.BOTTOM_CENTER);
        ItemLabelPosition p2 = new ItemLabelPosition(ItemLabelAnchor.CENTER,TextAnchor.BOTTOM_CENTER);
//        renderer.setDefaultNegativeItemLabelPosition(p1);
 //       renderer.setDefaultPositiveItemLabelPosition(p2);
        plot.setRenderer(renderer);  // workaround to update axis range

        return chart;
    }

	public void mousePressed(MouseEvent e) {
		Point2D p2d = chart_panel.translateJava2DToScreen(e.getPoint());
		
		p2d = getNearestPointWithEntity(p2d);
		
//		if(p2d == null) return;
		
//		drawBounding(p2d);
		
		ChartEntity ce = chart_panel.getEntityForPoint((int)p2d.getX(), (int)p2d.getY());
		
//		draggedEntity = ce;
//		firstP2D = p2d;
//		double x = ((XYSeriesCollection)((XYItemEntity)draggedEntity).getDataset()).getXValue(((XYItemEntity)draggedEntity).getSeriesIndex(), ((XYItemEntity)draggedEntity).getItem());
//		double y = ((XYSeriesCollection)((XYItemEntity)draggedEntity).getDataset()).getYValue(((XYItemEntity)draggedEntity).getSeriesIndex(), ((XYItemEntity)draggedEntity).getItem());
//		log(">>> clicked at " + p2d + " (" + x + "," + y + ")");
//				+ " shape=" + draggedEntity.getShapeCoords());
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
/*		if(draggedEntity != null) {
			Point2D p2d = chartpanel.translateScreenToJava2D(e.getPoint());
			
			Graphics2D g2d = (Graphics2D) chartpanel.getGraphics();
			g2d.setPaint(Color.BLACK);
			g2d.setXORMode(Color.WHITE);
			if(line != null) {
				g2d.draw(line);
			}
			line = new Line2D.Double(firstP2D.getX(), firstP2D.getY(), p2d.getX(), p2d.getY());
			g2d.draw(line);
		}
*/	}

	public void mouseReleased(MouseEvent e) {
		Point2D p2d = chart_panel.translateJava2DToScreen(e.getPoint());
		
/*		if(draggedEntity != null) {
			
	        double x = domainAxis.java2DToValue((float) p2d.getX(), chartpanel.getScreenDataArea(), plot.getDomainAxisEdge());
	        double y = rangeAxis.java2DToValue((float) p2d.getY(), chartpanel.getScreenDataArea(), plot.getRangeAxisEdge());
	        
	        log(">>> released at " + p2d + " (" + x + "," + y + ")");
			
			int itemIndex = ((XYItemEntity)draggedEntity).getItem();
			XYSeries series = dataset.getSeries(((XYItemEntity)draggedEntity).getSeriesIndex());
			series.delete(itemIndex, itemIndex);
			series.add(x, y);
			
			line = null;
			draggedEntity = null;
			firstP2D = null;
		}
*/		
		repaint();
	}
	
	public void mouseMoved(MouseEvent e) {
		Point2D p2d = getNearestPointWithEntity(e.getPoint());
		
//		drawBounding(p2d);
	}

	public void mouseClicked(MouseEvent e) {}

	public void mouseEntered(MouseEvent e) {}

	public void mouseExited(MouseEvent e) {}


}

