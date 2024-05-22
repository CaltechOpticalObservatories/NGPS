/* -------------------
 * JFreeChartDemo.java
 * -------------------
 * (C) Copyright 2004-2020, by Object Refinery Limited.
 *
 * 1.5.x
 */

package org.jfree.chart.demo;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.URL;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.ImageIcon;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JEditorPane;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextPane;
import javax.swing.JTree;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingUtilities;
import javax.swing.ToolTipManager;
import javax.swing.UIManager;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.filechooser.FileFilter;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.ChartTransferable;
import org.jfree.chart.ChartUtils;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.StandardChartTheme;
import org.jfree.chart.demo.MemoryUsageDemo.DataGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.CombinedDomainCategoryPlot;
import org.jfree.chart.plot.CombinedDomainXYPlot;
import org.jfree.chart.plot.CombinedRangeCategoryPlot;
import org.jfree.chart.plot.CombinedRangeXYPlot;
import org.jfree.chart.plot.MultiplePiePlot;
import org.jfree.chart.plot.PiePlot;
import org.jfree.chart.plot.Plot;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.ui.ApplicationFrame;
import org.jfree.chart.ui.RectangleInsets;
import org.jfree.chart.ui.UIUtils;
import com.orsonpdf.PDFDocument;
import com.orsonpdf.PDFGraphics2D;
import com.orsonpdf.Page;

/**
 * A collection of demos for JFreeChart.  This application is an "umbrella"
 * for the individual JFreeChart demos - you can run this application to look
 * at all the chart demos, or run each demo on its own.
 */
