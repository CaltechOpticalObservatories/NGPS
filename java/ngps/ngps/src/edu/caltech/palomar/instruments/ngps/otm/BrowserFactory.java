/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package edu.caltech.palomar.instruments.ngps.otm;
import static com.teamdev.jxbrowser.engine.RenderingMode.HARDWARE_ACCELERATED;
import com.teamdev.jxbrowser.browser.Browser;
import com.teamdev.jxbrowser.engine.Engine;
import com.teamdev.jxbrowser.engine.EngineOptions;
import com.teamdev.jxbrowser.engine.RenderingMode;
import com.teamdev.jxbrowser.view.swing.BrowserView;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JFrame;
import javax.swing.JInternalFrame;
import javax.swing.SwingUtilities;
import java.nio.file.Paths;

/**
 *
 * @author jennifermilburn
 */
public class BrowserFactory {
    private static final String LICENCE_KEY = "1BNDHFSC1G4J12D7G45JEGZZP6K0GN0DVLI4UKJNT0MB02T0EQ1F0T7C2GB48DHXDV7ZFM";   
    
  
    public static JFrame createFrame(String title, String url, int offset) {
        String userDataDir = System.getProperty("user.home");
        EngineOptions options = EngineOptions.newBuilder(RenderingMode.OFF_SCREEN).licenseKey(LICENCE_KEY).diskCacheSize(100_000_000).build();        
        Engine  engine = Engine.newInstance(options);
        Browser browser = engine.newBrowser();
        browser.navigation().loadUrl(url);

        BrowserView view = BrowserView.newInstance(browser);

        JFrame frame = new JFrame(title);
        frame.addWindowListener(new WindowAdapter() {
                @Override
                public void windowClosing(WindowEvent e) {
                    // Shutdown Chromium and release allocated resources.
                    engine.close();
                }
            });
        frame.setContentPane(view);
        frame.setLocation(100 + offset, 100 + offset);
        frame.setSize(1500, 600);
        frame.setVisible(true);
        return frame;
    }    
}
