/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.charts;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.Line2D.Double;
import java.util.Iterator;
import java.util.List;
import java.util.Random;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSpinner;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.SpinnerNumberModel;
import javax.swing.WindowConstants;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.entity.ChartEntity;
import org.jfree.chart.entity.EntityCollection;
import org.jfree.chart.entity.StandardEntityCollection;
import org.jfree.chart.entity.XYItemEntity;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.DefaultXYItemRenderer;
import org.jfree.data.xy.XYDataItem;
import org.jfree.data.xy.XYSeries;
import org.jfree.data.xy.XYSeriesCollection;


/**
* This code was edited or generated using CloudGarden's Jigloo
* SWT/Swing GUI Builder, which is free for non-commercial
* use. If Jigloo is being used commercially (ie, by a corporation,
* company or business for any purpose whatever) then you
* should purchase a license for each developer using Jigloo.
* Please visit www.cloudgarden.com for details.
* Use of Jigloo implies acceptance of these licensing terms.
* A COMMERCIAL LICENSE HAS NOT BEEN PURCHASED FOR
* THIS MACHINE, SO JIGLOO OR THIS CODE CANNOT BE USED
* LEGALLY FOR ANY CORPORATE OR COMMERCIAL PURPOSE.
*/
public class EntityTester extends JFrame implements MouseMotionListener, MouseListener {
	private static final int DATA_COUNT = 100;
	private static double MIN_DISTANCE_FROM_POINT = 20;
	private JFreeChart chart;
	private ChartPanel chartpanel;
	private JTextArea taLog;
	private JSpinner tfRadius;
	private XYSeriesCollection dataset;
	private ChartEntity draggedEntity;
	private Point2D firstP2D;
	private Double line;
	private XYPlot plot;
	private ValueAxis domainAxis;
	private ValueAxis rangeAxis;
	private JButton buttonSet;
	private Shape lastBound;
	private JPanel buttonPanel;

	public static void main(String[] args) {
		new EntityTester(args);
	}
	
	public EntityTester(String[] args) {
		super("EntityTester");
		
		dataset = createData();
		chart = ChartFactory.createXYLineChart(
				"Entity Test", "X", "Y",
				dataset,
				PlotOrientation.VERTICAL,
				true, true, true);
		
		plot = chart.getXYPlot();
		domainAxis = plot.getDomainAxis(); 
		rangeAxis = plot.getRangeAxis();
		
		DefaultXYItemRenderer renderer = new DefaultXYItemRenderer();
//		renderer.setShapesVisible(true);
//		renderer.setLinesVisible(true);
		chart.getXYPlot().setRenderer(renderer);
		
//		replaceEntities();
		getContentPane().setLayout(new BorderLayout());
		
		chartpanel = new ChartPanel(chart);
		chartpanel.setMouseZoomable(false);
		chartpanel.setMinimumDrawWidth(0);
		chartpanel.setMinimumDrawHeight(0);
		chartpanel.setMaximumDrawWidth(Integer.MAX_VALUE);
		chartpanel.setMaximumDrawHeight(Integer.MAX_VALUE);
		chartpanel.addMouseListener(this);
		chartpanel.addMouseMotionListener(this);
		
		buttonPanel = new JPanel();
		GridBagLayout buttonPanelLayout = new GridBagLayout();
		buttonPanelLayout.columnWidths = new int[] { 7, 20, 7, 7, 7 };
		buttonPanelLayout.rowHeights = new int[] { 7, 7, 7, 7, 7 };
		buttonPanelLayout.columnWeights = new double[] { 0.0, 0.1, 1.0, 0.1, 0.0 };
		buttonPanelLayout.rowWeights = new double[] { 0.0, 0.0, 0.0, 0.1, 0.0 };
		buttonPanel.setLayout(buttonPanelLayout);
		
		tfRadius = new JSpinner(new SpinnerNumberModel(
				MIN_DISTANCE_FROM_POINT,
				1,
				Integer.MAX_VALUE,
				1));
		buttonPanel.add(tfRadius, new GridBagConstraints(
				2,
				1,
				1,
				1,
				0.0,
				0.0,
				GridBagConstraints.CENTER,
				GridBagConstraints.HORIZONTAL,
				new Insets(0, 0, 0, 0),
				0,
				0));
		buttonSet = new JButton("Set");
		buttonPanel.add(buttonSet, new GridBagConstraints(
				3,
				1,
				1,
				1,
				0.0,
				0.0,
				GridBagConstraints.CENTER,
				GridBagConstraints.NONE,
				new Insets(0, 0, 0, 0),
				0,
				0));
		buttonSet.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				MIN_DISTANCE_FROM_POINT = ((Number) tfRadius.getValue()).intValue();
			}
		});
		buttonPanel.add(new JLabel("Radius:"), new GridBagConstraints(
				1,
				1,
				1,
				1,
				0.0,
				0.0,
				GridBagConstraints.CENTER,
				GridBagConstraints.NONE,
				new Insets(0, 0, 0, 0),
				0,
				0));
		taLog = new JTextArea(5, 70);
		taLog.setText("Here goes the output...\n");
		buttonPanel.add(new JScrollPane(taLog), new GridBagConstraints(
				1,
				3,
				3,
				1,
				0.0,
				0.0,
				GridBagConstraints.CENTER,
				GridBagConstraints.BOTH,
				new Insets(0, 0, 0, 0),
				0,
				0));
		
		getContentPane().add(new JSplitPane(JSplitPane.VERTICAL_SPLIT, chartpanel, buttonPanel), BorderLayout.CENTER);