public class JFreeChartDemo extends ApplicationFrame implements ActionListener, 
        TreeSelectionListener, ChangeListener {

    private static final long serialVersionUID = 1L;

    /** Exit action command. */
    public static final String EXIT_COMMAND = "EXIT";

    private JPanel displayPanel;

    private JPanel chartContainer;
  
    private JPanel descriptionContainer;

    private JTextPane descriptionPane;

    private JEditorPane editorPane;

    private TreePath defaultChartPath;

    JTabbedPane tabs;
    
    private JMenuItem exportToPDFMenuItem;
    
    private JMenuItem exportToSVGMenuItem;
    
    private JMenu editMenu;
    
    private JMenu themeMenu;
    
    /**
     * Creates a new demo instance.
     *
     * @param title  the frame title.
     */
    public JFreeChartDemo(String title) {
        super(title);
        setContentPane(createContent());
        setJMenuBar(createMenuBar());
    }

    /**
     * Creates a panel for the main window.
     *
     * @return A panel.
     */
    private JComponent createContent() {
        JPanel content = new JPanel(new BorderLayout());

        this.tabs = new JTabbedPane();
        tabs.addChangeListener(this);
        JPanel content1 = new JPanel(new BorderLayout());
        content1.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
        JSplitPane splitter = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
        JTree tree = new JTree(createTreeModel());
        tree.addTreeSelectionListener(this);
        JScrollPane treePane = new JScrollPane(tree);
        treePane.setPreferredSize(new Dimension(300, 100));
        splitter.setLeftComponent(treePane);
        splitter.setRightComponent(createChartDisplayPanel());
        content1.add(splitter);
        splitter.setDividerLocation(0.2);
        tabs.add("Demos", content1);
        MemoryUsageDemo memUse = new MemoryUsageDemo(300000);
        memUse.new DataGenerator(1000).start();
        tabs.add("Memory Usage", memUse);
        tabs.add("Source Code", createSourceCodePanel());
        tabs.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
        
        content.add(tabs);
        tree.setSelectionPath(this.defaultChartPath);
        return content;
    }

    private JMenuBar createMenuBar() {
        JMenuBar menuBar = new JMenuBar();

        // first the file menu
        JMenu fileMenu = new JMenu("File", true);
        fileMenu.setMnemonic('F');

        this.exportToPDFMenuItem = new JMenuItem("Export to PDF...", 'p');
        this.exportToPDFMenuItem.setActionCommand("EXPORT_TO_PDF");
        this.exportToPDFMenuItem.addActionListener(this);
        fileMenu.add(this.exportToPDFMenuItem);

        this.exportToSVGMenuItem = new JMenuItem("Export to SVG...", 'j');
        this.exportToSVGMenuItem.setActionCommand("EXPORT_TO_SVG");
        this.exportToSVGMenuItem.addActionListener(this);
        fileMenu.add(this.exportToSVGMenuItem);

        fileMenu.addSeparator();

        JMenuItem exitItem = new JMenuItem("Exit", 'x');
        exitItem.setActionCommand(EXIT_COMMAND);
        exitItem.addActionListener(this);
        fileMenu.add(exitItem);

        // finally, glue together the menu and return it
        menuBar.add(fileMenu);

        editMenu = new JMenu("Edit", false);
        menuBar.add(editMenu);
        JMenuItem copyItem = new JMenuItem("Copy", 'C');
        copyItem.setActionCommand("COPY");
        copyItem.addActionListener(this);
        editMenu.add(copyItem);

        this.themeMenu = new JMenu("Theme", true);
        themeMenu.setMnemonic('T');

        JCheckBoxMenuItem jfree = new JCheckBoxMenuItem("JFree", true);
        jfree.setActionCommand("JFREE_THEME");
        jfree.addActionListener(this);
        themeMenu.add(jfree);

        JCheckBoxMenuItem jfreeshadow = new JCheckBoxMenuItem("JFree/Shadow", false);
        jfreeshadow.setActionCommand("JFREE_SHADOW_THEME");
        jfreeshadow.addActionListener(this);
        themeMenu.add(jfreeshadow);

        JCheckBoxMenuItem darkness = new JCheckBoxMenuItem("Darkness", false);
        darkness.setActionCommand("DARKNESS_THEME");
        darkness.addActionListener(this);
        themeMenu.add(darkness);

        JCheckBoxMenuItem legacy = new JCheckBoxMenuItem("Legacy", false);
        legacy.setActionCommand("LEGACY_THEME");
        legacy.addActionListener(this);
        themeMenu.add(legacy);

        ButtonGroup g = new ButtonGroup();
        g.add(jfree);
        g.add(jfreeshadow);
        g.add(darkness);
        g.add(legacy);
        menuBar.add(themeMenu);

        return menuBar;
    }

    private JPanel createSourceCodePanel() {
        JPanel panel = new JPanel(new BorderLayout());
        panel.setBorder(BorderFactory.createEmptyBorder(4, 4, 4, 4));
        this.editorPane = new JEditorPane();
        this.editorPane.setEditable(false);
        this.editorPane.setFont(new Font("Monospaced", Font.PLAIN, 12));
        updateSourceCodePanel("source.html");

        JScrollPane editorScrollPane = new JScrollPane(this.editorPane);
        editorScrollPane.setVerticalScrollBarPolicy(
                ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED);
        editorScrollPane.setPreferredSize(new Dimension(250, 145));
        editorScrollPane.setMinimumSize(new Dimension(10, 10));

        panel.add(editorScrollPane);
        return panel;
    }

    /**
     * Display source code in the Source Code tab.  If the file cannot be
     * located then default to source.html
     *
     * @param sourceFilename  file to display in the source code panel
     */
    private void updateSourceCodePanel(String sourceFilename) {
        java.net.URL sourceURL = null;
        if (sourceFilename != null) {
            sourceURL = JFreeChartDemo.class.getResource(sourceFilename);
        }
        if (sourceURL == null) {
            // default if java source files are not available
            sourceURL = JFreeChartDemo.class.getResource("source.html");
        }

        if (sourceURL != null) {
            try {
                this.editorPane.setPage(sourceURL);
            }
            catch (IOException e) {
                System.err.println("Attempted to read a bad URL: " + sourceURL);
            }
        }
        else {
            System.err.println("Couldn't find file: source.html");
        }
    }

    private void copyToClipboard() {
        
        // tab 0 - copy the chart
        if (this.tabs.getSelectedIndex() == 0) {
            JFreeChart chart = null;
            int w = 0;
            int h = 0;

            Component c = this.chartContainer.getComponent(0);
            if (c instanceof ChartPanel) {
                ChartPanel cp = (ChartPanel) c;
                chart = cp.getChart();
                w = cp.getWidth();
                h = cp.getHeight();
            }
            else if (c instanceof DemoPanel) {
                DemoPanel dp = (DemoPanel) c;
                chart = dp.charts.get(0);
                w = dp.getWidth();
                h = dp.getHeight();
            }

            if (chart != null) {
                Clipboard systemClipboard
                    = Toolkit.getDefaultToolkit().getSystemClipboard();
                ChartTransferable selection = new ChartTransferable(chart, w, h);
                systemClipboard.setContents(selection, null);
            }
        }
        
        // tab 2 - copy the source code
        else if (this.tabs.getSelectedIndex() == 2) {
            this.editorPane.copy();
        }
    }

    /**
     * Handles menu selections by passing control to an appropriate method.
     *
     * @param event  the event.
     */
    @Override
    public void actionPerformed(ActionEvent event) {
        String command = event.getActionCommand();
        if (command.equals("EXPORT_TO_PDF")) {
            exportToPDF();
        } else if (command.equals("EXPORT_TO_SVG")) {
            exportToSVG();
        } else if (command.equals("COPY")) {
            copyToClipboard();
        } else if (command.equals("LEGACY_THEME")) {
            ChartFactory.setChartTheme(StandardChartTheme.createLegacyTheme());
            applyThemeToChart();
        } else if (command.equals("JFREE_THEME")) {
            ChartFactory.setChartTheme(StandardChartTheme.createJFreeTheme());
            applyThemeToChart();
        } else if (command.equals("JFREE_SHADOW_THEME")) {
            ChartFactory.setChartTheme(new StandardChartTheme("JFreeChart/Shadow", true));
            applyThemeToChart();
        } else if (command.equals("DARKNESS_THEME")) {
            ChartFactory.setChartTheme(StandardChartTheme.createDarknessTheme());
            applyThemeToChart();
        } else if (command.equals(EXIT_COMMAND)) {
            System.exit(0);
        }
    }

    private void applyThemeToChart() {
        Component c = this.chartContainer.getComponent(0);
        if (c instanceof ChartPanel) {
            ChartPanel cp = (ChartPanel) c;
            ChartUtils.applyCurrentTheme(cp.getChart());
        } else if (c instanceof DemoPanel) {
            DemoPanel dp = (DemoPanel) c;
            JFreeChart[] charts = dp.getCharts();
            for (JFreeChart chart : charts) {
                ChartUtils.applyCurrentTheme(chart);
            }
        }
    }
    
    private void exportToSVG() {
        if (this.tabs.getSelectedIndex() != 0) {
            return;
        }
        JFreeChart chart = null;
        int w = 0;
        int h = 0;

        Component c = this.chartContainer.getComponent(0);
        if (c instanceof ChartPanel) {
            ChartPanel cp = (ChartPanel) c;
            chart = cp.getChart();
            w = cp.getWidth();
            h = cp.getHeight();
        }
        else if (c instanceof DemoPanel) {
            DemoPanel dp = (DemoPanel) c;
            chart = dp.charts.get(0);
            w = dp.getWidth();
            h = dp.getHeight();
        }

        if (chart != null) {
            JFileChooser fc = new JFileChooser();
            fc.setName("untitled.html");
            fc.setFileFilter(new FileFilter() {
                @Override
                public boolean accept(File f) {
                    return f.isDirectory() || f.getName().endsWith(".html");
                }

                @Override
                public String getDescription() {
                    return "HTML (HTML)";
                }
            });
            int result = fc.showSaveDialog(this);
            if (result == JFileChooser.APPROVE_OPTION) {
                try {
                    JFreeChart chartClone = (JFreeChart) chart.clone();
                    disableShadowGeneration(chartClone);
 //                   SVGExportTask t = new SVGExportTask(chartClone, w, h,
 //                           fc.getSelectedFile());
 //                   Thread thread = new Thread(t);
 //                   thread.start();
                }
                catch (CloneNotSupportedException e) {
                    e.printStackTrace();
                }
            }
        }
        else {
            String message = "Unable to export the selected item.  There is ";
            message += "either no chart selected,\nor else the chart is not ";
            message += "at the expected location in the component hierarchy\n";
            message += "(future versions of the demo may include code to ";
            message += "handle these special cases).";
            JOptionPane.showMessageDialog(this, message, "SVG Export",
                    JOptionPane.INFORMATION_MESSAGE);
        }
    }
    
    /**
     * Opens a "Save As..." dialog, inviting the user to save the selected
     * chart to a file in PDF format.
     */
    private void exportToPDF() {
        // first tab is the JFreeChart demos
        if (this.tabs.getSelectedIndex() == 0) {
            JFreeChart chart = null;
            int w = 0;
            int h = 0;

            Component c = this.chartContainer.getComponent(0);
            if (c instanceof ChartPanel) {
                ChartPanel cp = (ChartPanel) c;
                chart = cp.getChart();
                w = cp.getWidth();
                h = cp.getHeight();
            }
            else if (c instanceof DemoPanel) {
                DemoPanel dp = (DemoPanel) c;
                chart = dp.charts.get(0);
                w = dp.getWidth();
                h = dp.getHeight();
            }

            if (chart != null) {
                JFileChooser fc = new JFileChooser();
                fc.setName("untitled.pdf");
                fc.setFileFilter(new FileFilter() {

                    @Override
                    public boolean accept(File f) {
                        return f.isDirectory() || f.getName().endsWith(".pdf");
                    }

                    @Override
                    public String getDescription() {
                        return "Portable Document Format (PDF)";
                    }});
                int result = fc.showSaveDialog(this);
                if (result == JFileChooser.APPROVE_OPTION) {
                    try {
                        JFreeChart chartClone = (JFreeChart) chart.clone();
                        disableShadowGeneration(chartClone);
                        PDFExportTask t = new PDFExportTask(chartClone, w, h,
                                fc.getSelectedFile());
                        Thread thread = new Thread(t);
                        thread.start();
                    }
                    catch (CloneNotSupportedException e) {
                        e.printStackTrace();
                    }
                }
            }
            else {
                String message = "Unable to export the selected item.  There is ";
                message += "either no chart selected,\nor else the chart is not ";
                message += "at the expected location in the component hierarchy\n";
                message += "(future versions of the demo may include code to ";
                message += "handle these special cases).";
                JOptionPane.showMessageDialog(this, message, "PDF Export",
                        JOptionPane.INFORMATION_MESSAGE);
            }
        }
    }
    
    /**
     * The shadows generated on charts are bitmap based, and for PDF output
     * we'd prefer to see vector output, so we disable the shadow generation
     * prior to export.
     * 
     * @param chart  the chart. 
     */
    private void disableShadowGeneration(JFreeChart chart) {
        Plot plot = chart.getPlot();
        if (plot instanceof CategoryPlot) {
            ((CategoryPlot) plot).setShadowGenerator(null);
        } else if (plot instanceof XYPlot) {
            ((XYPlot) plot).setShadowGenerator(null);
        } else if (plot instanceof PiePlot) {
            ((PiePlot) plot).setShadowGenerator(null);
        } else if (plot instanceof MultiplePiePlot) {
            disableShadowGeneration(((MultiplePiePlot) plot).getPieChart());
        } else if (plot instanceof CombinedDomainCategoryPlot) {
            ((CombinedDomainCategoryPlot) plot).setShadowGenerator(null);
        } else if (plot instanceof CombinedRangeCategoryPlot) {
            ((CombinedRangeCategoryPlot) plot).setShadowGenerator(null);
        } else if (plot instanceof CombinedDomainXYPlot) {
            ((CombinedDomainXYPlot) plot).setShadowGenerator(null);
        } else if (plot instanceof CombinedRangeXYPlot) {
            ((CombinedRangeXYPlot) plot).setShadowGenerator(null);
        }
    }

    @Override
    public void stateChanged(ChangeEvent e) {
        if (e.getSource() instanceof JTabbedPane) {
            JTabbedPane p = (JTabbedPane) e.getSource();
            if (this.themeMenu != null) {
                this.themeMenu.setEnabled(p.getSelectedIndex() == 0);
            }
            if (this.editMenu != null) {
                this.editMenu.setEnabled(p.getSelectedIndex() == 0 || p.getSelectedIndex() == 2);
            }
            if (this.exportToSVGMenuItem != null) {
                this.exportToSVGMenuItem.setEnabled(p.getSelectedIndex() == 0);
            }
            if (this.exportToPDFMenuItem != null) {
                this.exportToPDFMenuItem.setEnabled(p.getSelectedIndex() == 0);
            }
        }
    }

    static class PDFExportTask implements Runnable {

        JFreeChart chart;

        int width;

        int height;

        File file;

        /**
         * A task that exports a chart to a file in PDF format using 
         * JFreeGraphics2D.
         *
         * @param chart  the chart.
         * @param width  the width.
         * @param height  the height.
         * @param file  the file.
         */
        public PDFExportTask(JFreeChart chart, int width, int height,
                File file) {
            this.chart = chart;
            this.file = file;
            this.width = width;
            this.height = height;
            chart.setBorderVisible(true);
            chart.setPadding(new RectangleInsets(2, 2, 2, 2));
        }

        /**
         * Performs the task.
         */
        @Override
        public void run() {
            try {
                JFreeChartDemo.saveChartAsPDF(this.file, this.chart, this.width,
                        this.height);
            }
            catch (IOException e) {
                e.printStackTrace();
            }
        }

    }

    /**
     * Saves a chart to a file in PDF format using OrsonPDF.
     *
     * @param file  the file.
     * @param chart  the chart.
     * @param width  the chart width.
     * @param height  the chart height.
     *
     * @throws IOException if there is an I/O problem.
     */
    public static void saveChartAsPDF(File file, JFreeChart chart, int width,
            int height) throws IOException {
        PDFDocument pdfDoc = new PDFDocument();
        Page page = pdfDoc.createPage(new Rectangle(width, height));
        PDFGraphics2D g2 = page.getGraphics2D();
        chart.draw(g2, new Rectangle(width, height));
        pdfDoc.writeToFile(file);
    }


    private JPanel createChartDisplayPanel() {

        this.displayPanel = new JPanel(new BorderLayout());
        this.displayPanel.setPreferredSize(new Dimension(600, 400));
        this.chartContainer = new JPanel(new BorderLayout());
        this.chartContainer.setPreferredSize(new Dimension(600, 500));
        this.chartContainer.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createEmptyBorder(4, 4, 4, 4),
                BorderFactory.createLineBorder(Color.BLACK)));
        this.chartContainer.add(createNoDemoSelectedPanel());
        this.descriptionContainer = new JPanel(new BorderLayout());
        this.descriptionContainer.setBorder(BorderFactory.createEmptyBorder(4, 
                4, 4, 4));
        this.descriptionContainer.setPreferredSize(new Dimension(600, 140));
        this.descriptionPane = new JTextPane();
        this.descriptionPane.setEditable(false);
        JScrollPane scroller = new JScrollPane(this.descriptionPane,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        this.descriptionContainer.add(scroller);
        displayDescription("select.html");
        final JSplitPane splitter = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
        splitter.setTopComponent(this.chartContainer);
        splitter.setBottomComponent(this.descriptionContainer);
        this.displayPanel.add(splitter);
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                splitter.setDividerLocation(0.6);
            }
        });
        return this.displayPanel;
    }

    /**
     * Creates a {@code TreeModel} with references to all the individual
     * demo applications.  This is an ugly piece of hard-coding but, hey, it's
     * just a demo!
     *
     * @return A TreeModel.
     */
    private TreeModel createTreeModel() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("JFreeChart");
        MutableTreeNode showcase = createShowcaseNode(root);
        root.add(showcase);
        root.add(createAreaChartsNode());
        root.add(createBarChartsNode());
        root.add(createStackedBarChartsNode());
        root.add(createCombinedAxisChartsNode());
        root.add(createFinancialChartsNode());
        root.add(createGanttChartsNode());
        root.add(createLineChartsNode());
        root.add(createMeterChartsNode());
        root.add(createMultipleAxisChartsNode());
        root.add(createOverlaidChartsNode());
        root.add(createPieChartsNode());
        root.add(createStatisticalChartsNode());
        root.add(createTimeSeriesChartsNode());
        root.add(createXYChartsNode());
        root.add(createMiscellaneousChartsNode());
        return new DefaultTreeModel(root);
    }

    /**
     * Creates the tree node for the pie chart demos.
     *
     * @return A populated tree node.
     */
    private MutableTreeNode createPieChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Pie Charts");
        root.add(createNode(PieChartDemo1.class, "PieChartDemo1.java"));
        root.add(createNode(PieChartDemo2.class, "PieChartDemo2.java"));
        root.add(createNode(PieChartDemo3.class, "PieChartDemo3.java"));
        root.add(createNode(PieChartDemo4.class, "PieChartDemo4.java"));
        root.add(createNode(PieChartDemo5.class, "PieChartDemo5.java"));
        root.add(createNode(PieChartDemo6.class, "PieChartDemo6.java"));
        root.add(createNode(PieChartDemo7.class, "PieChartDemo7.java"));
        root.add(createNode(PieChartDemo8.class, "PieChartDemo8.java"));
        root.add(createNode(PieChart3DDemo1.class, "PieChart3DDemo1.java"));
        root.add(createNode(PieChart3DDemo2.class, "PieChart3DDemo2.java"));
        root.add(createNode(PieChart3DDemo3.class, "PieChart3DDemo3.java"));
        root.add(createNode(MultiplePieChartDemo1.class, "MultiplePieChartDemo1.java"));
        root.add(createNode(MultiplePieChartDemo2.class, "MultiplePieChartDemo2.java"));
        root.add(createNode(MultiplePieChartDemo3.class, "MultiplePieChartDemo3.java"));
        root.add(createNode(MultiplePieChartDemo4.class, "MultiplePieChartDemo4.java"));
        root.add(createNode(RingChartDemo1.class, "RingChartDemo1.java"));
        root.add(createNode(RingChartDemo2.class, "RingChartDemo2.java"));
        return root;
    }

    /**
     * Creates the tree node for the overlaid chart demos.
     *
     * @return A populated tree node.
     */
    private MutableTreeNode createOverlaidChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Overlaid Charts");
        root.add(createNode(OverlaidChartDemo1.class, "OverlaidChartDemo1.java"));
        root.add(createNode(OverlaidBarChartDemo1.class, "OverlaidBarChartDemo1.java"));
        root.add(createNode(OverlaidBarChartDemo2.class, "OverlaidBarChartDemo2.java"));
        root.add(createNode(OverlaidXYPlotDemo1.class, "OverlaidXYPlotDemo1.java"));
        root.add(createNode(OverlaidXYPlotDemo2.class, "OverlaidXYPlotDemo2.java"));
        return root;
    }

    /**
     * Creates a tree node containing sample bar charts.
     *
     * @return The tree node.
     */
    private MutableTreeNode createBarChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Bar Charts");
        root.add(createCategoryBarChartsNode());
        root.add(createXYBarChartsNode());
        return root;
    }

    /**
     * A node for various stacked bar charts.
     *
     * @return The node.
     */
    private MutableTreeNode createStackedBarChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Bar Charts - Stacked");
        root.add(createNode(PopulationChartDemo1.class, "PopulationChartDemo1.java"));
        root.add(createNode(StackedBarChartDemo1.class, "StackedBarChartDemo1.java"));
        root.add(createNode(StackedBarChartDemo2.class, "StackedBarChartDemo2.java"));
        root.add(createNode(StackedBarChartDemo3.class, "StackedBarChartDemo3.java"));
        root.add(createNode(StackedBarChartDemo4.class, "StackedBarChartDemo4.java"));
        root.add(createNode(StackedBarChartDemo5.class, "StackedBarChartDemo5.java"));
        root.add(createNode(StackedBarChartDemo6.class, "StackedBarChartDemo6.java"));
        root.add(createNode(StackedBarChartDemo7.class, "StackedBarChartDemo7.java"));
        root.add(createNode(StackedBarChart3DDemo4.class, "StackedBarChart3DDemo4.java"));
        root.add(createNode(StackedBarChart3DDemo5.class, "StackedBarChart3DDemo5.java"));
        return root;
    }

    /**
     * Creates a tree node containing bar charts based on the
     * {@link CategoryPlot} class.
     *
     * @return The tree node.
     */
    private MutableTreeNode createCategoryBarChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("CategoryPlot");

        root.add(createNode(BarChartDemo1.class, "BarChartDemo1.java"));
        root.add(createNode(BarChartDemo2.class, "BarChartDemo2.java"));
        root.add(createNode(BarChartDemo3.class, "BarChartDemo3.java"));
        root.add(createNode(BarChartDemo4.class, "BarChartDemo4.java"));
        root.add(createNode(BarChartDemo5.class, "BarChartDemo5.java"));
        root.add(createNode(BarChartDemo6.class, "BarChartDemo6.java"));
        root.add(createNode(BarChartDemo7.class, "BarChartDemo7.java"));
        root.add(createNode(BarChartDemo8.class, "BarChartDemo8.java"));
        root.add(createNode(BarChartDemo9.class, "BarChartDemo9.java"));
        root.add(createNode(BarChartDemo10.class, "BarChartDemo10.java"));
        root.add(createNode(BarChartDemo11.class, "BarChartDemo11.java"));
        root.add(createNode(IntervalBarChartDemo1.class, "IntervalBarChartDemo1.java"));
        root.add(createNode(LayeredBarChartDemo1.class, "LayeredBarChartDemo1.java"));
        root.add(createNode(LayeredBarChartDemo2.class, "LayeredBarChartDemo2.java"));
        root.add(createNode(SlidingCategoryDatasetDemo1.class, "SlidingCategoryDatasetDemo1.java"));
        root.add(createNode(SlidingCategoryDatasetDemo2.class, "SlidingCategoryDatasetDemo2.java"));
        root.add(createNode(StatisticalBarChartDemo1.class, "StatisticalBarChartDemo1.java"));
        root.add(createNode(SurveyResultsDemo1.class, "SurveyResultsDemo1.java"));
        root.add(createNode(SurveyResultsDemo2.class, "SurveyResultsDemo2.java"));
        root.add(createNode(SurveyResultsDemo3.class, "SurveyResultsDemo3.java"));
        root.add(createNode(WaterfallChartDemo1.class, "WaterfallChartDemo1.java"));
        return root;
    }

    private MutableTreeNode createXYBarChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("XYPlot");
        root.add(createNode(XYBarChartDemo1.class, "XYBarChartDemo1.java"));
        root.add(createNode(XYBarChartDemo2.class, "XYBarChartDemo2.java"));
        root.add(createNode(XYBarChartDemo3.class, "XYBarChartDemo3.java"));
        root.add(createNode(XYBarChartDemo4.class, "XYBarChartDemo4.java"));
        root.add(createNode(XYBarChartDemo5.class, "XYBarChartDemo5.java"));
        root.add(createNode(XYBarChartDemo6.class, "XYBarChartDemo6.java"));
        root.add(createNode(XYBarChartDemo7.class, "XYBarChartDemo7.java"));
        root.add(createNode(ClusteredXYBarRendererDemo1.class, "ClusteredXYBarRendererDemo1.java"));
        root.add(createNode(StackedXYBarChartDemo1.class, "StackedXYBarChartDemo1.java"));
        root.add(createNode(StackedXYBarChartDemo2.class, "StackedXYBarChartDemo2.java"));
        root.add(createNode(StackedXYBarChartDemo3.class, "StackedXYBarChartDemo3.java"));
        root.add(createNode(RelativeDateFormatDemo1.class, "RelativeDateFormatDemo1.java"));
        root.add(createNode(RelativeDateFormatDemo2.class, "RelativeDateFormatDemo2.java"));
        return root;
    }

    /**
     * Creates a tree node containing line chart items.
     *
     * @return A tree node.
     */
    private MutableTreeNode createLineChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Line Charts");
        root.add(createNode(AnnotationDemo1.class, "AnnotationDemo1.java"));
        root.add(createNode(LineChartDemo1.class, "LineChartDemo1.java"));
        root.add(createNode(LineChartDemo2.class, "LineChartDemo2.java"));
        root.add(createNode(LineChartDemo3.class, "LineChartDemo3.java"));
        root.add(createNode(LineChartDemo4.class, "LineChartDemo4.java"));
        root.add(createNode(LineChartDemo5.class, "LineChartDemo5.java"));
        root.add(createNode(LineChartDemo6.class, "LineChartDemo6.java"));
        root.add(createNode(LineChartDemo7.class, "LineChartDemo7.java"));
        root.add(createNode(LineChartDemo8.class, "LineChartDemo8.java"));
        root.add(createNode(StatisticalLineChartDemo1.class, "StatisticalLineChartDemo1.java"));
        root.add(createNode(XYSplineRendererDemo1.class, "XYSplineRendererDemo1.java"));
        root.add(createNode(XYStepRendererDemo1.class, "XYStepRendererDemo1.java"));
        root.add(createNode(XYStepRendererDemo2.class, "XYStepRendererDemo2.java"));
        return root;
    }

    private MutableTreeNode createNode(Class demoClass, String file) {
        return new DefaultMutableTreeNode(new DemoDescription(demoClass, file));
    }

    /**
     * A node for the 'cool' charts.
     *
     * @param r  the root node.
     *
     * @return The node.
     */
    private MutableTreeNode createShowcaseNode(DefaultMutableTreeNode r) {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode(
                "*** Showcase Charts ***");
        MutableTreeNode defaultNode = createNode(BarChartDemo1.class, "BarChartDemo1.java");
        this.defaultChartPath = new TreePath(new Object[] {r, root, defaultNode});
        root.add(defaultNode);
        root.add(createNode(CrosshairOverlayDemo2.class, "CrosshairOverlayDemo2.java"));
        root.add(createNode(CrosshairDemo2.class, "CrosshairDemo2.java"));
        root.add(createNode(CrossSectionDemo1.class, "CrossSectionDemo1.java"));
        root.add(createNode(DeviationRendererDemo2.class, "DeviationRendererDemo2.java"));
        root.add(createNode(DifferenceChartDemo1.class, "DifferenceChartDemo1.java"));
        root.add(createNode(DifferenceChartDemo2.class, "DifferenceChartDemo2.java"));
        root.add(createNode(DialDemo2a.class, "DialDemo2a.java"));
        root.add(createNode(DualAxisDemo1.class, "DualAxisDemo1.java"));
        root.add(createNode(HistogramDemo1.class, "HistogramDemo1.java"));
        root.add(createNode(LineChartDemo1.class, "LineChartDemo1.java"));
        root.add(createNode(MultipleAxisDemo1.class, "MultipleAxisDemo1.java"));
        root.add(createNode(MultiplePieChartDemo1.class, "MultiplePieChartDemo1.java"));
        root.add(createNode(NormalDistributionDemo2.class, "NormalDistributionDemo2.java"));
        root.add(createNode(OverlaidChartDemo1.class, "OverlaidChartDemo1.java"));
        root.add(createNode(ParetoChartDemo1.class, "ParetoChartDemo1.java"));
        root.add(createNode(PieChartDemo1.class, "PieChartDemo1.java"));
        root.add(createNode(PieChartDemo2.class, "PieChartDemo2.java"));
        root.add(createNode(PieChartDemo4.class, "PieChartDemo4.java"));
        root.add(createNode(PriceVolumeDemo1.class, "PriceVolumeDemo1.java"));
        root.add(createNode(RingChartDemo2.class, "RingChartDemo2.java"));
        root.add(createNode(ScatterPlotDemo4.class, "ScatterPlotDemo4.java"));
        root.add(createNode(SlidingCategoryDatasetDemo2.class, "SlidingCategoryDatasetDemo2.java"));
        root.add(createNode(StackedBarChartDemo2.class, "StackedBarChartDemo2.java"));
        root.add(createNode(StackedXYBarChartDemo2.class, "StackedXYBarChartDemo2.java"));
        root.add(createNode(StatisticalBarChartDemo1.class, "StatisticalBarChartDemo1.java"));
        root.add(createNode(TimeSeriesDemo6.class, "TimeSeriesDemo6.java"));
        root.add(createNode(TimeSeriesDemo14.class, "TimeSeriesDemo14.java"));
        root.add(createNode(VectorPlotDemo1.class, "VectorPlotDemo1.java"));
        root.add(createNode(WaterfallChartDemo1.class, "WaterfallChartDemo1.java"));
        root.add(createNode(XYDrawableAnnotationDemo1.class, "XYDrawableAnnotationDemo1.java"));
        root.add(createNode(XYSplineRendererDemo1.class, "XYSplineRendererDemo1.java"));
        root.add(createNode(XYTaskDatasetDemo2.class, "XYTaskDatasetDemo2.java"));
        root.add(createNode(YieldCurveDemo1.class, "YieldCurveDemo1.java"));
        return root;
    }

    /**
     * A node for various area charts.
     *
     * @return The node.
     */
    private MutableTreeNode createAreaChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Area Charts");
        root.add(createNode(AreaChartDemo1.class, "AreaChartDemo1.java"));
        root.add(createNode(StackedAreaChartDemo1.class, "StackedAreaChartDemo1.java"));
        root.add(createNode(StackedXYAreaChartDemo1.class, "StackedXYAreaChartDemo1.java"));
        root.add(createNode(StackedXYAreaChartDemo2.class, "StackedXYAreaChartDemo2.java"));
        root.add(createNode(StackedXYAreaRendererDemo1.class, "StackedXYAreaRendererDemo1.java"));
        root.add(createNode(XYAreaChartDemo1.class, "XYAreaChartDemo1.java"));
        root.add(createNode(XYAreaChartDemo2.class, "XYAreaChartDemo2.java"));
        root.add(createNode(XYAreaRenderer2Demo1.class, "XYAreaRenderer2Demo1.java"));
        root.add(createNode(XYStepAreaRendererDemo1.class, "XYStepAreaRendererDemo1.java"));
        return root;
    }

    private MutableTreeNode createStatisticalChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Statistical Charts");
        root.add(createNode(BoxAndWhiskerChartDemo1.class, "BoxAndWhiskerChartDemo1.java"));
        root.add(createNode(BoxAndWhiskerChartDemo2.class, "BoxAndWhiskerChartDemo2.java"));
        root.add(createNode(HistogramDemo1.class, "HistogramDemo1.java"));
        root.add(createNode(HistogramDemo2.class, "HistogramDemo2.java"));
        root.add(createNode(MinMaxCategoryPlotDemo1.class, "MinMaxCategoryPlotDemo1.java"));
        root.add(createNode(NormalDistributionDemo1.class, "NormalDistributionDemo1.java"));
        root.add(createNode(NormalDistributionDemo2.class, "NormalDistributionDemo2.java"));
        root.add(createNode(RegressionDemo1.class, "RegressionDemo1.java"));
        root.add(createNode(ScatterPlotDemo1.class, "ScatterPlotDemo1.java"));
        root.add(createNode(ScatterPlotDemo2.class, "ScatterPlotDemo2.java"));
        root.add(createNode(ScatterPlotDemo3.class, "ScatterPlotDemo3.java"));
        root.add(createNode(ScatterPlotDemo4.class, "ScatterPlotDemo4.java"));
        root.add(createNode(ScatterPlotDemo5.class, "ScatterPlotDemo5.java"));
        root.add(createNode(XYErrorRendererDemo1.class, "XYErrorRendererDemo1.java"));
        root.add(createNode(XYErrorRendererDemo2.class, "XYErrorRendererDemo2.java"));
        return root;
    }

    /**
     * Creates a sub-tree for the time series charts.
     *
     * @return The root node for the subtree.
     */
    private MutableTreeNode createTimeSeriesChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode(
                "Time Series Charts");

        root.add(createNode(TimeSeriesDemo1.class, "TimeSeriesDemo1.java"));
        root.add(createNode(TimeSeriesDemo2.class, "TimeSeriesDemo2.java"));
        root.add(createNode(TimeSeriesDemo3.class, "TimeSeriesDemo3.java"));
        root.add(createNode(TimeSeriesDemo4.class, "TimeSeriesDemo4.java"));
        root.add(createNode(TimeSeriesDemo5.class, "TimeSeriesDemo5.java"));
        root.add(createNode(TimeSeriesDemo6.class, "TimeSeriesDemo6.java"));
        root.add(createNode(TimeSeriesDemo7.class, "TimeSeriesDemo7.java"));
        root.add(createNode(TimeSeriesDemo8.class, "TimeSeriesDemo8.java"));
        root.add(createNode(TimeSeriesDemo9.class, "TimeSeriesDemo9.java"));
        root.add(createNode(TimeSeriesDemo10.class, "TimeSeriesDemo10.java"));
        root.add(createNode(TimeSeriesDemo11.class, "TimeSeriesDemo11.java"));
        root.add(createNode(TimeSeriesDemo12.class, "TimeSeriesDemo12.java"));
        root.add(createNode(TimeSeriesDemo13.class, "TimeSeriesDemo13.java"));
        root.add(createNode(TimeSeriesDemo14.class, "TimeSeriesDemo14.java"));

        root.add(createNode(PeriodAxisDemo1.class, "PeriodAxisDemo1.java"));
        root.add(createNode(PeriodAxisDemo2.class, "PeriodAxisDemo2.java"));
        root.add(createNode(PeriodAxisDemo3.class, "PeriodAxisDemo3.java"));
        root.add(createNode(RelativeDateFormatDemo1.class, "RelativeDateFormatDemo1.java"));
        root.add(createNode(DeviationRendererDemo1.class, "DeviationRendererDemo1.java"));
        root.add(createNode(DeviationRendererDemo2.class, "DeviationRendererDemo2.java"));
        root.add(createNode(DifferenceChartDemo1.class, "DifferenceChartDemo1.java"));
        root.add(createNode(DifferenceChartDemo2.class, "DifferenceChartDemo2.java"));
        root.add(createNode(CompareToPreviousYearDemo.class, "CompareToPreviousYearDemo.java"));

        return root;
    }

    /**
     * Creates a node for the tree model that contains financial charts.
     *
     * @return The tree node.
     */
    private MutableTreeNode createFinancialChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Financial Charts");
        root.add(createNode(CandlestickChartDemo1.class, "CandlestickChartDemo1.java"));
        root.add(createNode(HighLowChartDemo1.class, "HighLowChartDemo1.java"));
        root.add(createNode(HighLowChartDemo2.class, "HighLowChartDemo2.java"));
        root.add(createNode(HighLowChartDemo3.class, "HighLowChartDemo3.java"));
        root.add(createNode(MovingAverageDemo1.class, "MovingAverageDemo1.java"));
        root.add(createNode(PriceVolumeDemo1.class, "PriceVolumeDemo1.java"));
        root.add(createNode(PriceVolumeDemo2.class, "PriceVolumeDemo2.java"));
        root.add(createNode(YieldCurveDemo1.class, "YieldCurveDemo1.java"));
        return root;
    }

    private MutableTreeNode createXYChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("XY Charts");
        root.add(createNode(ScatterPlotDemo1.class, "ScatterPlotDemo1.java"));
        root.add(createNode(ScatterPlotDemo2.class, "ScatterPlotDemo2.java"));
        root.add(createNode(ScatterPlotDemo3.class, "ScatterPlotDemo3.java"));
        root.add(createNode(LogAxisDemo1.class, "LogAxisDemo1.java"));
        root.add(createNode(Function2DDemo1.class, "Function2DDemo1.java"));
        root.add(createNode(XYBlockChartDemo1.class, "XYBlockChartDemo1.java"));
        root.add(createNode(XYBlockChartDemo2.class, "XYBlockChartDemo2.java"));
        root.add(createNode(XYBlockChartDemo3.class, "XYBlockChartDemo3.java"));
        root.add(createNode(XYLineAndShapeRendererDemo1.class, "XYLineAndShapeRendererDemo1.java"));
        root.add(createNode(XYLineAndShapeRendererDemo2.class, "XYLineAndShapeRendererDemo2.java"));
        root.add(createNode(XYSeriesDemo1.class, "XYSeriesDemo1.java"));
        root.add(createNode(XYSeriesDemo2.class, "XYSeriesDemo2.java"));
        root.add(createNode(XYSeriesDemo3.class, "XYSeriesDemo3.java"));
        root.add(createNode(XYShapeRendererDemo1.class, "XYShapeRendererDemo1.java"));
        root.add(createNode(VectorPlotDemo1.class, "VectorPlotDemo1.java"));
        return root;
    }

    /**
     * Creates a node for the tree model that contains dial and meter charts.
     *
     * @return The tree node.
     */
    private MutableTreeNode createMeterChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode(
                "Dial / Meter Charts");
        root.add(createNode(DialDemo1.class, "DialDemo1.java"));
        root.add(createNode(DialDemo2.class, "DialDemo2.java"));
        root.add(createNode(DialDemo2a.class, "DialDemo2a.java"));
        root.add(createNode(DialDemo3.class, "DialDemo3.java"));
        root.add(createNode(DialDemo4.class, "DialDemo4.java"));
        root.add(createNode(DialDemo5.class, "DialDemo5.java"));
        root.add(createNode(MeterChartDemo1.class, "MeterChartDemo1.java"));
        root.add(createNode(MeterChartDemo2.class, "MeterChartDemo2.java"));
        root.add(createNode(MeterChartDemo3.class, "MeterChartDemo3.java"));
        root.add(createNode(ThermometerDemo1.class, "ThermometerDemo1.java"));
        return root;
    }

    private MutableTreeNode createMultipleAxisChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode(
                "Multiple Axis Charts");
        root.add(createNode(DualAxisDemo1.class, "DualAxisDemo1.java"));
        root.add(createNode(DualAxisDemo2.class, "DualAxisDemo2.java"));
        root.add(createNode(DualAxisDemo3.class, "DualAxisDemo3.java"));
        root.add(createNode(DualAxisDemo4.class, "DualAxisDemo4.java"));
        root.add(createNode(DualAxisDemo5.class, "DualAxisDemo5.java"));
        root.add(createNode(MultipleAxisDemo1.class, "MultipleAxisDemo1.java"));
        root.add(createNode(MultipleAxisDemo2.class, "MultipleAxisDemo2.java"));
        root.add(createNode(MultipleAxisDemo3.class, "MultipleAxisDemo3.java"));
        root.add(createNode(ParetoChartDemo1.class, "ParetoChartDemo1.java"));
        return root;
    }

    private MutableTreeNode createCombinedAxisChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Combined Axis Charts");
        root.add(createNode(CombinedCategoryPlotDemo1.class, "CombinedCategoryPlotDemo1.java"));
        root.add(createNode(CombinedCategoryPlotDemo2.class, "CombinedCategoryPlotDemo2.java"));
        root.add(createNode(CombinedTimeSeriesDemo1.class, "CombinedTimeSeriesDemo1.java"));
        root.add(createNode(CombinedXYPlotDemo1.class, "CombinedXYPlotDemo1.java"));
        root.add(createNode(CombinedXYPlotDemo2.class, "CombinedXYPlotDemo2.java"));
        root.add(createNode(CombinedXYPlotDemo3.class, "CombinedXYPlotDemo3.java"));
        root.add(createNode(CombinedXYPlotDemo4.class, "CombinedXYPlotDemo4.java"));
        return root;
    }

    private MutableTreeNode createGanttChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Gantt Charts");
        root.add(createNode(GanttDemo1.class, "GanttDemo1.java"));
        root.add(createNode(GanttDemo2.class, "GanttDemo2.java"));
        root.add(createNode(SlidingGanttDatasetDemo1.class, "SlidingGanttDatasetDemo1.java"));
        root.add(createNode(XYTaskDatasetDemo1.class, "XYTaskDatasetDemo1"));
        root.add(createNode(XYTaskDatasetDemo2.class, "XYTaskDatasetDemo2"));
        return root;
    }

    /**
     * Creates the subtree containing the miscellaneous chart types.
     *
     * @return A subtree.
     */
    private MutableTreeNode createMiscellaneousChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Miscellaneous");
        root.add(createAnnotationsNode());
        root.add(createCrosshairChartsNode());
        root.add(createDynamicChartsNode());
        root.add(createItemLabelsNode());
        root.add(createLegendNode());
        root.add(createMarkersNode());
        root.add(createOrientationNode());

        root.add(createNode(AxisOffsetsDemo1.class, "AxisOffsetsDemo1.java"));
        root.add(createNode(BubbleChartDemo1.class, "BubbleChartDemo1.java"));
        root.add(createNode(BubbleChartDemo2.class, "BubbleChartDemo2.java"));
        root.add(createNode(CategoryLabelPositionsDemo1.class, "CategoryLabelPositionsDemo1.java"));
        root.add(createNode(CategoryStepChartDemo1.class, "CategoryStepChartDemo1.java"));
        root.add(createNode(CompassDemo1.class, "CompassDemo1.java"));
        root.add(createNode(CompassFormatDemo1.class, "CompassFormatDemo1.java"));
        root.add(createNode(CompassFormatDemo2.class, "CompassFormatDemo2.java"));
        root.add(createNode(EventFrequencyDemo1.class, "EventFrequencyDemo1.java"));
        root.add(createNode(GradientPaintTransformerDemo1.class, "GradientPaintTransformerDemo1.java"));
        root.add(createNode(GridBandDemo1.class, "GridBandDemo1.java"));
        root.add(createNode(HideSeriesDemo1.class, "HideSeriesDemo1.java"));
        root.add(createNode(HideSeriesDemo2.class, "HideSeriesDemo2.java"));
        root.add(createNode(HideSeriesDemo3.class, "HideSeriesDemo3.java"));
        root.add(createNode(MultipleDatasetDemo1.class, "MultipleDatasetDemo1.java"));
        root.add(createNode(PolarChartDemo1.class, "PolarChartDemo1.java"));
        root.add(createNode(ScatterRendererDemo1.class, "ScatterRendererDemo1.java"));
        root.add(createNode(SpiderWebChartDemo1.class, "SpiderWebChartDemo1.java"));
        root.add(createNode(SymbolAxisDemo1.class, "SymbolAxisDemo1.java"));
        root.add(createNode(ThermometerDemo1.class, "ThermometerDemo1.java"));
        root.add(createNode(ThermometerDemo2.class, "ThermometerDemo2.java"));
        root.add(createNode(ThumbnailDemo1.class, "ThumbnailDemo1.java"));
        root.add(createNode(TranslateDemo1.class, "TranslateDemo1.java"));
        root.add(createNode(WindChartDemo1.class, "WindChartDemo1.java"));
        root.add(createNode(YIntervalChartDemo1.class, "YIntervalChartDemo1.java"));
        root.add(createNode(YIntervalChartDemo2.class, "YIntervalChartDemo2.java"));
        return root;
    }

    private MutableTreeNode createAnnotationsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Annotations");
        root.add(createNode(AnnotationDemo1.class, "AnnotationDemo1.java"));
        root.add(createNode(AnnotationDemo2.class, "AnnotationDemo2.java"));
        root.add(createNode(CategoryPointerAnnotationDemo1.class, "CategoryPointerAnnotationDemo1.java"));
        root.add(createNode(XYBoxAnnotationDemo1.class, "XYBoxAnnotationDemo1.java"));
        root.add(createNode(XYPolygonAnnotationDemo1.class, "XYPolygonAnnotationDemo1.java"));
        root.add(createNode(XYTitleAnnotationDemo1.class, "XYTitleAnnotationDemo1.java"));
        return root;
    }

    private MutableTreeNode createCrosshairChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Crosshairs");
        root.add(createNode(CrosshairOverlayDemo1.class, "CrosshairOverlayDemo1.java"));
        root.add(createNode(CrosshairOverlayDemo2.class, "CrosshairOverlayDemo2.java"));
        root.add(createNode(CrosshairDemo1.class, "CrosshairDemo1.java"));
        root.add(createNode(CrosshairDemo2.class, "CrosshairDemo2.java"));
        root.add(createNode(CrosshairDemo3.class, "CrosshairDemo3.java"));
        root.add(createNode(CrosshairDemo4.class, "CrosshairDemo4.java"));
        return root;
    }

    private MutableTreeNode createDynamicChartsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Dynamic Charts");
        root.add(createNode(DynamicDataDemo1.class, "DynamicDataDemo1.java"));
        root.add(createNode(DynamicDataDemo2.class, "DynamicDataDemo2.java"));
        root.add(createNode(DynamicDataDemo3.class, "DynamicDataDemo3.java"));
        root.add(createNode(MouseOverDemo1.class, "MouseOverDemo1.java"));
        return root;
    }

    private MutableTreeNode createItemLabelsNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Item Labels");
        root.add(createNode(ItemLabelDemo1.class, "ItemLabelDemo1.java"));
        root.add(createNode(ItemLabelDemo2.class, "ItemLabelDemo2.java"));
        root.add(createNode(ItemLabelDemo3.class, "ItemLabelDemo3.java"));
        root.add(createNode(ItemLabelDemo4.class, "ItemLabelDemo4.java"));
        root.add(createNode(ItemLabelDemo5.class, "ItemLabelDemo5.java"));
        return root;
    }

    private MutableTreeNode createLegendNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Legends");
        root.add(createNode(LegendWrapperDemo1.class, "LegendWrapperDemo1.java"));
        return root;
    }

    private MutableTreeNode createMarkersNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Markers");
        root.add(createNode(CategoryMarkerDemo1.class, "CategoryMarkerDemo1.java"));
        root.add(createNode(CategoryMarkerDemo2.class, "CategoryMarkerDemo2.java"));
        root.add(createNode(MarkerDemo1.class, "MarkerDemo1.java"));
        root.add(createNode(MarkerDemo2.class, "MarkerDemo2.java"));
        return root;
    }

    private MutableTreeNode createOrientationNode() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Plot Orientation");
        root.add(createNode(PlotOrientationDemo1.class, "PlotOrientationDemo1.java"));
        root.add(createNode(PlotOrientationDemo2.class, "PlotOrientationDemo2.java"));
        return root;
    }

    private void displayDescription(String fileName) {
        java.net.URL descriptionURL = JFreeChartDemo.class.getResource(fileName);
        if (descriptionURL != null) {
            try {
                this.descriptionPane.setPage(descriptionURL);
            }
            catch (IOException e) {
                System.err.println("Attempted to read a bad URL: "
                        + descriptionURL);
            }
        }
        else {
            System.err.println("Couldn't find file: " + fileName);
        }

    }

    /**
     * Receives notification of tree selection events and updates the demo
     * display accordingly.
     *
     * @param event  the event.
     */
    @Override
    public void valueChanged(TreeSelectionEvent event) {
        String sourceFilename = null;
        TreePath path = event.getPath();
        Object obj = path.getLastPathComponent();
        if (obj != null) {
            DefaultMutableTreeNode n = (DefaultMutableTreeNode) obj;
            Object userObj = n.getUserObject();
            if (userObj instanceof DemoDescription) {
                DemoDescription dd = (DemoDescription) userObj;
                sourceFilename = dd.getDescription();
                updateSourceCodePanel(sourceFilename);
                SwingUtilities.invokeLater(new DisplayDemo(this, dd));
            }
            else {
                this.chartContainer.removeAll();
                this.chartContainer.add(createNoDemoSelectedPanel());
                this.displayPanel.validate();
                displayDescription("select.html");
                updateSourceCodePanel(null);
            }
        }
    }

    private JPanel createNoDemoSelectedPanel() {
        JPanel panel = new JPanel(new FlowLayout()) {

            /* (non-Javadoc)
             * @see javax.swing.JComponent#getToolTipText()
             */
            @Override
            public String getToolTipText() {
                return "(" + getWidth() + ", " + getHeight() + ")";
            }

        };
        ToolTipManager.sharedInstance().registerComponent(panel);
        panel.add(new JLabel("No demo selected"));
        panel.setPreferredSize(new Dimension(600, 400));
        return panel;
    }

    /**
     * Starting point for the JFreeChart Demo Collection.
     *
     * @param args  ignored.
     */
    public static void main(String[] args) {
        SwingUtilities.invokeLater(new Runnable() {
            @Override
            public void run() {
                // let's see if Nimbus is available...
                try {
                    UIManager.setLookAndFeel("com.sun.java.swing.plaf.nimbus.NimbusLookAndFeel");
                }
                catch (Exception e) {
                    // ... otherwise just use the system look and feel
                    try {
                        UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
                    }
                    catch (Exception e2) {
                        e2.printStackTrace();
                    }
                }

                JFreeChartDemo demo = new JFreeChartDemo("JFreeChart 1.5.2 Demo Collection");
                demo.pack();
                UIUtils.centerFrameOnScreen(demo);
                demo.setVisible(true);
            }  
        });
    }

    static class DisplayDemo implements Runnable {

        private JFreeChartDemo app;

        private DemoDescription demoDescription;

        /**
         * Creates a new runnable.
         *
         * @param app  the application.
         * @param d  the demo description.
         */
        public DisplayDemo(JFreeChartDemo app, DemoDescription d) {
            this.app = app;
            this.demoDescription = d;
        }

        /**
         * Runs the task.
         */
        @Override
        public void run() {
            try {
                Class c = this.demoDescription.getDemoClass();
                Method m = c.getDeclaredMethod("createDemoPanel",
                        (Class[]) null);
                JPanel panel = (JPanel) m.invoke(null, (Object[]) null);
                this.app.chartContainer.removeAll();
                this.app.chartContainer.add(panel);
                this.app.displayPanel.validate();
                String className = c.getName();
                String fileName = className;
                int i = className.lastIndexOf('.');
                if (i > 0) {
                    fileName = className.substring(i + 1);
                }
                fileName = fileName + ".html";
                this.app.displayDescription(fileName);

            }
            catch (NoSuchMethodException e2) {
                e2.printStackTrace();
            }
            catch (InvocationTargetException e3) {
                e3.printStackTrace();
            }
            catch (IllegalAccessException e4) {
                e4.printStackTrace();
            }

        }

    }
    
    /**
     * Returns a small image of a gorilla used for demo purposes.
     * 
     * @return A small image. 
     */
    public static Image getTestImage() {
        URL imageURL = JFreeChartDemo.class.getClassLoader().getResource(
                    "org/jfree/chart/demo/gorilla.jpg");
        ImageIcon temp = new ImageIcon(imageURL);
        return temp.getImage();
    }
}

