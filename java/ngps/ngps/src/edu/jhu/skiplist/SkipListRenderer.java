package edu.jhu.skiplist;

//
// File:        SkipListRenderer.java
//
// Language:    Java 1.02
// Description: Class extending Canvas to render a "SkipList" proposed by William Pugh
//
// Author:  Thomas Wenger, Jan-7-1998
//

import java.awt.*;
import java.applet.*;


public class SkipListRenderer extends Canvas {

  //
  // constants:   ---------
  //              |   |   |
  //              |   |   |
  //              | 2 |---|       ---------   /
  //              |   |  _|_______| 3 |   |   |
  //              |   |   |       |   |   |   | POINTERBOX_HEIGHT
  //              ---------       ---------   /
  //
  //                      /-------/
  //                           BOX_GAP
  //              /---/               /---/
  //                TEXTBOX_WIDTH        POINTERBOX_WIDTH
  //
  protected final int POINTERBOX_HEIGHT = 20;
  protected final int POINTERBOX_WIDTH = 20;
  protected final int TEXTBOX_WIDTH = 40;
  protected final int BOX_GAP = 20;
  protected final int Y_OFFSET = 200;
  protected final int X_OFFSET = 10;
  protected final long NIL_KEY = Long.MAX_VALUE;

  //
  // constructor:
  //   expects the calling applet and the SkipList to render as a parameter
  //
  public SkipListRenderer(Applet applet, SkipList aSkipList) {
	super();                      // calls cosntructor of class Canvas
	// init canvas:
	setBackground(Color.white);
	resize(800,200);
	myApplet = applet;
	mySkipList = aSkipList;
	myXOffset = 0;
  }


  //
  // paint the SkipList  overrides java.awt.Component.paint()
  //
  public void paint(Graphics g) {

	// init holder for last element of a given level:
	int[] lastNr = new int[mySkipList.getMaxLevel()+1];
	for (int i=0; i<=mySkipList.getMaxLevel(); i++) {
	  lastNr[i] = 0;
	}

	// give info in textform about SkipList:
	String info = "";
	info += "Level of this SkipList is " + (mySkipList.getLevel()+1) + ".  ";
	info += "Maximum would be " + (mySkipList.getMaxLevel()+1) + ".  ";
	info += "Probability is " + mySkipList.getProbability() + ".";
	g.setColor(Color.red);
	g.drawString(info,10,20);

	// traverse SkipList at level=0 and draw each element:
	int elementNr = 0;
	SkipListElement element = mySkipList.getHeader();
	while (element != null) {
	  renderNode(g, element, elementNr, lastNr);
	  // update the last element holder to this node:
	  for (int i=0; i<=element.getLevel(); i++) {
		lastNr[i] = elementNr;
	  }
	  // and continue with traversal:
	  element = element.forward[0];
	  elementNr++;
	}
  }


  //
  // helper functions for rendering calculations:
  //

  // calculate the left x coordinate of an element at position elementNr in list:
  private int calculateLeftX(int elementNr) {
	int lx = myXOffset + X_OFFSET + (elementNr)*(TEXTBOX_WIDTH + POINTERBOX_WIDTH + BOX_GAP);
	return lx;
  }

  // calculate the right x coordinate of an element at position elementNr in list:
  private int calculateRightX(int elementNr) {
	int rx = calculateLeftX(elementNr);
	rx += TEXTBOX_WIDTH + POINTERBOX_WIDTH;
	return rx;
  }


  //
  // renderNode():
  //   draws a node of the SkipList at position elementNr in the list
  //   lastNr holds the position of the last node in the list at all levels
  //
  private void renderNode(Graphics g, SkipListElement element, int elementNr, int[] lastNr) {
	// calculate the points of the mainbox:
	int lx = calculateLeftX(elementNr);                           // left x
	int rx = calculateRightX(elementNr);                          // right x
	int by = Y_OFFSET;                                            // bottom y
	int ty = Y_OFFSET - (element.getLevel()+1)*POINTERBOX_HEIGHT; // top y

	// draw the box:
	g.setColor(Color.black);
	g.drawRect(lx,ty,rx-lx,by-ty);

	// draw the pointer boxes:
	for (int i=0; i<=element.getLevel(); i++) {
	  g.drawRect(lx+TEXTBOX_WIDTH,ty+i*POINTERBOX_HEIGHT,POINTERBOX_WIDTH,POINTERBOX_HEIGHT);
	}

	// draw the key as text:
	long key = element.key;
	String keyString;
	if (key == SkipList.HEADER_KEY ) {
	  keyString = "HDR";
	} else if (key == NIL_KEY) {
	  keyString = "NIL";
	} else {
		keyString = Long.toString(key);
	}

	Font f = getFont();
	FontMetrics fm = getFontMetrics(f);
	int dx = (TEXTBOX_WIDTH-fm.stringWidth(keyString)) / 2;
	int dy = (by-ty)/2 + fm.getHeight()/2;
	g.drawString(keyString, lx+dx, ty+dy);

	// draw the arrows:
	if (elementNr > 0) {
	  for (int i=0; i<=element.getLevel(); i++) {
		int lastx = calculateRightX(lastNr[i]) - POINTERBOX_WIDTH/2;
		int x = lx;
		int y = by - (int)((i+0.5)*POINTERBOX_HEIGHT);
		g.drawLine(lastx,y,x,y);            // the line
		g.drawLine(x,y,x-7,y-3);            // upper wing of arrow
		g.drawLine(x,y,x-7,y+3);            // lower wing of arrow
		g.fillOval(lastx-3,y-3,6,6);        // starting circle
	  }
	}

  }


  //
  // calculateImageWidth():
  //   returns width of rendered image of the list in pixels
  //
  public int calculateImageWidth() {
	// init "cursor"-element to header:
	SkipListElement element = mySkipList.getHeader();
	int numberOfNodes = 0;
	// traverse List at level 0 and count nodes:
	while (element != null) {
	  numberOfNodes++;
	  element = element.forward[0];
	}
	// return width in pixels:
	return numberOfNodes * (TEXTBOX_WIDTH+POINTERBOX_WIDTH+BOX_GAP);
  }


  //
  // calculateElementAt():
  //
  public SkipListElement findElementAt(int x, int z) {
	x += -myXOffset - X_OFFSET;
	int elementNr = x / (TEXTBOX_WIDTH + POINTERBOX_WIDTH + BOX_GAP);
	int rest = x % (TEXTBOX_WIDTH + POINTERBOX_WIDTH + BOX_GAP);
	if (rest<(TEXTBOX_WIDTH + POINTERBOX_WIDTH) && rest>0) {
	  SkipListElement element = mySkipList.getHeader();
	  for (int i=0; i<elementNr && element!=null; i++) {
		  element = element.forward[0];
	  }
	  return element;
	}
	return null;
  }


  //
  // set the left beginning of the image in pixels: (used for scrolling)
  //
  public void setXOffset(int newOffset) {
	myXOffset = newOffset;
  }

  public void setSkipList(SkipList aSkipList) {
	mySkipList = aSkipList;
  }

  //
  // members:
  //
  private Applet myApplet;
  private SkipList mySkipList;
  private int myXOffset;
}
