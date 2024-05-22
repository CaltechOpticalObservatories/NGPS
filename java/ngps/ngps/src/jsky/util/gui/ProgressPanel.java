/*
 * $Id: ProgressPanel.java,v 1.2 2009/04/21 13:31:17 abrighto Exp $
 */

package jsky.util.gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
 
import javax.swing.*;

import jsky.util.Resources;
import jsky.util.StatusLogger;


/**
 * A panel to display while a download or other background operation is in
 * progress.
 * <p/>
 * This class is designed to be usable from any thread and all GUI access is done
 * synchronously in the event dispatching thread.
 *
 * @author Allan Brighton
 * @version $Revision: 1.2 $
 */
public class ProgressPanel extends JPanel implements ActionListener, StatusLogger {

    // Parent of this window (frame or internal frame), used to close the window
    private JDialog _parent;

    // The title string
    private String _title;

    // Displays the title
    private JLabel _titleLabel;

    // Button to interrupt the task
    private JButton _stopButton;

    // Displays the progress bar and status text
    private StatusPanel _statusPanel;

    // If set, this is the current input stream being monitored
    private ProgressBarFilterInputStream _loggedInputStream;

    // Set to true if the stop button was pressed
    private boolean _interrupted;

    // Used to create a new progress panel in the event dispatching thread
    private static ProgressPanel _newPanel;


    /**
     * Initialize a progress panel with the given title string.
     *
     * @param parent the parent dialog (of this progress panel), used to close the window
     * @param title  the title string
     */
    public ProgressPanel(JDialog parent, String title) {
        _parent = parent;
        _title = title;
        init();
    }

    /**
     * Default constructor
     */
    public ProgressPanel() {
        this(null, "Download in Progress...");
    }

    /**
     * @return the dialog stop button
     */
    public JButton getStopButton() {
        return _stopButton;
    }


    /**
     * Initialize the progreass panel. This method may be called from any
     * thread, but will always run in the event dispatching thread.
     */
    protected void init() {
        // make sure this is done in the event dispatch thread
        if (!SwingUtilities.isEventDispatchThread()) {
            invokeAndWait(new Runnable() {
                public void run() {
                    init();
                }
            });
            return;
        }
        setLayout(new BorderLayout());
        setBorder(BorderFactory.createEtchedBorder());
        JPanel top = new JPanel();
        top.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        top.setLayout(new BorderLayout());
        _titleLabel = new JLabel(_title, SwingConstants.CENTER);
        _titleLabel.setForeground(Color.black);
        top.add(_titleLabel, BorderLayout.WEST);
        JLabel iconLabel = new JLabel(Resources.getIcon("TaskStatusOn.gif"));
        top.add(iconLabel, BorderLayout.EAST);

        JPanel center = new JPanel();
        _stopButton = new JButton("Stop");
        _stopButton.addActionListener(this);
        center.add(_stopButton);
        top.add(center, BorderLayout.SOUTH);

        _statusPanel = new StatusPanel();
        _statusPanel.getTextField().setColumns(25);

        add(top, BorderLayout.NORTH);
        add(_statusPanel, BorderLayout.SOUTH);
    }