//		setBounds(30, 30, 600, 600);
		pack();
		setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
		setVisible(true);
	}

	private void replaceEntities() {
		StandardEntityCollection newEntities = new StandardEntityCollection();
		XYItemEntity entity;
		Shape shape;
		int SHAPE_SIZE = 20;
		double x, y;

		List series = dataset.getSeries();
		for (int i = 0; i < series.size(); i++) {
			XYSeries s = (XYSeries) series.get(i);
			for (int index = 0; index < s.getItemCount(); index++) {
				XYDataItem item = (XYDataItem) s.getItems().get(index);
				x = domainAxis.valueToJava2D(item.getX().doubleValue(), chartpanel.getScreenDataArea(), plot.getDomainAxisEdge());
				y = rangeAxis.valueToJava2D(item.getY().doubleValue(), chartpanel.getScreenDataArea(), plot.getRangeAxisEdge());
				shape = new Rectangle2D.Double(x - SHAPE_SIZE/2, y - SHAPE_SIZE/2, x + SHAPE_SIZE/2, y + SHAPE_SIZE/2);
				entity = new XYItemEntity(
						shape,
						dataset,
						i, index, 
						x + "," + y, "urltext");
				log(">>> shape=" + entity.getShapeCoords());
				newEntities.add(entity);
			}
		}
		
		chartpanel.getChartRenderingInfo().setEntityCollection(newEntities);
	}

	private void log(String msg) {
		taLog.append(msg + "\n");
		taLog.setCaretPosition(taLog.getText().length());
	}

	public void mousePressed(MouseEvent e) {
		Point2D p2d = chartpanel.translateJava2DToScreen(e.getPoint());
		
		p2d = getNearestPointWithEntity(p2d);
		
		if(p2d == null) return;
		
		drawBounding(p2d);
		
		ChartEntity ce = chartpanel.getEntityForPoint((int)p2d.getX(), (int)p2d.getY());
		
		draggedEntity = ce;
		firstP2D = p2d;
		double x = ((XYSeriesCollection)((XYItemEntity)draggedEntity).getDataset()).getXValue(((XYItemEntity)draggedEntity).getSeriesIndex(), ((XYItemEntity)draggedEntity).getItem());
		double y = ((XYSeriesCollection)((XYItemEntity)draggedEntity).getDataset()).getYValue(((XYItemEntity)draggedEntity).getSeriesIndex(), ((XYItemEntity)draggedEntity).getItem());
		log(">>> clicked at " + p2d + " (" + x + "," + y + ")");
//				+ " shape=" + draggedEntity.getShapeCoords());
	}

	private Point2D getNearestPointWithEntity(Point2D p) {
		double minDistance = MIN_DISTANCE_FROM_POINT;
		
		Point2D point = null;
		EntityCollection entities = chartpanel.getChartRenderingInfo().getEntityCollection();
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
		if(draggedEntity != null) {
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
	}

	public void mouseReleased(MouseEvent e) {
		Point2D p2d = chartpanel.translateJava2DToScreen(e.getPoint());
		
		if(draggedEntity != null) {
			
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
		
		repaint();
	}
	
	public void mouseMoved(MouseEvent e) {
		Point2D p2d = getNearestPointWithEntity(e.getPoint());
		
		drawBounding(p2d);
	}

	public void mouseClicked(MouseEvent e) {}

	public void mouseEntered(MouseEvent e) {}

	public void mouseExited(MouseEvent e) {}
	
	private void drawBounding(Point2D p2d) {
		Graphics2D g2d = (Graphics2D) chartpanel.getGraphics();
		g2d.setPaint(Color.BLACK);
		g2d.setXORMode(Color.WHITE);
		
		if(p2d == null) return;
		
//		Rectangle2D.Double last = 
//			new Rectangle2D.Double(
//					(int)(p2d.getX() - MIN_DISTANCE_FROM_POINT), (int)(p2d.getY() - MIN_DISTANCE_FROM_POINT),
//					(int)MIN_DISTANCE_FROM_POINT*2, (int)MIN_DISTANCE_FROM_POINT*2);
		Ellipse2D.Double last = new Ellipse2D.Double(
				p2d.getX() - MIN_DISTANCE_FROM_POINT, p2d.getY() - MIN_DISTANCE_FROM_POINT,
				MIN_DISTANCE_FROM_POINT*2, MIN_DISTANCE_FROM_POINT*2);
		if(lastBound != last) { 
			// remove last bounding
			if(lastBound != null) {
				g2d.draw(lastBound);
			}
			g2d.draw(last);
			lastBound = last;
		}
	}

	private XYSeriesCollection createData() {
		XYSeriesCollection col = new XYSeriesCollection();
		XYSeries series = new XYSeries("data");
		
		double lastX = 0;
		double lastY = 0;
		double x;
		double y;
		
		Random r = new Random(System.currentTimeMillis());
		for (int i = 0; i < DATA_COUNT; i++) {
			x = i;
			y = r.nextDouble() + r.nextDouble() * lastY;
			
			series.add(x, y);
			
			lastX = x;
			lastY = y;
		}
		col.addSeries(series);
		
		return col;
	}
}