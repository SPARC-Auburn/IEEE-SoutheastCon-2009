import Tkinter
from Tkconstants import *
import tkMessageBox

def window(tk):
	global photo #### SIC!
	frame=Tkinter.Frame(tk)
	frame.pack()

	canvas=Tkinter.Canvas(frame,bg ='blue', width=500,height=500)
	canvas.pack()

	photo=Tkinter.PhotoImage(file="gold.gif")
	canvas.create_image(200, 250, image=photo)
	button1=Tkinter.Button(frame,fg="blue", text="GENERATE", activebackground='red').pack(padx =50,side = LEFT, pady =15)
	button=Tkinter.Button(frame, fg ="blue",text="EXIT",activebackground='red',command=quitbutton).pack(side =LEFT, padx=20)


def quitbutton():
	if tkMessageBox.askyesno("Quit", "Do you really wish to quit?"):
		root.destroy()



root = Tkinter.Tk()
root.title("gold")
window(root)
root.mainloop() 