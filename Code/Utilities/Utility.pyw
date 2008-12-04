import wx
import math

global data
data = []
for x in range(0, 768, 8):
    data.append(200)
    data.append(500)
    data.append(1000)
    data.append(1500)
    data.append(2000)
    data.append(3500)
    data.append(2500)
    data.append(2000)

class LRFPanel(wx.Panel):
    def __init__(self, parent, id):
        wx.Panel.__init__(self, parent, id, size = (600, 480))
        self.SetBackgroundColour(wx.WHITE)
        self.Bind(wx.EVT_PAINT, self.on_paint)
        
    def on_paint(self, event):
        global data
        dc = wx.PaintDC(self)
        dc.SetPen(wx.Pen('black', .25))
        #dc.DrawLine(100,100,200,200)
        i = 0
        for x in range(0, len(data)):
            print i
            i += 1
            x, y = self.to_xy(x, data[x-1])
            dc.DrawLine(240, 600, x, y)
        
    def to_xy(self, step, r):
        theta = (step * (360/1024)) - 135
        x = r * math.sin(theta) + 240
        y = r * math.cos(theta) - 600
        return x, y


class UtilityApp(wx.Frame):
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title, size = (1024, 768))
        self.parent = parent
        self.initialize()
        
    def initialize(self):
        self.Show(True)
        self.panel = LRFPanel(self, wx.ID_ANY)


if __name__ == "__main__":
    app = wx.PySimpleApp()
    frame = UtilityApp(None, wx.ID_ANY, 'SPaRC Utility')
    app.MainLoop()