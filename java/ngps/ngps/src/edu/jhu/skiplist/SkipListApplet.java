package edu.jhu.skiplist;

//
// File:        SkipListApplet.java
//
// Language:    Java 1.02
// Description: Class for a Applet that demonstrates a "SkipList" proposed by William Pugh
//
// Author:  Thomas Wenger, Jan-7-1998
//

import java.awt.*;
import java.applet.*;


public class SkipListApplet extends Applet {


  //
  // constructor:
  //
  public SkipListApplet() {
	mySkipList = new SkipList(0.5f, 6);
	mySkipListRenderer = new SkipListRenderer(this, mySkipList);
  }


  //
  // init():  is called by Webbrowser when started first time
  //
  public void init() {
	// build the GUI:
	setLayout(new BorderLayout());

	Panel p0 = new Panel();
	p0.add(new Label("SkipList Demonstration     (C) 1997 Thomas Wenger      "));
	p0.add(myRandomizeButton = new Button("Insert 10 Random nodes"));
	p0.add(myResetButton = new Button("Reset"));

	add("North",p0);
	add("Center",mySkipListRenderer);

	Panel p1 = new Panel();
	p1.add(new Label("Key:"));
	p1.add(myKeyTextField = new TextField(6));
	p1.add(new Label("Value:"));
	p1.add(myValueTextField = new TextField(6));
	myKeyTextField.setText("");
	myValueTextField.setText("0");

	p1.add(myInsertButton = new Button("Insert"));
	p1.add(mySearchButton = new Button("Search"));
	p1.add(myDeleteButton = new Button("Delete"));
	p1.add(new Label("Result:"));
	p1.add(myOutputTextField = new TextField(35));

	Panel p = new Panel();
	p.setLayout(new BorderLayout());
	p.add("North",myScrollbar = new Scrollbar(Scrollbar.HORIZONTAL,0,250,0,1000));
	p.add("South",p1);

	add("South",p);
  }


  //
  // handleEvent():  is called by AWT event handling mechanism for all events
  //
  public boolean handleEvent(Event  ev) {
	//
	// scrollbare moved:
	//
	if (ev.target == myScrollbar) {
	  if (ev.id == Event.SCROLL_LINE_UP || ev.id == Event.SCROLL_LINE_DOWN ||
		  ev.id == Event.SCROLL_PAGE_UP || ev.id == Event.SCROLL_PAGE_DOWN ||
		  ev.id == Event.SCROLL_ABSOLUTE) {
		int pos = ((Integer)ev.arg).intValue();
		Rectangle bounds = mySkipListRenderer.bounds();
		int widthToScroll = mySkipListRenderer.calculateImageWidth()-bounds.width;
		widthToScroll = ( (widthToScroll<0) ? 0 : widthToScroll);
		int newOffset = (int)((-pos/1000f)*widthToScroll);
		mySkipListRenderer.setXOffset(newOffset);
		mySkipListRenderer.repaint();
		return true;
	  }
	}
	//
	// nothing processed:
	//
	return super.handleEvent(ev);
  }


  //
  // action():  is called by AWT event handling mechanism for ACTION events
  //
  public boolean action(Event  ev, Object  obj) {
	//
	// insert pressed:
	//
	if (ev.target == myInsertButton) {
	  long key = Integer.parseInt(myKeyTextField.getText());
	  long value = Integer.parseInt(myValueTextField.getText());
	  myKeyTextField.setText("");
	  myValueTextField.setText("0");
	  mySkipList.insert(key,value);
	  myOutputTextField.setText("Node (k=" + key + ", v=" + value + ") inserted.");
	  mySkipListRenderer.repaint();
	  return true;
	}
	//
	// search pressed:
	//
	if (ev.target == mySearchButton) {
	  long key = Integer.parseInt(myKeyTextField.getText());
	  long value = mySkipList.search(key);
	  myKeyTextField.setText("");
	  myValueTextField.setText("0");
	  if (value != SkipList.NOT_FOUND) {
		myOutputTextField.setText("Found: Node (k=" + key + ", v=" + value + ")");
	  }
	  else {
		myOutputTextField.setText("Not found: Node with k=" + key);
	  }
	  return true;
	}
	//
	// delete pressed:
	//
	if (ev.target == myDeleteButton) {
	  long key = Integer.parseInt(myKeyTextField.getText());
	  long value = mySkipList.search(key);
	  myKeyTextField.setText("");
	  myValueTextField.setText("0");
	  if (value != SkipList.NOT_FOUND && key != Integer.MAX_VALUE) {
		mySkipList.delete(key);
		myOutputTextField.setText("Deleted: Node (k=" + key + ", v=" + value + ")");
		mySkipListRenderer.repaint();
	  }
	  else {
		myOutputTextField.setText("Not deleted: No node with k=" + key);
	  }
	  return true;
	}
	//
	// randomize insertion button pressed:
	//
	if (ev.target == myRandomizeButton) {
	  for (int i=0; i<10; i++) {
		long key = (long)(Math.random()*100);
		long value = (long)(Math.random()*100);
		mySkipList.insert(key,value);
	  }
	  myOutputTextField.setText("10 random nodes inserted");
	  mySkipListRenderer.repaint();
	  return true;
	}
	//
	// reset button pressed:
	//
	if (ev.target == myResetButton) {
	  mySkipList = new SkipList(0.5f, 6);
	  mySkipListRenderer.setSkipList(mySkipList);
	  mySkipListRenderer.setXOffset(0);
	  mySkipListRenderer.repaint();
	  myScrollbar.setValue(0);
	  myOutputTextField.setText("List reseted.");
	  return true;
	}
	//
	// nothing processed:
	//
	return super.action(ev,obj);
  }


  //
  // mouseDown():  is called by AWT event handling mechanism for mouse events
  //
  public boolean mouseDown(Event ev, int x, int y) {
	if (ev.target == mySkipListRenderer) {
	  SkipListElement element = mySkipListRenderer.findElementAt(x,y);
	  if (element != null && element.key>=0 && element.key<Integer.MAX_VALUE) {
		myKeyTextField.setText(Long.toString(element.key));
		myValueTextField.setText(Long.toString(element.value));
		myOutputTextField.setText("Node selected.");
	  }
	  return true;
	}
	return false;
  }


  //
  // members:
  //
  private SkipListRenderer mySkipListRenderer;
  private SkipList mySkipList;
  private TextField myKeyTextField;
  private TextField myValueTextField;
  private TextField myOutputTextField;
  private Button myInsertButton;
  private Button mySearchButton;
  private Button myDeleteButton;
  private Button myRandomizeButton;
  private Button myResetButton;
  private Scrollbar myScrollbar;
}