    /**
     * Run the given Runnable synchronously in the event dispatching thread.
     * @param r the runnable
     */
    protected static void invokeAndWait(Runnable r) {
        try {
            SwingUtilities.invokeAndWait(r);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void setTitle(final String title) {
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    setTitle(title);
                }
            });
            return;
        }
        this._title = title;
        _titleLabel.setText(title);
    }

    /**
     * Log or display the given message
     */
    public void logMessage(final String msg) {
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    _statusPanel.setText(msg);
                }
            });
            return;
        }
        _statusPanel.setText(msg);
    }

    public void setText(final String s) {
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    _statusPanel.setText(s);
                }
            });
            return;
        }
        _statusPanel.setText(s);
    }


    /**
     * Add a listener to be called when the user presses the stop button.
     * @param l the listener
     */
    public void addActionListener(ActionListener l) {
        _stopButton.addActionListener(l);
    }

    /**
     * Remove the listener
     * @param l the listener
     */
    public void removeActionListener(ActionListener l) {
        _stopButton.removeActionListener(l);
    }

    /**
     * @return the status panel (displays the progress bar and text field).
     */
    public StatusPanel getStatusPanel() {
        return _statusPanel;
    }

    /**
     * Called when the Stop button is pressed.
     */
    public void actionPerformed(ActionEvent e) {
        _interrupted = true;
        stop();
    }

    /**
     * @return true if the stop button was pressed
     */
    public boolean isInterrupted() {
        return _interrupted;
    }

    /**
     * Return a connection to the given URL and log messages before and after
     * opening the connection.
     */
    public URLConnection openConnection(URL url) throws IOException {
        start();
        URLConnection connection = _statusPanel.openConnection(url);
        if (_interrupted) {
            throw new ProgressException("Interrupted");
        }
        return connection;
    }


    /**
     * Display the progress panel. This method may be called from any
     * thread, but will always run in the event dispatching thread.
     */
    public void start() {
        // make sure this is done in the event dispatch thread
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    start();
                }
            });
            return;
        }

        _interrupted = false;
        _parent.setVisible(true);
        _statusPanel.getProgressBar().startAnimation();
        BusyWin.setBusy(true, _parent);
    }

    /**
     * Starts or restarts the progress bar animation
     */
    public void startAnimation() {
        _statusPanel.getProgressBar().startAnimation();
    }


    /**
     * Stop displaying the progress panel. This method may be called
     * from any thread, but will always run in the event dispatching
     * thread.
     */
    public void stop() {
        // make sure this is done in the event dispatch thread
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    stop();
                }
            });
            return;
        }

        BusyWin.setBusy(false, _parent);
        if (_loggedInputStream != null) {
            _loggedInputStream.interrupt();
            _loggedInputStream = null;
        }
        _parent.setVisible(false);
        _statusPanel.interrupt();
        _statusPanel.getProgressBar().stopAnimation();
        _statusPanel.setText("");
        _statusPanel.getProgressBar().setStringPainted(false);
        _statusPanel.getProgressBar().setValue(0);
        _parent.dispose(); // XXX
    }


    /**
     * Make a ProgressPanel and frame and return the panel.
     *
     * @param title  the title string
     * @param window window to display the dialog over, may be null
     * @return the panel
     */
    public static ProgressPanel makeProgressPanel(final String title, final Component window) {
        if (!SwingUtilities.isEventDispatchThread()) {
            invokeAndWait(new Runnable() {
                public void run() {
                    _newPanel = ProgressPanel.makeProgressPanel(title, window);
                }
            });
            return _newPanel;
        }

        // get the parent frame so that the dialog won't be hidden behind it
        Frame parent = SwingUtil.getFrame(window);
        ProgressPanelDialog f = new ProgressPanelDialog(title, parent);
//        f.show();
        f.setVisible(true);
        return f.getProgressPanel();
    }


    /**
     * Make a ProgressPanel and frame  and return the panel.
     *
     * @param title the title string
     * @return the panel
     */
    public static ProgressPanel makeProgressPanel(String title) {
        return makeProgressPanel(title, DialogUtil.getActiveFrame());
    }


    /**
     * Make a ProgressPanel and frame and return the panel.
     * @return the panel
     */
    public static ProgressPanel makeProgressPanel() {
        return makeProgressPanel("Downloading data...");
    }

    /**
     * Set the percent done. A 0 value resets the bar and hides the percent value.
     */
    public void setProgress(final int percent) {
        if (!SwingUtilities.isEventDispatchThread()) {
            SwingUtilities.invokeLater(new Runnable() {
                public void run() {
                    _statusPanel.getProgressBar().stopAnimation();
                    _statusPanel.setProgress(percent);
                }
            });
            return;
        }
        _statusPanel.getProgressBar().stopAnimation();
        _statusPanel.setProgress(percent);
    }


    /**
     * Return a input stream that will generate log messages showing
     * the progress of the read from the given stream.
     *
     * @param in   the input stream to be monitored
     * @param size the size in bytes of the date to be read, or 0 if not known
     */
    public ProgressBarFilterInputStream getLoggedInputStream(InputStream in, int size) throws IOException {
        if (_interrupted) {
            throw new ProgressException("Interrupted");
        }
        _loggedInputStream = _statusPanel.getLoggedInputStream(in, size);
        return _loggedInputStream;
    }


    /**
     * Return an input stream to use for reading from the given URL
     * that will generate log messages showing the progress of the read.
     *
     * @param url the URL to read
     */
    public ProgressBarFilterInputStream getLoggedInputStream(URL url) throws IOException {
        if (_interrupted) {
            throw new ProgressException("Interrupted");
        }
        _loggedInputStream = _statusPanel.getLoggedInputStream(url);
        return _loggedInputStream;
    }

    /**
     * Stop logging reads from the input stream returned from an
     * earlier call to getLoggedInputStream().
     *
     * @param in an input stream returned from getLoggedInputStream()
     */
    public void stopLoggingInputStream(ProgressBarFilterInputStream in) throws IOException {
        _loggedInputStream = null;
        _statusPanel.stopLoggingInputStream(in);
    }
}

