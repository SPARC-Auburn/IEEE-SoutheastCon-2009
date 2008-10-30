import wx

class LRFPanel(wx.Panel):
    def __init__(self, parent, id):
        wx.Panel.__init__(self, parent, id, size = (600, 480))
        self.SetBackgroundColour(wx.WHITE)
        self.Bind(wx.EVT_PAINT, self.on_paint)
        
    def on_paint(self, event):
        dc = wx.PaintDC(self)
        dc.SetPen(wx.Pen('black', 1))
        dc.DrawLine(100,100,200,200)


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