import g4p_controls.*;

import org.gamecontrolplus.*;

import processing.core.*;

public class TSelectEntry implements Comparable<TSelectEntry> {
  public final PApplet app;
  public final ControlIO controlIO;
  public final ControlDevice device;
  public final GLabel displayName;
  public final GButton btnGoConfig;
  public TConfigUI winCofig = null;
  public String string;
  public GDropList ipList;

  public TSelectEntry(PApplet papp, ControlIO controlIO, ControlDevice dev, String string) {
    this.app = papp;
    this.controlIO = controlIO;
    this.device = dev;
    this.string = string;
    displayName = new GLabel(papp, 36, 20, app.width-36, 20);
    displayName.setText(string);
    displayName.setTextAlign(GAlign.LEFT, null);
    btnGoConfig = new GButton(app, 4, 24, 24, 14);
    btnGoConfig.addEventHandler(this, "configClick");
    
    ipList = new GDropList(papp, 200, 30, 120, 100);
    ipList.setItems(new String[]{"N/A"}, 0);
  }

  public void setIndex(int startY, int index) {
    displayName.moveTo(36, startY + index * 20);
    if (btnGoConfig != null) btnGoConfig.moveTo(4, startY + 4 + index * 20);
    if (ipList != null) ipList.moveTo(200, startY + index * 20);
  }

  public void configClick(GButton source, GEvent event){
      winCofig = new TConfigUI(app, this);
  }

  @Override
    public int compareTo(TSelectEntry entry) {
    return device.compareTo(entry.device);
  }
}
